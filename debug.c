/*
 * @file debug.c
 * 
 * 
 * @author: Jyotiprakash Sahoo
 */ 

/*
 * R XXXX		Read byte at address XXXX
 * W XXXX YY	Write byte at address XXXX
 * B XXXX		Peek 4 bytes in hex starting from address XXXX
 * C XXXX		Peek 4 bytes in ASCII starting from address XXXX
 * 1			PINA
 * 2			PINB
 * 3			PINC
 * 4			PIND
 * > X YY		Write YY to PORT X (1-4)
 * < X			Read PORT X (1-4)
 * D X YY		Write YY to DDR X (1-4)
 * d X			Read DDR X (1-4)
 * A X			Read Analog channel X (0-7)
 * I XXXX		Inspect data at EEPROM address XXXX
 * P XXXX YY	Write byte YY at EEPROM address XXXX
 * % XX			Display ASCII value on LCD
 * @ XX			Command to LCD
 *
 */

#include <avr/pgmspace.h>

#include "kernel.h"
#include "lcd.h"

#define c_ENTER		0x0D
#define c_NEWLINE	0x0A

#define outp(x,y)		(y) = (x)
#define inp(x)			(x)

extern uint8_t v_sys_state;

volatile unsigned char v_debugcount;
volatile unsigned char buf_debugentry[c_DEBUG_BUFLEN];

// prototypes
void f_debug_process_cmd(void);

void f_debug_readbyte    (void);
void f_debug_writebyte   (void);
void f_debug_asctab      (void);
void f_debug_bintab      (void);
void f_debug_pinA        (void);
void f_debug_pinB        (void);
void f_debug_pinC        (void);
void f_debug_pinD        (void);
void f_debug_rPROMbyte	 (void);
void f_debug_wPROMbyte	 (void);
void f_debug_wPORT		 (void);
void f_debug_rPORT		 (void);
void f_debug_readADC	 (void);
void f_debug_disable	   (void);

unsigned char __attribute__((progmem)) lut_dbgcmd[] = {
	
	'R', 'W', 'B', 'C', '1', '2', '3', '4', 'I', 'P', 'd', 'D', 'A', '*'
};	

boolean v_use_lcd;

/*** Hex to Bin ***/

unsigned short f_HexBin(char *hex_in)
{
	unsigned short data = 0;
	unsigned char i;
	char cbyte;

	for(i=0; i<4; i++)
	{
		cbyte = *(hex_in+i);
		if ( cbyte >= 'A') cbyte = (cbyte - 'A') + 10;
		else cbyte-= '0';
		data |= (cbyte << (4*(3-i)));
	}

	return data;
}

void f_BinHex(unsigned char bin_in, char *str_out) {
	static char lut_Hex[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	*(str_out+0) = lut_Hex[(bin_in >> 4) & 0x0F];
	*(str_out+1) = lut_Hex[bin_in & 0x0F];
}

unsigned char f_HexBinByte(char *hex_in)
{
	unsigned char data = 0;
	unsigned char i;
	char cbyte;

	for(i=0; i<2; i++)
	{
		cbyte = *(hex_in+i);
		if ( cbyte >= 'A') {
			cbyte = (cbyte - 'A') + 10;
		}			
		else {
			cbyte-= '0';
		}			
		data |= (cbyte << (4*(1-i)));
	}

	return data;
}



void (*fp_lut_dbgfunc[c_DEBUG_AVAILCMD])(void) = {
	f_debug_readbyte,			//'R'
	f_debug_writebyte,		//'W' 
	f_debug_asctab,				//'B'
	f_debug_bintab,				//'C'
	f_debug_pinA,				  //'1'
	f_debug_pinB,				  //'2'
	f_debug_pinC,				  //'3'
	f_debug_pinD,				  //'4'
	f_debug_rPROMbyte,		//'I'
	f_debug_wPROMbyte,		//'P'
	f_debug_rPORT,				//'d'
	f_debug_wPORT,				//'D'
	f_debug_readADC,			//'A'
	f_debug_disable				//'*'
};

void f_init_dbg(boolean use_lcd) {
	
	v_use_lcd = !!use_lcd;
	v_debugcount = 0;
	v_sys_state |= bm_DBGEN;
	
	f_uart_put_str("Debugger initialized");
	f_uart_new_line();
	f_uart_put_str("Debugger running now");
	f_uart_new_line();
	f_uart_put_str("? ");
	
	if (v_use_lcd == TRUE) {
	  f_lcd_put_str("? ");
	}	  
}

void f_debugger(void) {
	
	unsigned char c;		
	
	if((c = f_uart_get_char()) != (0xFF)) {
		
		if((v_sys_state & bm_DBGEN) == 0) {
			if (c == '*') {
				v_sys_state |= bm_DBGEN;
				f_uart_put_char(c);
				f_uart_new_line();
				f_uart_put_str("Debugger enabled!");
				f_uart_new_line();
				f_uart_put_str("? ");\

				if (v_use_lcd == TRUE) {
				  f_lcd_put_str("? ");
				}				  
			}
			return;
		}		
		
		buf_debugentry[v_debugcount] = c;
		
		v_debugcount++;
		
		if(v_debugcount == c_DEBUG_BUFLEN) {
			v_debugcount = 0;
			f_uart_put_str("Error");
			f_uart_new_line();
			f_uart_put_str("? ");

	    if (v_use_lcd == TRUE) {
			    f_lcd_clear();
			    f_lcd_put_str("? ");
		  }				
		}
		else {
			f_uart_put_char(c);
			
			if (v_use_lcd == TRUE) {
			  f_lcd_put_char(c);
			}
						  
			if(c == c_ENTER) {
				f_debug_process_cmd();
				if((v_sys_state & bm_DBGEN) == 0) {
					return;
				}					
				f_uart_put_str("? ");

      	if (v_use_lcd == TRUE) {
				  f_lcd_clear();
				  f_lcd_put_str("? ");
		    }				  
			}
		}			
	}
}

void f_debug_process_cmd(void) {
	uint8_t v_temp = 0;
	v_debugcount = 0;

	f_uart_new_line();
	
	while((v_temp < c_DEBUG_AVAILCMD) && (pgm_read_byte(&lut_dbgcmd[v_temp]) != buf_debugentry[0])) {
		++v_temp;
	}
	
	if(v_temp != c_DEBUG_AVAILCMD) {
		(*fp_lut_dbgfunc[v_temp])();
	}
}

void f_debug_port(unsigned char data) {
	char str[] = {"  "};

	f_BinHex(data, str);
	f_uart_put_str(str);
	f_uart_new_line();
}

void f_debug_port_v(unsigned char data) {
	char str[] = {"   "};
		
	f_BinHex(data, str);
	
	f_uart_put_str(str);
}

void f_debug_readbyte(void) {
	unsigned char v_data;
	unsigned short addr;
	char *p_str;

	p_str = (char *) &buf_debugentry[2];
	addr = (f_HexBin(p_str));
	v_data = * (unsigned char *) addr;
	f_debug_port(v_data);
}

void f_debug_writebyte   (void)
{
	unsigned char v_data;
	unsigned short addr;
	char *p_str;

	p_str = (char *) &buf_debugentry[2];
	addr = f_HexBin(p_str);
	p_str = (char *) &buf_debugentry[2+4+1];
	v_data = f_HexBinByte(p_str);

	* (unsigned char *) addr = v_data;
	/* readback */
	v_data = * (unsigned char *) addr;
	f_debug_port(v_data);
}
void f_debug_asctab      (void)
{
	unsigned char v_data;
	unsigned short addr;
	char *p_str; 
	char str[]={"        "};
	unsigned char i;

	p_str = (char *) &buf_debugentry[2];
	addr = (f_HexBin(p_str));

	for(i=0; i<8 ;i++)
	{
		v_data = * (unsigned char *) addr;
		addr++;
		str[i] = v_data;
	}

	f_uart_put_str(str);
	f_uart_new_line();

}
void f_debug_bintab(void)
{
	unsigned char v_data;
	unsigned short addr;
	char *p_str;
	char str[]={"            "};
	unsigned char i;

	p_str = (char *) &buf_debugentry[2];
	addr = (f_HexBin(p_str));
	p_str = &str[0];

	for(i=0; i<6 ;i++)
	{
		v_data = * (unsigned char *) addr;
		addr++;
		f_BinHex(v_data, p_str);
		p_str++;
		p_str++;
	}

	f_uart_put_str(str);
	f_uart_new_line();
}

void f_debug_pinA        (void)
{
	//unsigned char data;
	//data = inp(PINA);
	//f_debug_port(data);
}

void f_debug_pinB        (void)
{
	unsigned char data;
	data = inp(PINB);
	f_debug_port(data);
}

void f_debug_pinC        (void)
{
	unsigned char data;
	data = inp(PINC);
	f_debug_port(data);
}

void f_debug_pinD        (void)
{
	unsigned char data;
	data = inp(PIND);
	f_debug_port(data);
}


void f_debug_rPROMbyte  (void) {
/*
	unsigned char v_data;
	unsigned short addr;
	char *p_str;

	p_str = (char *) &buf_debugentry[2];
	addr = (f_HexBin(p_str));
	v_data = f_EERead( addr);
	f_debug_port(v_data);
*/
}

void f_debug_wPROMbyte (void) {
/*
	unsigned char v_data;
	unsigned short addr;
	char *p_str;

	p_str = (char *) &buf_debugentry[2];
	addr = f_HexBin(p_str);
	p_str = (char *) &buf_debugentry[2+4+1];
	v_data = f_HexBinByte(p_str);

    f_EEWrite( addr, v_data);
	
	// readback 
	v_data = f_EERead( addr);
	f_debug_port(v_data);
*/
}


void f_debug_disable(void)
{
	v_sys_state &= ~bm_DBGEN;
	
	f_uart_put_str("Debugger disabled!");
	f_uart_new_line();
	if (v_use_lcd == TRUE) {
	  f_lcd_clear();
	}	  
}

void f_debug_wPORT(void)
{
	unsigned char v_data;
	unsigned short addr;
	char *p_str;

	p_str = (char *) &buf_debugentry[2];
	addr = f_HexBinByte(p_str);
	p_str = (char *) &buf_debugentry[2+2+1];
	v_data = f_HexBinByte(p_str);
	
	switch (addr)
	{
		//case 0x01:
		//	outp(v_data, PORTA);
		//	break;
		case 0x02:
			outp(v_data, PORTB);
			break;
		case 0x03:
			outp(v_data, PORTC);
			break;
		case 0x04:
			outp(v_data, PORTD);
			break;
		case 0x11:
		//	outp(v_data, DDRA);
		//	break;
		case 0x12:
			outp(v_data, DDRB);
			break;
		case 0x13:
			outp(v_data, DDRC);
			break;
		case 0x14:
			outp(v_data, DDRD);
			break;
			
		default:
			break;	
			
	}

}

void f_debug_rPORT(void)
{
	unsigned char v_data = 0;
	unsigned short addr;
	char *p_str;

	p_str = (char *) &buf_debugentry[2];
	addr = f_HexBinByte(p_str);
	
	switch (addr)
	{
		//case 0x01:
		//	v_data = PORTA;
		//	break;
		case 0x02:
			v_data = PORTB;
			break;
		case 0x03:
			v_data = PORTC;
			break;
		case 0x04:
			v_data = PORTD;
			break;
		//case 0x11:
		//	v_data = DDRA;
		//	break;
		case 0x12:
			v_data = DDRB;
			break;
		case 0x13:
			 v_data = DDRC;
			break;
		case 0x14:
			 v_data = DDRD;
			break;
			
		default:
			break;	
			
	}
	
	f_debug_port(v_data);

}


void f_debug_readADC(void) {
/*
	unsigned char v_data = 0;
	unsigned short addr;
	char *p_str;

	p_str = (char *) &buf_debugentry[2];
	addr = f_HexBinByte(p_str);

	if ( (addr < 0) || (addr > 7) )
	{
		f_uart_put_str("Bad Chan\r");	
		return;
	}
	
	v_data = (unsigned char) ( (v_Chan[addr] >> 8) & 0xff);
	f_debug_port_v(v_data);
	v_data = (unsigned char) (v_Chan[addr] & 0xff);
	f_debug_port(v_data);

*/
}
