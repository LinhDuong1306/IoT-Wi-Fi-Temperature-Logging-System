#include "ESP8266.h"

//=====================//
//   ESP8266 Functions //
//=====================//

void ESP8266_SendCommand(const char *cmd) {
	UART_Flush(); // Dùng hàm t? driver UART.h
	UART_SendString(cmd);
	UART_SendString("\r\n");
}

uint8_t ESP8266_WaitResponse(const char *expected, uint16_t timeout_ms) {
	static char response[384];
	uint16_t index = 0;
	uint32_t start_time = 0;
	uint32_t timeout_loops = (uint32_t)timeout_ms * 100; // 1 loop ~ 10us
	
	memset(response, 0, sizeof(response));
	
	while (start_time < timeout_loops) {
		if (UART_Available()) {
			// Thay th? UDR0 b?ng hàm nh?n t? driver
			char c = UART_ReceiveChar();
			
			if (index < sizeof(response) - 1) {
				response[index++] = c;
				response[index] = '\0';
				} else {
				// Shift buffer n?u ??y
				memmove(response, response + 1, sizeof(response) - 2);
				index = sizeof(response) - 2;
				response[index++] = c;
				response[index] = '\0';
			}
			
			if (strstr(response, expected) != NULL) {
				return 1;
			}
			} else {
			_delay_us(10);
			start_time++;
		}
	}
	return 0; // Timeout
}

uint8_t ESP8266_WaitMultiResponse(const char *resp1, const char *resp2, uint16_t timeout_ms) {
	static char response[384];
	uint16_t index = 0;
	uint32_t start_time = 0;
	uint32_t timeout_loops = (uint32_t)timeout_ms * 100;
	
	memset(response, 0, sizeof(response));
	
	while (start_time < timeout_loops) {
		if (UART_Available()) {
			// Thay th? UDR0 b?ng hàm nh?n t? driver
			char c = UART_ReceiveChar();
			
			if (index < sizeof(response) - 1) {
				response[index++] = c;
				response[index] = '\0';
				} else {
				memmove(response, response + 1, sizeof(response) - 2);
				index = sizeof(response) - 2;
				response[index++] = c;
				response[index] = '\0';
			}
			
			if (strstr(response, resp1) != NULL || strstr(response, resp2) != NULL) {
				return 1;
			}
			} else {
			_delay_us(10);
			start_time++;
		}
	}
	return 0;
}

void ESP8266_ReadResponse(char *buffer, uint16_t max_len, uint16_t timeout_ms) {
	uint16_t index = 0;
	uint32_t start_time = 0;
	uint32_t timeout_loops = (uint32_t)timeout_ms * 100;
	
	memset(buffer, 0, max_len);
	
	while (start_time < timeout_loops && index < max_len - 1) {
		if (UART_Available()) {
			buffer[index++] = UART_ReceiveChar();
			buffer[index] = '\0';
			} else {
			_delay_us(10);
			start_time++;
		}
	}
}

void ESP8266_Init(void) {
	UART_Init(); // G?i hàm kh?i t?o t? driver
	_delay_ms(1000);
	
	// Reset ESP8266 ?? ??m b?o tr?ng thái s?ch
	ESP8266_SendCommand("AT+RST");
	_delay_ms(2500); // Ch? kh?i ??ng l?i
	UART_Flush();
	
	ESP8266_SendCommand("ATE0"); // T?t Echo
	ESP8266_WaitResponse("OK", 1000);
	
	ESP8266_SendCommand("AT+CWMODE=1"); // Station Mode
	ESP8266_WaitResponse("OK", 2000);
	
	ESP8266_SendCommand("AT+CIPMUX=0"); // Single Connection (QUAN TR?NG)
	ESP8266_WaitResponse("OK", 1000);
}

uint8_t ESP8266_ConnectWiFi(void) {
	char cmd[100];
	
	// Ng?t k?t n?i c?
	ESP8266_SendCommand("AT+CWQAP");
	ESP8266_WaitResponse("OK", 1000);
	
	_delay_ms(500);
	
	// K?t n?i m?i
	sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASS);
	ESP8266_SendCommand(cmd);
	
	// Ch? k?t n?i (Timeout 15s)
	if (ESP8266_WaitMultiResponse("OK", "WIFI GOT IP", 15000)) {
		_delay_ms(1000);
		return 1;
	}
	return 0;
}

void ESP8266_CloseTCP(void) {
	ESP8266_SendCommand("AT+CIPCLOSE");
	_delay_ms(200);
	UART_Flush();
}

uint8_t ESP8266_SendToThingSpeak(float field1, float field2) {
	char packet_buffer[300];
	char payload[50];
	char cmd_buffer[30];
	
	// 1. D?n d?p k?t n?i c?
	ESP8266_CloseTCP();
	
	// 2. T?o k?t n?i TCP
	ESP8266_SendCommand("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80");
	
	if (!ESP8266_WaitMultiResponse("CONNECT", "OK", 10000)) {
		return 0; // Th?t b?i
	}
	
	_delay_ms(200);
	
	// 3. Chu?n b? d? li?u (Payload)
	// X? lý s? float th? công ?? tránh l?i %f trên m?t s? trình biên d?ch AVR c?
	int f1_int = (int)field1;
	int f1_dec = (int)((field1 - f1_int) * 100 + 0.5); if (f1_dec < 0) f1_dec = -f1_dec;
	
	int f2_int = (int)field2;
	int f2_dec = (int)((field2 - f2_int) * 100 + 0.5); if (f2_dec < 0) f2_dec = -f2_dec;
	
	sprintf(payload, "field1=%d.%02d&field2=%d.%02d", f1_int, f1_dec, f2_int, f2_dec);
	
	// 4. T?o gói tin HTTP
	sprintf(packet_buffer,
	"POST /update HTTP/1.1\r\n"
	"Host: api.thingspeak.com\r\n"
	"Connection: close\r\n"
	"X-THINGSPEAKAPIKEY: %s\r\n"
	"Content-Type: application/x-www-form-urlencoded\r\n"
	"Content-Length: %d\r\n\r\n"
	"%s",
	THINGSPEAK_API_KEY, (int)strlen(payload), payload);
	
	// 5. G?i l?nh CIPSEND
	int packet_len = strlen(packet_buffer);
	sprintf(cmd_buffer, "AT+CIPSEND=%d", packet_len);
	ESP8266_SendCommand(cmd_buffer);
	
	// 6. Ch? d?u nh?c '>'
	if (!ESP8266_WaitResponse(">", 5000)) {
		ESP8266_CloseTCP();
		return 0;
	}
	
	// 7. G?i n?i dung
	UART_SendString(packet_buffer);
	
	// 8. Ch? xác nh?n SEND OK
	if (!ESP8266_WaitResponse("SEND OK", 10000)) {
		ESP8266_CloseTCP();
		return 0;
	}
	
	// 9. Ch? ?óng k?t n?i
	ESP8266_WaitResponse("CLOSED", 5000);
	
	return 1; // Thành công
}