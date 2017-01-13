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
#include "debug.h"
/*
 * TODO have config.h file to put
 *      all config info
 */

/*** Config data ***/
#define MCU_FAMILY	  "AVR"
#define MCU_NAME	    "ATMEGA328P"

#define ADC_PORT	PORTA

/*** Constants ***/
#define c_SYS_TIMEMAX	  	    0xFFFE
#define c_SYS_MAXSWTIMERS	    4

#define c_MAX_APPS      2

/*** Bit values ***/
#define bv_SYSTICK			0
#define bv_APP1TICK			1
#define bv_APP2TICK			2

/*** Bitmask ***/
#define bm_SYSTICK			(1<<bv_SYSTICK)
#define bm_APP1TICK			(1<<bv_APP1TICK)
#define bm_APP2TICK			(1<<bv_APP2TICK)

#define c_APP_TICKS			      10		// 10ms AppTick

/*
 * currently arguments are not supported
 * args should be NULL
 */
typedef void(*fp_app_init_cb)(void*);

typedef void(*fp_app_cb)(void*);

//void SIG_OUTPUT_COMPARE1A( void ) __attribute__ ( ( signal ) );

void f_init_systick_timer(unsigned short duration);

void f_kernel_tick(void);

uint8_t f_get_free_timer(void);

boolean f_reg_timer(uint8_t id, uint16_t ms);

boolean f_dereg_timer(uint8_t id);

boolean f_set_timer(uint8_t id, uint16_t ms);

boolean f_clr_timer(uint8_t id);

uint16_t f_check_timer(uint8_t id);

boolean f_reg_app(fp_app_init_cb hook, fp_app_cb run_hook);

#endif /* KERNEL_H_ */
