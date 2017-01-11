/*
 * uart.c
 *
 * Created: 05-01-2014 07:48:24
 *  Author: JP
 */

#include "common.h"
#include "kernel.h"

#define BAUD_RATE_DEFAULT	9600

#define c_UART_TXBUFLEN		    128
#define c_UART_RXBUFLEN		    128

/*** Utility Macros ***/
#define UART_BAUDREG_CALC		((F_CPU - 16L*BAUD_RATE_DEFAULT)/(16L*BAUD_RATE_DEFAULT))

volatile unsigned char v_uart_txbuf_count;
volatile unsigned char v_uart_rxbuf_count;
unsigned char *p_uart_txin, *p_uart_txout;
unsigned char *p_uart_rxin, *p_uart_rxout;
unsigned char buf_uart_tx[c_UART_TXBUFLEN];
unsigned char buf_uart_rx[c_UART_RXBUFLEN];

char UART_CLR[] = {27, '[','H', 27, '[', '2', 'J',0};
char UART_NL[] = {0x0d,0x0a,0};

extern uint8_t v_sys_state;

#define c_MAX_UART_RECV_CB      2	

typedef void(*fp_uart_recv_cb)(void*);

void (*fp_uart_cb_arr[c_MAX_UART_RECV_CB])(void*);

/*
 * Replace index variables with bitmasks.
 */
uint8_t v_recv_cb_reg_idx;

char v_uart_recv_match[c_MAX_UART_RECV_CB];

boolean v_uart_cb_enabled;

void f_uart_flush() {
	
  p_uart_txin = p_uart_txout = buf_uart_tx;
	p_uart_rxin = p_uart_rxout = buf_uart_rx;

	v_uart_txbuf_count = 0;
	v_uart_rxbuf_count = 0;

}

void f_uart_flush_ext() {
	if (v_sys_state & bm_DBGEN) {
		return;
	}		
	else {
		f_uart_flush();
	}
}	

void f_config_uart(void) {
	
	UBRR0L = ((unsigned char) (UART_BAUDREG_CALC)) & 0xFF;
	UBRR0H = (((unsigned char) (UART_BAUDREG_CALC)) >> 8) & 0xFF;

	UCSR0A = 0x00;
	UCSR0B = (1<<RXCIE0) | (1<<UDRIE0)| (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

  f_uart_flush();

  v_uart_cb_enabled = FALSE;
}

boolean f_reg_uart_cb(fp_uart_recv_cb hook, char c) {
	
	if (hook == NULL) {
    return FALSE;
  }
  
  if (v_recv_cb_reg_idx == c_MAX_UART_RECV_CB) {
    /* cannot register anymore callbacks */
    return FALSE;
  }
  
  fp_uart_cb_arr[v_recv_cb_reg_idx] = hook;
  v_uart_recv_match[v_recv_cb_reg_idx] = c;
  
  v_recv_cb_reg_idx++;  
  
  return TRUE;
}

boolean f_dereg_uart_cb(fp_uart_recv_cb hook, char c) {
  boolean v_is_cb_found = FALSE;

  if (hook == NULL) {
    return v_is_cb_found;
  }

  uint8_t i;
	for (i = 0; i < v_recv_cb_reg_idx; i++) {
    if (fp_uart_cb_arr[i] == hook) {
      fp_uart_cb_arr[i] = NULL;
      v_is_cb_found = TRUE;
      
      /*
       * TODO: accomadate more than 2 callbacks
       *       OR change this!
       */
    }
    if ( (v_is_cb_found == TRUE) && ((i + 1) < v_recv_cb_reg_idx) ) {
      fp_uart_cb_arr[i] = fp_uart_cb_arr[i + 1];
      v_uart_recv_match[i] = v_uart_recv_match[i + 1];
    }
  }
  
  if (v_is_cb_found == TRUE) {
    v_recv_cb_reg_idx--;
  }    

  return v_is_cb_found;
}

void f_uart_enable_callbacks() {
  v_uart_cb_enabled = TRUE;
}  

void f_uart_disable_callbacks() {
  v_uart_cb_enabled = FALSE;
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

  uint8_t i;
  if (v_uart_cb_enabled == TRUE) {
	  for (i = 0; i < v_recv_cb_reg_idx; i++) {
		  /*
		   * if received character matches registered character
		   * process callback and return
		   * don't store character in buffer
		   */
	    if (v_uart_recv_match[i] == *p_uart_rxin) {
        (*fp_uart_cb_arr[i])(NULL);
        return;
	    } 
	  }
  }    

	++v_uart_rxbuf_count;

	if(++p_uart_rxin >= buf_uart_rx + c_UART_RXBUFLEN) {
		p_uart_rxin = buf_uart_rx;
	}
}

unsigned char f_uart_peek_char(void) {
  unsigned char c;

	if(v_uart_rxbuf_count > 0) {

		c = *p_uart_rxout;
		return c;
	}
  else {
	  return 0xFF;
	}
}

size_t f_uart_peek_str(unsigned char* str, unsigned int n) {
  uint8_t i;
	size_t len = 0;
	unsigned char* p_str = str;
	
	if (v_uart_rxbuf_count == 0) {
		return len;
	}
	
	n--;
	
	if ( n > v_uart_rxbuf_count ) {
		n = v_uart_rxbuf_count;
	}
	
	for (i = 0; i < n; i++) {
    *p_str++ = *(p_uart_rxout + i);
	  len++;
	}  

	*p_str = '\0';
	len++;
		
	return len;
}

unsigned char f_uart_get_char(void) {

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
	  return 0xFF;
	}
}	

unsigned char f_uart_get_char_ext(void) {
	if (v_sys_state & bm_DBGEN) {
		return 0xFF;
	}
	else {
	  return f_uart_get_char();
	}	
}

size_t f_uart_get_str(unsigned char* str, unsigned int n) {
	
	uint8_t i;
	size_t len = 0;
	unsigned char c;
	unsigned char* p_str = str;
	
	n--;
	
	for (i = 0; i < n; i++) {
		
		c = f_uart_get_char();
		
		if (c != 0xFF) {
			*p_str++ = c;
			len++;
		}
	}
	
	*p_str = '\0';
	len++;

	return len;
}

size_t f_uart_get_str_ext(unsigned char* str, unsigned int n) {
	if (v_sys_state & bm_DBGEN) {
		return 0;
	}
	else {
	  return f_uart_get_str(str, n);
	}	
}

/*
 * TODO:  optimize this
 */
size_t f_uart_put_char(unsigned char c) {
	
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


size_t f_uart_put_char_ext(unsigned char c) {
	if (v_sys_state & bm_DBGEN) {
		return 0;
	}
	else {
    return f_uart_put_char(c);
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

size_t f_uart_put_str_ext(char s[]) {
	if (v_sys_state & bm_DBGEN) {
		return 0;
	}
	else {
		return f_uart_put_str(s);
	}
}

unsigned char f_uart_new_line(void) {
	
	return f_uart_put_str(UART_NL);
}

unsigned char f_uart_new_line_ext(void) {
	if (v_sys_state & bm_DBGEN) {
		return 0xFF;
	}
	else {
		return f_uart_new_line();
	}
	
}

unsigned char f_Uart_ClrScr(void) {
	
	return f_uart_put_str(UART_CLR);
}