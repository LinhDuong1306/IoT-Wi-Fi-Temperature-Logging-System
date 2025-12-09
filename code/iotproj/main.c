#include "BOARD.h" // B?T BU?C: Ph?i ? ??u tiên ?? l?y F_CPU

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

// Include các th? vi?n driver
#include "LCD.h"      // Gi? s? b?n ?ã có file này
#include "ESP8266.h"  // ?ã bao g?m UART bên trong
#include "AHT20.h"    // ?ã bao g?m TWI bên trong

// ==========================
// C?u hình Chân & Ng??ng
// ==========================
#define LED_PIN         PC3 // LED báo tr?ng thái/Alarm
#define BUTTON_PIN      PD2 // Nút nh?n (INT0)
#define BUZZER_PIN      PD3 // Còi báo ??ng

#define TEMP_THRESHOLD  30.0f
#define HUM_THRESHOLD   60.0f

#define UPLOAD_INTERVAL 15  // Chu k? upload (giây)

// ==========================
// Bi?n toàn c?c
// ==========================
volatile uint8_t button_pressed = 0;
volatile uint8_t alarm_enabled = 1; // 1: B?t c?nh báo, 0: T?t
char display_buffer[20];

// ==========================
// Interrupt Service Routine (INT0)
// ==========================
ISR(INT0_vect) {
	// Ch? b?t s? ki?n, x? lý s? làm ? main loop ?? tránh delay trong ISR
	if (!(PIND & (1 << BUTTON_PIN))) {
		button_pressed = 1;
	}
}

// ==========================
// Hàm h? tr? ph?n c?ng
// ==========================
void Hardware_Init(void) {
	// 1. C?u hình Buzzer (PD3) & LED (PC3) là Output
	DDRD |= (1 << BUZZER_PIN);
	DDRC |= (1 << LED_PIN);
	
	// Tr?ng thái ban ??u
	PORTD &= ~(1 << BUZZER_PIN); // T?t Buzzer
	if (alarm_enabled) PORTC |= (1 << LED_PIN); // B?t LED n?u alarm ?ang enable
	
	// 2. C?u hình Button (PD2) là Input Pull-up
	DDRD &= ~(1 << BUTTON_PIN);
	PORTD |= (1 << BUTTON_PIN);
	
	// 3. C?u hình ng?t ngoài INT0 (C?nh xu?ng)
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);
	EIMSK |= (1 << INT0);
	
	sei(); // Cho phép ng?t toàn c?c
}

void Check_Button_Process(void) {
	if (button_pressed) {
		_delay_ms(50); // Debounce
		
		if (!(PIND & (1 << BUTTON_PIN))) { // Ki?m tra ch?c ch?n còn nh?n
			// ??o tr?ng thái c?nh báo
			alarm_enabled = !alarm_enabled;
			
			// C?p nh?t LED và Buzzer ngay l?p t?c
			if (alarm_enabled) {
				PORTC |= (1 << LED_PIN); // LED ON báo hi?u h? th?ng Alarm b?t
				} else {
				PORTC &= ~(1 << LED_PIN);  // LED OFF
				PORTD &= ~(1 << BUZZER_PIN); // T?t Buzzer ngay n?u ?ang kêu
			}
			
			// Ch? nh? nút
			while (!(PIND & (1 << BUTTON_PIN)));
		}
		button_pressed = 0; // Xóa c?
		
		// Xóa c? ng?t ?? tránh kích ho?t l?i ngay l?p t?c (INTF0)
		EIFR |= (1 << INTF0);
	}
}

void Alarm_Logic(float temp, float hum) {
	if (alarm_enabled) {
		if (temp > TEMP_THRESHOLD || hum > HUM_THRESHOLD) {
			PORTD |= (1 << BUZZER_PIN); // B?t còi
			} else {
			PORTD &= ~(1 << BUZZER_PIN); // T?t còi
		}
		} else {
		PORTD &= ~(1 << BUZZER_PIN); // Luôn t?t còi khi alarm disabled
	}
}

// Hàm delay thông minh: V?a delay v?a check nút nh?n
void Delay_With_Button_Check(uint16_t ms) {
	uint16_t loops = ms / 10;
	for (uint16_t i = 0; i < loops; i++) {
		_delay_ms(10);
		Check_Button_Process();
	}
}

// ==========================
// Hàm Main
// ==========================
int main(void) {
	float temp = 0.0, hum = 0.0;
	uint8_t wifi_status = 0;
	uint8_t time_count = 0;

	// 1. Kh?i t?o
	Hardware_Init();
	LCD_Init();
	
	LCD_SetCursor(0, 0);
	LCD_PrintString("System Init...");
	
	AHT20_Init(); // T? ??ng g?i I2C_Init
	ESP8266_Init(); // T? ??ng g?i UART_Init
	
	// 2. K?t n?i WiFi ban ??u
	LCD_SetCursor(0, 0);
	LCD_PrintString("Connecting WiFi ");
	wifi_status = ESP8266_ConnectWiFi();
	
	if(wifi_status) {
		LCD_SetCursor(1, 0);
		LCD_PrintString("WiFi: OK        ");
		} else {
		LCD_SetCursor(1, 0);
		LCD_PrintString("WiFi: Failed    ");
	}
	_delay_ms(1000);
	LCD_Clear();

	// ==========================
	// Vòng l?p chính
	// ==========================
	while (1) {
		// A. ??c c?m bi?n
		AHT20_ReadAll(&temp, &hum);
		
		// B. X? lý hi?n th? LCD
		LCD_SetCursor(0, 0);
		sprintf(display_buffer, "T:%.1fC H:%.1f%%", temp, hum);
		LCD_PrintString(display_buffer);
		
		// C. X? lý C?nh báo (Alarm)
		Alarm_Logic(temp, hum);
		
		// Hi?n th? tr?ng thái Alarm trên LCD dòng 2
		LCD_SetCursor(1, 0);
		if (alarm_enabled) LCD_PrintString("ALM:ON ");
		else               LCD_PrintString("ALM:OFF");
		
		// D. X? lý Upload ThingSpeak (m?i UPLOAD_INTERVAL giây)
		if (++time_count >= UPLOAD_INTERVAL) {
			time_count = 0;
			LCD_SetCursor(1, 8);
			LCD_PrintString(" Upld...");
			
			// Th? g?i d? li?u
			if (ESP8266_SendToThingSpeak(temp, hum)) {
				LCD_SetCursor(1, 8);
				LCD_PrintString(" S-OK   ");
				} else {
				LCD_SetCursor(1, 8);
				LCD_PrintString(" S-ERR  ");
				// N?u l?i, th? k?t n?i l?i WiFi cho l?n sau
				ESP8266_ConnectWiFi();
			}
			Delay_With_Button_Check(1000); // Show tr?ng thái g?i trong 1s
			} else {
			// Hi?n th? ??m ng??c (Optional)
			LCD_SetCursor(1, 8);
			sprintf(display_buffer, " Next:%2d", UPLOAD_INTERVAL - time_count);
			LCD_PrintString(display_buffer);
		}

		// E. Delay chu k? (1 giây)
		// Thay vì _delay_ms(1000) c?ng, ta dùng hàm check nút
		Delay_With_Button_Check(1000);
	}
	
	return 0;
}