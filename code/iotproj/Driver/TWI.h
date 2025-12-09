#ifndef TWI_H_
#define TWI_H_

#include <avr/io.h>

// Hàm giao ti?p I2C (TWI)
void I2C_Init(void);
uint8_t I2C_Start(uint8_t addr);
void I2C_Stop(void);
uint8_t I2C_Write(uint8_t data);
uint8_t I2C_ReadACK(void);
uint8_t I2C_ReadNACK(void);

#endif /* TWI_H_ */