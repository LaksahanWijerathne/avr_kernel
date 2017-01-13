/*
 * debug.h
 *
 * Created: 12-01-2017 03:46:40
 *  Author: JP
 */ 


#ifndef DEBUG_H_
#define DEBUG_H_

#define bv_DBGEN		  	3

#define bm_DBGEN	    	(1<<bv_DBGEN)

void f_init_dbg(boolean use_lcd);

void f_debug_enable(void);

void f_debug_disable(void);

void f_debugger(void);

#endif /* DEBUG_H_ */