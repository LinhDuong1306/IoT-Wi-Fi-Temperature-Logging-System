// File: lcd.c
#include "LCD.h"

// ====================================================================
// CH?C N?NG N?I B?
// ====================================================================

static void LCD_PulseEnable(void) {
	LCD_CTRL_PORT |= (1 << LCD_E_PIN);  // E = 1
	_delay_us(1);
	LCD_CTRL_PORT &= ~(1 << LCD_E_PIN); // E = 0
	_delay_us(50);
}

// Mapping:
// Bit 4 (D4) -> PB0
// Bit 5 (D5) -> PD7
// Bit 6 (D6) -> PB1
// Bit 7 (D7) -> PB2
static void LCD_Nibble(unsigned char val) {
	// D4 (Bit 4) trên PB0 ---
	if (val & 0x10) {
		PORTB |= (1 << 0);
		} else {
		PORTB &= ~(1 << 0);
	}

	// D5 (Bit 5) trên PD7 ---
	if (val & 0x20) {
		PORTD |= (1 << 7);
		} else {
		PORTD &= ~(1 << 7);
	}

	// D6 (Bit 6) trên PB1 ---
	if (val & 0x40) {
		PORTB |= (1 << 1);
		} else {
		PORTB &= ~(1 << 1);
	}

	// D7 (Bit 7) trên PB2 ---
	if (val & 0x80) {
		PORTB |= (1 << 2);
		} else {
		PORTB &= ~(1 << 2);
	}
}

static void LCD_Send(unsigned char val, unsigned char isData) {
	// RS
	if (isData)
	LCD_CTRL_PORT |= (1 << LCD_RS_PIN);
	else
	LCD_CTRL_PORT &= ~(1 << LCD_RS_PIN);

	// G?i Nibble cao (D7-D4)
	LCD_Nibble(val);
	LCD_PulseEnable();

	// G?i Nibble th?p (D3-D0)
	LCD_Nibble(val << 4);
	LCD_PulseEnable();
}

// ====================================================================
// CH?C N?NG CÔNG C?NG
// ====================================================================

void LCD_Command(unsigned char cmd) {
	LCD_Send(cmd, 0);
}

void LCD_Data(unsigned char data) {
	LCD_Send(data, 1);
}

void LCD_Clear(void) {
	LCD_Command(0x01); // G?i l?nh 0x01 (Clear Display)
	_delay_ms(2);      // Ch? 2ms
}

void LCD_Init(void) {
	// 1. C?u hình Output
	
	// PORTD: RS(PD5), E(PD6), D5(PD7)
	DDRD |= (1 << 5) | (1 << 6) | (1 << 7);
	
	// PORTB: D4(PB0), D6(PB1), D7(PB2)
	DDRB |= (1 << 0) | (1 << 1) | (1 << 2);

	// Reset tr?ng thái ban ??u v? 0
	PORTD &= ~((1 << 5) | (1 << 6) | (1 << 7));
	PORTB &= ~((1 << 0) | (1 << 1) | (1 << 2));

	_delay_ms(50); // Ch? ngu?n ?n ??nh
	
	// RS = 0 (Command)
	LCD_CTRL_PORT &= ~(1 << LCD_RS_PIN);

	// L?n 1: G?i 0x30
	LCD_Nibble(0x30);
	LCD_PulseEnable();
	_delay_ms(5);

	// L?n 2: G?i 0x30
	LCD_Nibble(0x30);
	LCD_PulseEnable();
	_delay_us(200);

	// L?n 3: G?i 0x30
	LCD_Nibble(0x30);
	LCD_PulseEnable();
	_delay_us(200);

	// Chuy?n sang 4-bit mode: G?i 0x20
	LCD_Nibble(0x20);
	LCD_PulseEnable();
	_delay_us(200);

	// --- C?u hình chi ti?t ---
	LCD_Command(0x28);  // 4-bit mode, 2 lines, 5x8 font
	LCD_Command(0x0C);  // Display On, Cursor Off
	LCD_Command(0x06);  // Auto Increment cursor
	LCD_Command(0x01);  // Clear Display
	_delay_ms(2);
}

void LCD_PrintString(const char *str) {
	while (*str) {
		LCD_Data(*str++);
	}
}

void LCD_SetCursor(unsigned char row, unsigned char col) {
	unsigned char address;
	if (row == 0) {
		address = 0x80 + col;
		} else {
		address = 0xC0 + col;
	}
	LCD_Command(address);
}