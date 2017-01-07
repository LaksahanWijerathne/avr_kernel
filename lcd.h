/*
 * lcd.h
 *
 * Created: 08-02-2014 08:36:19
 *  Author: JP
 */ 


#ifndef LCD_H_
#define LCD_H_

#include "common.h"

/*** C0nfig data ***/
#define LCD_PORT	PORTB
#define LCD_DDR		DDRB

//#define bv_LCDRS		    5
//#define bv_LCDEN			  4
//#define b_LCDWrite		  1
 
#define bm_LCDRS		      (1<<bv_LCDRS)
#define bm_LCDEN			    (1<<bv_LCDEN)
//#define m_LCDWrite		  (1<<b_LCDWrite)

#define LCD_DDRAM_ADDR			0x80
#define LCD_ROW2_OFFSET		  0x40

//#define LCD_WRITE_EN	(LCD_PORT &= ~m_LCDWrite)

#define LCD_PIN_NC          0xFF   

// commands
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

// flags for function set
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

void f_lcd_write (unsigned char data);

void f_lcd_cmd(char cmd);

void f_lcd_set_cursor(short int r,short int c);                 //sets position of the cursor to the Rth row and Cth column

void f_lcd_put_char(char c);

void f_lcd_put_str(char *str);

void f_lcd_put_digi(int n);

void f_lcd_put_int(int num);

void f_lcd_put_num(float n,int p);
		
void f_lcd_reset();

void f_lcd_init(uint8_t rs,
                uint8_t en,
				        uint8_t D0,
				        uint8_t D1,
				        uint8_t D2,
				        uint8_t D3,
				        uint8_t D4,
				        uint8_t D5,
				        uint8_t D6,
				        uint8_t D7);

#endif /* LCD_H_ */