/*
 * uart.h
 *
 * Created: 12-01-2017 09:20:08
 *  Author: JP
 */ 


#ifndef UART_H_
#define UART_H_

typedef void(*fp_uart_recv_cb)(void*);

void f_config_uart(void);

boolean f_reg_uart_cb(fp_uart_recv_cb hook, char c);

boolean f_dereg_uart_cb(fp_uart_recv_cb hook, char c);

unsigned char f_uart_peek_char(void);

size_t f_uart_peek_str(unsigned char* str, unsigned int n);

void f_uart_flush();

unsigned char f_uart_get_char(void);

size_t f_uart_get_str(unsigned char* str, unsigned int n);

size_t f_uart_put_char(unsigned char c);

size_t f_uart_put_str(char s[]);

unsigned char f_uart_new_line(void);

unsigned char f_uart_clrscr(void);

#endif /* UART_H_ */