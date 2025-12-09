// File: lcd.h
#ifndef LCD_H
#define LCD_H

#include "BOARD.h"
#include <avr/io.h>
#include <util/delay.h>

/* =========================================================
   RS -> PD5
   E  -> PD6
   D4 -> PB0 
   D5 -> PD7 
   D6 -> PB1
   D7 -> PB2
   ========================================================= */

// Chân ?i?u khi?n
#define LCD_CTRL_PORT   PORTD
#define LCD_CTRL_DDR    DDRD
#define LCD_RS_PIN      5      // PD5
#define LCD_E_PIN       6      // PD6

// Prototype các hàm
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_Clear(void);
void LCD_Init(void);
void LCD_PrintString(const char *str);
void LCD_SetCursor(unsigned char row, unsigned char col);

#endif