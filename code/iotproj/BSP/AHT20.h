#ifndef AHT20_H_
#define AHT20_H_

#include "BOARD.h"

// Include driver TWI (I2C)
#include "TWI.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// ??a ch? I2C 7-bit c?a AHT20
#define AHT20_ADDR 0x38

// Các l?nh theo datasheet
#define AHT20_CMD_INIT          0xBE    // Initialization command
#define AHT20_INIT_PARAM1       0x08    // First parameter for init
#define AHT20_INIT_PARAM2       0x00    // Second parameter for init

#define AHT20_CMD_TRIGGER       0xAC    // Trigger measurement
#define AHT20_TRIGGER_PARAM1    0x33    // First parameter for trigger
#define AHT20_TRIGGER_PARAM2    0x00    // Second parameter for trigger

#define AHT20_CMD_SOFT_RESET    0xBA    // Soft reset

#define AHT20_CMD_READ_STATUS   0x71    // Read status register

// Status bits
#define AHT20_STATUS_BUSY       0x80    // Bit[7]: 1=Busy, 0=Free
#define AHT20_STATUS_MODE       0x60    // Bit[6:5]: Mode status
#define AHT20_STATUS_CAL        0x08    // Bit[3]: 1=Calibrated, 0=Uncalibrated

// Hàm x? lý AHT20
void AHT20_Init(void);
void AHT20_SoftReset(void);
uint8_t AHT20_ReadStatus(void);
uint8_t AHT20_IsCalibrated(void);
uint8_t AHT20_IsBusy(void);
void AHT20_TriggerMeasurement(void);
void AHT20_ReadRawData(uint8_t *data);
float AHT20_GetTemperature(uint8_t *data);
float AHT20_GetHumidity(uint8_t *data);
void AHT20_ReadAll(float *temperature, float *humidity);

#endif /* AHT20_H_ */