/*
 * app.h
 *
 * Created: 24-02-2014 08:35:49
 *  Author: JP
 */ 


#ifndef APP_H_
#define APP_H_

#include "common.h"
#include "utility.h"

#define c_APP_TIMEOUT_MS    3000

#define bm_APP_TRIGGER      0x04

#define bm_APP_STATE        0x03

typedef enum {
	e_APP_STATE_IDLE,
	e_APP_STATE_ON
} app_state_t;

void f_init_app_led(void* data);

void f_app_uart_cb(void* data);

void f_run_app(void* data);


#endif /* APP_H_ */