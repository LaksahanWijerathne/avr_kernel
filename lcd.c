/*
 * lcd.c
 *
 * Created: 08-02-2014 08:36:07
 *  Author: JP
 */ 

/************* LCD routines ************/

#include "lcd.h"

uint8_t v_data_pins[8] = {3, 2, 1, 0, 255, 255, 255, 255};
uint8_t bv_LCDRS;
uint8_t bv_LCDEN;

void f_lcd_toggle_en()
{
	LCD_DDR |= bm_LCDEN;
	// set EN pin as LOW
	LCD_PORT &= ~bm_LCDEN;
	DELAY_US(1);
	// set EN pin as HIGH
	LCD_PORT |= bm_LCDEN;
	DELAY_US(200);
	// set EN pin as LOW
	LCD_PORT &= ~bm_LCDEN;
	DELAY_US(100);
}

void f_lcd_write_nibble(unsigned char nib)
{
	uint8_t i, bit;
	
	LCD_DDR |= 0x0F;
	
	for (i = 0; i < 4; i++) {
		bit = (nib >> i) & 0x01;
		if (bit == 1) {
			LCD_PORT |= 1 << v_data_pins[i];
		}
		else {
			LCD_PORT &= ~(1 << v_data_pins[i]);
		}					
	}

	f_lcd_toggle_en();
}

void f_lcd_data(unsigned char data)
{
	// Set RS pin as HIGH
	LCD_PORT |= bm_LCDRS;
	
	// write high nibble
	f_lcd_write_nibble(data >> 4);
	// write low nibble
	f_lcd_write_nibble(data & 0x0F);
}

void f_lcd_cmd(char cmd)
{
	// Set RS pin as LOW
	LCD_PORT &= ~bm_LCDRS;

    // write high nibble
	f_lcd_write_nibble(cmd >> 4);
	// write low nibble
	f_lcd_write_nibble(cmd & 0x0F);
}

//sets position of the cursor to the Rth row and Cth column
void f_lcd_set_cursor(short int row, short int col)
{  
	if (row == 0) {
		f_lcd_cmd(LCD_DDRAM_ADDR | col); 
	}
	else if(row == 1) { 
		f_lcd_cmd(LCD_DDRAM_ADDR | LCD_ROW2_OFFSET | col);
	}
}

void f_lcd_put_char(char c)
{
	f_lcd_data(c);
}

void f_lcd_put_str(char *str) {
	while(*str) {
		f_lcd_put_char(*str++);
	}
}

void f_lcd_put_digi(int n)
{
	f_lcd_data(48+n);
}

void f_lcd_put_int(int num)
{ 
	if(num==0) {
		f_lcd_put_digi(0);
	}			
	else {
		while(num){
			f_lcd_put_digi(num%10);
			num /= 10;
		}
	}
}

void f_lcd_put_num(float n,int p)
{
	int a,c;
	a=n;
	n=n-a;
	f_lcd_put_int(a);
	f_lcd_put_char('.');
	for(c=1;c<=p;c++) {
		n=n*10;
		f_lcd_put_int(n);
	}
}

void f_lcd_clear() {
	f_lcd_cmd(0x01);
	DELAY_US(2000);
	f_lcd_cmd(0x02);
	DELAY_US(2000);
}	
		
void f_lcd_reset()
{
	DELAY_US(50000);
	LCD_DDR |= 0x3F;
	LCD_PORT &= ~bm_LCDEN;
	LCD_PORT &= ~bm_LCDRS;

	f_lcd_write_nibble(0x03);
	DELAY_US(4500);

	f_lcd_write_nibble(0x03);
	DELAY_US(4500);

	f_lcd_write_nibble(0x03);
	DELAY_US(200);
	
	f_lcd_write_nibble(0x02);
}

void f_lcd_init(uint8_t rs,
                uint8_t en,
				        uint8_t D0,
				        uint8_t D1,
				        uint8_t D2,
				        uint8_t D3,
				        uint8_t D4,
				        uint8_t D5,
				        uint8_t D6,
				        uint8_t D7)
{
	if (rs != LCD_PIN_NC) {
		bv_LCDRS = rs;
	}
	if (en != LCD_PIN_NC) {
		bv_LCDEN = en;
	}
	
	if (D0 != LCD_PIN_NC) {
		v_data_pins[0] = D0;
	}
	if (D1 != LCD_PIN_NC) {
		v_data_pins[1] = D1;
	}
	if (D2 != LCD_PIN_NC) {
		v_data_pins[2] = D2;
	}
	if (D3 != LCD_PIN_NC) {
		v_data_pins[3] = D3;
	}
	if (D4 != LCD_PIN_NC) {
		v_data_pins[4] = D4;
	}
	if (D5 != LCD_PIN_NC) {
		v_data_pins[5] = D5;
	}
	if (D6 != LCD_PIN_NC) {
		v_data_pins[6] = D6;
	}
	if (D7 != LCD_PIN_NC) {
		v_data_pins[7] = D7;
	}
	
	OUT_PORT(LCD_DDR);
	ZERO_PORT(LCD_PORT);
	
	f_lcd_reset();      // Reset LCD
	DELAY_US(2000);

	f_lcd_cmd(0x28);    // 4-bit mode - 2 line - 5x7 font.
	f_lcd_cmd(0x08);    // Display off
	f_lcd_cmd(0x0F);    // Display on, cusror on, blink on.
	f_lcd_cmd(0x01);    // Clear display
	f_lcd_cmd(0x06);    // Automatic Increment - No Display shift.
	DELAY_US(2000);
}  
