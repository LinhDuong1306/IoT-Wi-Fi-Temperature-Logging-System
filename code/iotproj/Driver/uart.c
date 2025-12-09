#include "UART.h"

// Kh?i t?o UART
void UART_Init(void) {
	UBRR0H = (uint8_t)(MYUBRR >> 8);
	UBRR0L = (uint8_t)MYUBRR;
	UCSR0A |= (1 << U2X0); // Double speed cho 115200
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Cho phép RX và TX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Khung truy?n 8-bit
}

// G?i 1 ký t?
void UART_SendChar(char data) {
	while (!(UCSR0A & (1 << UDRE0))); // Ch? b? ??m phát tr?ng
	UDR0 = data;
}

// G?i chu?i ký t?
void UART_SendString(const char *str) {
	while (*str) {
		UART_SendChar(*str++);
	}
}

// Ki?m tra có d? li?u ??n không
uint8_t UART_Available(void) {
	return (UCSR0A & (1 << RXC0));
}

// Nh?n 1 ký t? 
char UART_ReceiveChar(void) {
	while (!(UCSR0A & (1 << RXC0))); // Ch? có d? li?u
	return UDR0;
}

// Xóa b? ??m nh?n
void UART_Flush(void) {
	unsigned char dummy;
	while (UCSR0A & (1 << RXC0)) {
		dummy = UDR0;
	}
	(void)dummy;
}