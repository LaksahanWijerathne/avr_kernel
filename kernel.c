/*
 *  kernel.c
 *  simple kernel for AVRs. currently for atmega32
 *  Created: 01-12-2013 08:14:27
 *  Author: JP
 */ 


#include "kernel.h"
#include "debug.h"
#include "uart.h"
#include "lcd.h"
#include "app.h"

/********************/

volatile uint16_t v_sys_state;		// the internal kernel status variable
volatile uint16_t v_sys_timer;		// the kernel timer count variable

uint16_t v_sw_timer_ms[c_SYS_MAXSWTIMERS];

/*
 * Bit mask for SW timers
 * Set bits for timers in use
 */
uint8_t v_sw_timers;

void (*fp_app_init_cb_arr[c_MAX_APPS])(void*);

void (*fp_app_cb_arr[c_MAX_APPS])(void*);

/*
 * TODO: Replace index variables with bitmasks.
 */
uint8_t v_app_init_cb_idx;

/* Timer interrupt routine */
//void SIG_OUTPUT_COMPARE1A( void ) {
ISR(TIMER1_COMPA_vect) {
  SET_REG_BIT(v_sys_state, bv_SYSTICK);
	
  /* handle overflow */
	if (v_sys_timer == 0xFFFF) {
		SET_REG(v_sys_timer, 0x0000);
	}
}

uint8_t f_get_free_timer(void)
{
	uint8_t id;
	for(id = 0; id < c_SYS_MAXSWTIMERS; id++)
	{
		if ( (v_sw_timers & (1<<id)) == 0 )
		{
			SET_REG_BIT(v_sw_timers, id);
			return id;
		}
	}
	return 0xFF;
}

boolean f_dereg_timer(uint8_t id)
{
	if (id >= c_SYS_MAXSWTIMERS)
	{
		return FALSE;
	}
	if ( (v_sw_timers & (1<<id)) == 0 )
	{
		return FALSE;
	}
	
	CLR_REG_BIT(v_sw_timers, id);
	return TRUE;
}

boolean f_set_timer(uint8_t id, uint16_t ms)
{
	if (id >= c_SYS_MAXSWTIMERS)
	{
		return FALSE;
	}
	if ( (v_sw_timers & (1<<id)) == 0 )
	{
		return FALSE;
	}
	
	v_sw_timer_ms[id] = ms;
	return TRUE;
}

boolean f_clr_timer(uint8_t id)
{
	if (id >= c_SYS_MAXSWTIMERS)
	{
		return FALSE;
	}
	if ( (v_sw_timers & (1<<id)) == 0 )
	{
		return FALSE;
	}
	
	v_sw_timer_ms[id] = 0;
	return TRUE;
}

uint16_t f_check_timer(uint8_t id)
{
	if (id >= c_SYS_MAXSWTIMERS)
	{
		return 0xFFFF;
	}
	if ( (v_sw_timers & (1<<id)) == 0 )
	{
		return 0xFFFF;
	}

  return v_sw_timer_ms[id];
}

boolean f_reg_app(fp_app_init_cb init_hook, fp_app_cb run_hook) {
  
  if ( (init_hook == NULL) || (run_hook == NULL) ) {
    return FALSE;
  }

  /* cannot register anymore callbacks */
  if (v_app_init_cb_idx == c_MAX_APPS) {
    return FALSE;
  }
  
  fp_app_init_cb_arr[v_app_init_cb_idx] = init_hook;
  fp_app_cb_arr[v_app_init_cb_idx] = run_hook;

  v_app_init_cb_idx++;
  
  return TRUE;
}

void f_init_systick_timer(unsigned short duration) { 
	
  TCCR1A = (0<<COM1A1)| (0<<COM1A0) |     //normal operation for com1a 
           (0<<COM1B1)| (0<<COM1B0) |     //normal operation for channel b 
           (0<<FOC1A) | (0<<FOC1B)  |     //channel a and b output disabled 
		       (0<<WGM11) | (0<<WGM10);		  	//wgm13..0=0b0100, CTC mode 
	
	TCCR1B = (0<<ICNC1)|					                  //input capture noise canceler: disabled 
			     (0<<ICES1)|					                  //input capture edge select: falling edge 
			     (0<<WGM13)| (1<<WGM12) |			          //CTC mode 
			     (0<<CS12) | (0<<CS11)  | (1<<CS10);		//cs2..0=0b001, timer activated, 1:1 pre-scaler 

	OCR1A = duration;					          	//set the top of CTC on channel a 
	TCNT1 = 0;							              //reset timer counter 
	TIMSK1 |= (1<<OCIE1A);					    	//output compare interrupt for channel a enabled 
	
	/* 
	 * clear v_sw_timer_ms[] array
	 */
	uint8_t id;
	for (id = 0; id < c_SYS_MAXSWTIMERS; id++)
	{
		v_sw_timer_ms[id] = 0;
	}
}

void f_kernel_tick(void) {  // SysTick
	uint16_t v_temp;

	if ((v_sys_state & bm_SYSTICK) != 0) {

		CLR_REG(v_sys_state, bm_SYSTICK);

		++v_sys_timer;

		if (v_sys_timer == c_SYS_TIMEMAX) {
			v_sys_timer = 0;
		}

		if ( (v_sys_timer % c_APP_TICKS) == 0 ) {
			SET_REG(v_sys_state, bm_APP1TICK);
			SET_REG(v_sys_state, bm_APP2TICK);
		}

		for ( v_temp=0;v_temp<c_SYS_MAXSWTIMERS;++v_temp ) {
      
			if ( v_sw_timer_ms[v_temp] != 0 ) {
				--v_sw_timer_ms[v_temp];
			}
		}
	}
	
}

void f_init_kernel(unsigned long systick_freq)
{
	ZERO_REG(v_sys_state);
	
	f_init_systick_timer(systick_freq); // 1ms SysTick
	
	v_app_init_cb_idx = 0;
}

void f_app_tick(void) {

	if ((v_sys_state & bm_APP1TICK) != 0) {

		uint8_t i;
		for (i = 0; i < v_app_init_cb_idx; i++)
		{
      (*fp_app_cb_arr[i])(NULL);
		}			
    
		CLR_REG(v_sys_state, bm_APP1TICK);
	}

}

void f_init_apps(void) {
  uint8_t i;
  for (i = 0; i < v_app_init_cb_idx; i++) {
    (*fp_app_init_cb_arr[i])(NULL);
  }
}

/***************************************/

void f_InitADC()
{
  ADMUX=(1<<REFS0);                                     // For Aref=AVcc;
  ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);    //Rrescalar div factor =128
}

uint16_t f_ReadADC(uint8_t ch)
{
  /* Select ADC Channel ch must be 0-7 */
  ch=ch&0b00000111;
  ADMUX|=ch;

  /* Start Single conversion */
  ADCSRA|=(1<<ADSC);

  /* f_wait for conversion to complete */
  while(!(ADCSRA & (1<<ADIF)));

   /* 
    * Clear ADIF by writing one to it
    * Note you may be wondering why we have write one to clear it
    * This is standard way of clearing bits in io as said in datasheets.
    * The code writes '1' but it result in setting bit to '0' !!!
    */

  ADCSRA|=(1<<ADIF);

  return(ADC);
}


void f_wait()
{
   uint8_t i;
   for(i=0;i<20;i++) {
    _delay_loop_2(0);
   }    
}

/***************************************/

void f_print_startup_msg() {
	f_lcd_put_str("Kush v-0.1 beta");
	
	f_uart_put_str("Kush v-0.1 beta");
	f_uart_new_line();

	f_uart_put_str("Co-operative kernel for AVR family");
	f_uart_new_line();

}

/************** main *******************/
int main(void) {

	/* Use code snippets, CTRL+K, CTRL+X */
	
	//unsigned char adc_val = 0;
	//char buffer[128];

  f_init_kernel(F_CPU/1000);

	f_config_uart();
	
	f_lcd_init(5, 4, 3, 2, 1, 0, LCD_PIN_NC, LCD_PIN_NC, LCD_PIN_NC, LCD_PIN_NC);
	
	f_lcd_set_cursor(0, 0);
	
  f_print_startup_msg();
	
	DELAY_MS(2500);
	
	f_uart_clrscr();
	
	f_lcd_clear();

	//f_InitADC();
	
	f_init_dbg(FALSE);

  f_reg_app(&f_init_app_led, &f_run_app);
  f_reg_uart_cb(&f_app_uart_cb, 'c');
	
	f_init_apps();

	sei();

	for(;;) {
		f_kernel_tick();
		
		f_app_tick();

		f_debugger();
				
		//adc_val = f_ReadADC(1);
		
		//sprintf(buffer, "ADC value is: %d", adc_val);
		
		//f_uart_put_str(buffer);
		//f_uart_new_line();
	}
    
	/*
	DDRB = 0xFF;
	while(1) {
	  f_uart_put_str("HELLO");
	  
	  CLR_PIN(PORTB, 5);
	
	  DELAY_MS(1000);
	
	  SET_PIN(PORTB, 5);
	
	  DELAY_MS(1000);
	
	  CLR_PIN(PORTB, 5);
	
	  DELAY_MS(1000);
	
	  SET_PIN(PORTB, 5);
	
	  DELAY_MS(1000);
    }	 
	*/
	return 0;
}
