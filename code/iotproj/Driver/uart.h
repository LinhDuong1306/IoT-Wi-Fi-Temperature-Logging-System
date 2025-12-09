#ifndef UART_H_
#define UART_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>

// BAUDRATE
#define BAUD 115200
#define MYUBRR (F_CPU/8/BAUD - 1)

// FUNCTION
void UART_Init(void);
void UART_SendChar(char data);
void UART_SendString(const char *str);
char UART_ReceiveChar(void);
uint8_t UART_Available(void);
void UART_Flush(void);

#endif /* UART_H_ */