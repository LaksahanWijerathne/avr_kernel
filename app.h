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
#include "kernel.h"

#define c_APP_TICKS			      10		// 10ms AppTick
#define c_APP_TIMERID		      3

void f_init_app(void);

void f_run_app(void);


#endif /* APP_H_ */