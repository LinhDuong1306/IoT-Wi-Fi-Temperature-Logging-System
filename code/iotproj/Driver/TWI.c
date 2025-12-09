#include "TWI.h"

// Kh?i t?o I2C (TWI)
void I2C_Init(void) {
	TWSR = 0x00;             // prescaler = 1
	TWBR = 0x48;             // 100kHz v?i F_CPU = 16MHz
}

// G?i tín hi?u Start và ??a ch? Slave
uint8_t I2C_Start(uint8_t addr) {
	// G?i ?i?u ki?n START
	TWCR = (1<<TWSTA)|(1<<TWEN)|(1<<TWINT);
	while (!(TWCR & (1<<TWINT)));

	// G?i ??a ch? + R/W
	TWDR = addr;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));

	return (TWSR & 0xF8);
}

// G?i tín hi?u Stop
void I2C_Stop(void) {
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

// Ghi 1 byte d? li?u
uint8_t I2C_Write(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return (TWSR & 0xF8);
}

// ??c 1 byte và g?i ACK (ti?p t?c ??c)
uint8_t I2C_ReadACK(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}

// ??c 1 byte và g?i NACK (k?t thúc ??c)
uint8_t I2C_ReadNACK(void) {
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}