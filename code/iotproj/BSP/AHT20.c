#include "aht20.h"

//======================//
//   AHT20 Functions    //
//======================//

uint8_t AHT20_ReadStatus(void) {
	uint8_t status;
	
	// S? d?ng hàm t? driver TWI.h
	I2C_Start((AHT20_ADDR<<1) | 0); // Write mode
	I2C_Write(AHT20_CMD_READ_STATUS);
	I2C_Stop();
	
	_delay_us(10);
	
	I2C_Start((AHT20_ADDR<<1) | 1); // Read mode
	status = I2C_ReadNACK();
	I2C_Stop();
	
	return status;
}

uint8_t AHT20_IsCalibrated(void) {
	uint8_t status = AHT20_ReadStatus();
	return (status & AHT20_STATUS_CAL) ? 1 : 0;
}

uint8_t AHT20_IsBusy(void) {
	uint8_t status = AHT20_ReadStatus();
	return (status & AHT20_STATUS_BUSY) ? 1 : 0;
}

void AHT20_SoftReset(void) {
	I2C_Start((AHT20_ADDR<<1) | 0);
	I2C_Write(AHT20_CMD_SOFT_RESET);
	I2C_Stop();
	
	_delay_ms(20); // Wait for reset to complete
}

void AHT20_Init(void) {
	// kh?i t?o I2C
	I2C_Init();
	
	// ??i 40ms sau power-on
	_delay_ms(40);
	
	// Ki?m tra calibration status
	if (!AHT20_IsCalibrated()) {
		// G?i l?nh initialization
		I2C_Start((AHT20_ADDR<<1) | 0);
		I2C_Write(AHT20_CMD_INIT);
		I2C_Write(AHT20_INIT_PARAM1);
		I2C_Write(AHT20_INIT_PARAM2);
		I2C_Stop();
		
		_delay_ms(10); // ??i initialization hoàn t?t
	}
}

void AHT20_TriggerMeasurement(void) {
	I2C_Start((AHT20_ADDR<<1) | 0);
	I2C_Write(AHT20_CMD_TRIGGER);
	I2C_Write(AHT20_TRIGGER_PARAM1);
	I2C_Write(AHT20_TRIGGER_PARAM2);
	I2C_Stop();
}

void AHT20_ReadRawData(uint8_t *data) {
	// Trigger measurement
	AHT20_TriggerMeasurement();
	
	// ??i measurement hoàn t?t (t?i ?a 80ms)
	_delay_ms(80);
	
	// Ki?m tra busy status
	while (AHT20_IsBusy()) {
		_delay_ms(10);
	}
	
	// ??c 6 bytes d? li?u
	I2C_Start((AHT20_ADDR<<1) | 1); // Read mode
	
	data[0] = I2C_ReadACK(); // Status byte
	data[1] = I2C_ReadACK(); // Humidity[19:12]
	data[2] = I2C_ReadACK(); // Humidity[11:4]
	data[3] = I2C_ReadACK(); // Humidity[3:0] + Temperature[19:16]
	data[4] = I2C_ReadACK(); // Temperature[15:8]
	data[5] = I2C_ReadNACK(); // Temperature[7:0]
	
	I2C_Stop();
}

float AHT20_GetHumidity(uint8_t *data) {
	uint32_t raw_humidity;
	float humidity;
	
	// Trích xu?t 20-bit humidity
	raw_humidity = ((uint32_t)data[1] << 12) |
	((uint32_t)data[2] << 4) |
	((uint32_t)data[3] >> 4);
	
	// RH[%] = (S_RH / 2^20) * 100%
	humidity = ((float)raw_humidity / 1048576.0) * 100.0;
	
	if (humidity < 0) humidity = 0;
	if (humidity > 100) humidity = 100;
	
	return humidity;
}

float AHT20_GetTemperature(uint8_t *data) {
	uint32_t raw_temperature;
	float temperature;
	
	// Trích xu?t 20-bit temperature
	raw_temperature = (((uint32_t)data[3] & 0x0F) << 16) |
	((uint32_t)data[4] << 8) |
	((uint32_t)data[5]);
	
	// T(°C) = (S_T / 2^20) * 200 - 50
	temperature = ((float)raw_temperature / 1048576.0) * 200.0 - 50.0;
	
	return temperature;
}

void AHT20_ReadAll(float *temperature, float *humidity) {
	uint8_t data[6];
	
	// ??c d? li?u thô
	AHT20_ReadRawData(data);
	
	// Chuy?n ??i sang giá tr? th?c
	*humidity = AHT20_GetHumidity(data);
	*temperature = AHT20_GetTemperature(data);
}