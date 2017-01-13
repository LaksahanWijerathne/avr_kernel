/*
 * uart_priv.h
 *
 * Created: 12-01-2017 09:25:31
 *  Author: JP
 */ 


#ifndef UART_PRIV_H_
#define UART_PRIV_H_

#include "uart.h"

void f_uart_enable_callbacks();

void f_uart_disable_callbacks();

void f_uart_flush_priv();

unsigned char f_uart_get_char_priv(void);

size_t f_uart_get_str_priv(unsigned char* str, unsigned int n);

size_t f_uart_put_char_priv(unsigned char c);

size_t f_uart_put_str_priv(char s[]);

unsigned char f_uart_new_line_priv(void);

unsigned char f_uart_clrscr_priv(void);

#endif /* UART_PRIV_H_ */