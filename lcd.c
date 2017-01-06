/*
 * lcd.c
 *
 * Created: 08-02-2014 08:36:07
 *  Author: JP
 */ 

/************* LCD routines ************/

#include "lcd.h"

uint8_t data_pins[8] = {3, 2, 1, 0, 255, 255, 255, 255};

void f_LCD_ToggleEN()
{
	LCD_DDR |= m_LCDEnable;
	// set EN pin as LOW
	LCD_PORT &= ~m_LCDEnable;
	DELAY_US(1);
	// set EN pin as HIGH
	LCD_PORT |= m_LCDEnable;
	DELAY_US(200);
	// set EN pin as LOW
	LCD_PORT &= ~m_LCDEnable;
	DELAY_US(100);
}

void f_LCD_WriteNibble(unsigned char nib)
{
	uint8_t i, bit;
	
	LCD_DDR |= 0x0F;
	
	for (i = 0; i < 4; i++) {
		bit = (nib >> i) & 0x01;
		if (bit == 1) {
			LCD_PORT |= 1 << data_pins[i];
		}
		else {
			LCD_PORT &= ~(1 << data_pins[i]);
		}					
	}

	f_LCD_ToggleEN();
}

void f_LCD_Data(unsigned char data)
{
	// Set RS pin as HIGH
	LCD_PORT |= m_LCDReadSel;
	
	// write high nibble
	f_LCD_WriteNibble(data >> 4);
	// write low nibble
	f_LCD_WriteNibble(data & 0x0F);
}

void f_LCD_Cmd(char cmd)
{
	// Set RS pin as LOW
	LCD_PORT &= ~m_LCDReadSel;

    // write high nibble
	f_LCD_WriteNibble(cmd >> 4);
	// write low nibble
	f_LCD_WriteNibble(cmd & 0x0F);
}

//sets position of the cursor to the Rth row and Cth column
void f_lcd_set_cursor(short int row, short int col)
{  
	if (row == 0) {
		f_LCD_Cmd(LCD_DDRAM | col); 
	}
	else if(row == 1) { 
		f_LCD_Cmd(LCD_DDRAM | LCD_ROW2_OFFSET | col);
	}
}

void f_lcd_put_char(char c)
{
	f_LCD_Data(c);
}

void f_lcd_put_str(char *str) {
	while(*str) {
		f_lcd_put_char(*str++);
	}
}

void f_LCD_PutDigi(int n)
{
	f_LCD_Data(48+n);
}

void f_LCD_PutInt(int num)
{ 
	if(num==0) {
		f_LCD_PutDigi(0);
	}			
	else {
		while(num){
			f_LCD_PutDigi(num%10);
			num /= 10;
		}
	}
}

void f_LCD_PutNum(float n,int p)
{
	int a,c;
	a=n;
	n=n-a;
	f_LCD_PutInt(a);
	f_lcd_put_char('.');
	for(c=1;c<=p;c++) {
		n=n*10;
		f_LCD_PutInt(n);
	}
}

void f_lcd_clear() {
	f_LCD_Cmd(0x01);
	DELAY_US(2000);
	f_LCD_Cmd(0x02);
	DELAY_US(2000);
}	
		
void f_LCD_Reset()
{
	DELAY_US(50000);
	LCD_DDR |= 0x3F;
	LCD_PORT &= ~m_LCDEnable;
	LCD_PORT &= ~m_LCDReadSel;

	f_LCD_WriteNibble(0x03);
	DELAY_US(4500);

	f_LCD_WriteNibble(0x03);
	DELAY_US(4500);

	f_LCD_WriteNibble(0x03);
	DELAY_US(200);
	
	f_LCD_WriteNibble(0x02);
}

void f_config_display()
{
	LCD_DDR |= 0xFF;
	LCD_PORT = 0x00;
	
	f_LCD_Reset(); // Call LCD reset
	DELAY_US(2000);
	f_LCD_Cmd(0x28); // 4-bit mode - 2 line - 5x7 font.
	f_LCD_Cmd(0x08); // Display off
	f_LCD_Cmd(0x0F); // Display on, cusror on, blink on.
	f_LCD_Cmd(0x01); // Clear display
	f_LCD_Cmd(0x06); // Automatic Increment - No Display shift.
	DELAY_US(2000);
}  
