/*
 * uart.c
 *
 * Created: 05-01-2014 07:48:24
 *  Author: JP
 */

#include "common.h"

#define BAUD_RATE	9600

#define c_UART_TXBUFLEN		    128
#define c_UART_RXBUFLEN		    128

/*** Utility Macros ***/
#define UART_BAUDREG_CALC		((F_CPU - 16L*BAUD_RATE)/(16L*BAUD_RATE))

volatile unsigned char v_uart_txbuf_count;
volatile unsigned char v_uart_rxbuf_count;
unsigned char *p_uart_txin, *p_uart_txout;
unsigned char *p_uart_rxin, *p_uart_rxout;
unsigned char buf_uart_tx[c_UART_TXBUFLEN];
unsigned char buf_uart_rx[c_UART_RXBUFLEN];

char UART_CLR[] = {27, '[','H', 27, '[', '2', 'J',0};
char UART_NL[] = {0x0d,0x0a,0};

void f_config_serial(void) {
	
	UBRR0L = ((unsigned char) (UART_BAUDREG_CALC)) & 0xFF;
	UBRR0H = (((unsigned char) (UART_BAUDREG_CALC)) >> 8) & 0xFF;

	UCSR0A = 0x00;
	UCSR0B = (1<<RXCIE0) | (1<<UDRIE0)| (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

	p_uart_txin = p_uart_txout = buf_uart_tx;
	p_uart_rxin = p_uart_rxout = buf_uart_rx;

	v_uart_txbuf_count = 0;
	v_uart_rxbuf_count = 0;

}

ISR(USART_UDRE_vect) {
	if (v_uart_txbuf_count > 0) {
		UDR0 = *p_uart_txout;

		if(++p_uart_txout >= buf_uart_tx + c_UART_TXBUFLEN) {
			p_uart_txout = buf_uart_tx;
		}

		if(--v_uart_txbuf_count == 0) {
			UCSR0B &= ~(1<<UDRIE0);
		}
	}
}

ISR(USART_RX_vect) {
	*p_uart_rxin = UDR0;

	++v_uart_rxbuf_count;

	if(++p_uart_rxin >= buf_uart_rx + c_UART_RXBUFLEN) {
		p_uart_rxin = buf_uart_rx;
	}
}

unsigned short f_uart_get_char(void) {
	unsigned char c;

	if(v_uart_rxbuf_count > 0) {
		cli();

		v_uart_rxbuf_count--;

		c = *p_uart_rxout;

		if (++p_uart_rxout >= buf_uart_rx + c_UART_RXBUFLEN) {
			p_uart_rxout = buf_uart_rx;
		}

		sei();

		return c;
	}
  else {
	  return -1;
	}
}

unsigned char f_uart_put_char(unsigned char c)
{
    if (v_uart_txbuf_count < c_UART_TXBUFLEN) {
        cli();

	++v_uart_txbuf_count;

	*p_uart_txin = c;

	if (++p_uart_txin >= buf_uart_tx + c_UART_TXBUFLEN) {
            	p_uart_txin = buf_uart_tx;
	}
        
       	UCSR0B |= (1<<UDRIE0);
        
        sei();

	return 1;
    }
    else {
        return 0;
    }
}

size_t f_uart_put_str(char s[]) {
	char *c = s;
	size_t len = 0;

	while (*c) {
    if (f_uart_put_char(*c)) {
      ++c;
	    ++len;
		}
	}

	return len;
}

unsigned char f_uart_new_line(void) {
	
	return f_uart_put_str(UART_NL);
}

unsigned char f_Uart_ClrScr(void) {
	
	return f_uart_put_str(UART_CLR);
}