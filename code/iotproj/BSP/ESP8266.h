#ifndef ESP8266_H_
#define ESP8266_H_

// Include c?u hình Board (ch?a F_CPU)
#include "BOARD.h"

// Include các th? vi?n c?n thi?t
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

// Include driver UART ?ã tách
#include "UART.h"

// Thông tin WiFi và ThingSpeak
// (B?n có th? s?a l?i thông tin WiFi c?a b?n ? ?ây)
#define WIFI_SSID "P632"
#define WIFI_PASS "632632632"
#define THINGSPEAK_API_KEY "GFTHOFIINSM11O0T"

// ESP8266 Core Functions
void ESP8266_Init(void);
void ESP8266_SendCommand(const char *cmd);
uint8_t ESP8266_WaitResponse(const char *expected, uint16_t timeout_ms);
uint8_t ESP8266_WaitMultiResponse(const char *resp1, const char *resp2, uint16_t timeout_ms);
void ESP8266_ReadResponse(char *buffer, uint16_t max_len, uint16_t timeout_ms);

// ESP8266 WiFi Functions
uint8_t ESP8266_ConnectWiFi(void);

// ESP8266 TCP Functions
void ESP8266_CloseTCP(void);

// ESP8266 ThingSpeak Functions
uint8_t ESP8266_SendToThingSpeak(float field1, float field2);

#endif /* ESP8266_H_ */