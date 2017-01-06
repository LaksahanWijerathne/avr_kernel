/*
 * lcd.h
 *
 * Created: 08-02-2014 08:36:19
 *  Author: JP
 */ 


#ifndef LCD_H_
#define LCD_H_

#include "common.h"

#define LCD_PORT	PORTB
#define LCD_DDR		DDRB

#define b_LCDReadSel		5
#define b_LCDEnable			4
//#define b_LCDWrite		1
 
#define m_LCDEnable			(1<<b_LCDEnable)
#define m_LCDReadSel		(1<<b_LCDReadSel)
//#define m_LCDWrite		(1<<b_LCDWrite)

#define LCD_DDRAM			0x80
#define LCD_ROW2_OFFSET		0x40

//#define LCD_WRITE_EN	(LCD_PORT &= ~m_LCDWrite)

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

void f_LCD_Write (unsigned char data);

void f_LCD_Cmd(char cmd);

void f_LCD_SetPos(short int r,short int c);                 //sets position of the cursor to the Rth row and Cth column

void f_lcd_put_char(char c);

void f_lcd_put_str(char *str);

void f_LCD_PutDigi(int n);

void f_LCD_PutInt(int num);

void f_LCD_PutNum(float n,int p);
		
void f_LCD_Reset();

void f_config_display();





#endif /* LCD_H_ */