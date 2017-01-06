/*
 * kernel.h
 *
 * Created: 01-12-2013 08:25:56
 *  Author: JP
 *
 *
 */ 

#ifndef KERNEL_H_
#define KERNEL_H_

#include "common.h"

#define F_CPU		16000000UL

/*** Settings ***/
#define CPU_FAMILY	AVR
#define CPU_NAME	ATMEGA328P
#define ADC_PORT	PORTA


/*** Constants ***/
#define c_SYS_TIMEMAX	  	    0xFFFE
#define c_SYS_SWTIMERSMAX	    4
#define c_SYS_SWTIMERID0	    0
#define c_SYS_SWTIMERID1	    1
#define c_SYS_SWTIMERID2	    2
#define c_SYS_SWTIMERID3	    3

#define c_APP_TICKS			      10		// 10ms AppTick
#define c_APP_TIMERID		      3

#define c_DEBUG_AVAILCMD	    14
#define c_DEBUG_BUFLEN		    128

#define c_MAX_APP_INIT_HOOKS      2

/*** Bit values ***/
#define bv_SYSTICK			0
#define bv_APPTICK			1
#define bv_DBGEN		  	2

/*** Bitmask ***/
#define bm_SYSTICK			(1<<bv_SYSTICK)
#define bm_APPTICK			(1<<bv_APPTICK)
#define bm_DBGEN	  	(1<<bv_DBGEN)


void SIG_OUTPUT_COMPARE1A( void ) __attribute__ ( ( signal ) );

void f_init_systick_timer(unsigned short duration);

void f_kernel(void);

#endif /* KERNEL_H_ */
