/*
 *  kernel.c
 *  simple kernel for AVRs. currently for atmega32
 *  Created: 01-12-2013 08:14:27
 *  Author: JP
 */ 


#include "kernel.h"
#include "lcd.h"
#include "app.h"

/*** App settings ***/
#define APP_SWITCH_PIN		2
#define APP_LED_PIN			5

typedef enum {
	APP_STATE_IDLE,
	APP_STATE_ON
} app_state_t;

volatile app_state_t v_app_state;

/********************/

volatile uint16_t v_sys_state;		// the internal kernel status variable
volatile uint16_t v_sys_timer;		// the kernel timer count variable

uint16_t v_sw_timer_ms[c_SYS_SWTIMERSMAX];

/*
 * currently arguments are not supported
 * args should be NULL
 */
typedef void(*fp_app_init_cb)(void*);

void (*fp_app_init_cb_arr[c_MAX_APPS])(void*);
uint8_t v_app_init_cb_idx;

/*
 * TODO: add framework for callbacks to run apps
 */
typedef void(*fp_app_cb)(void*);

void (*fp_app_cb_arr[c_MAX_APPS])(void*);

/*
 * Replace index variables with bitmasks.
 */
uint8_t v_app_cb_idx;

/* Timer interrupt routine */
//void SIG_OUTPUT_COMPARE1A( void ) {
ISR(TIMER1_COMPA_vect) {
  SET_REG_BIT(v_sys_state, bv_SYSTICK);
	
  /* handle overflow */
	if (v_sys_timer == 0xFFFF) {
		SET_REG(v_sys_timer, 0x0000);
	}
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
	 * TODO: clear v_sw_timer_ms[] array
	 */
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

		for ( v_temp=0;v_temp<c_SYS_SWTIMERSMAX;++v_temp ) {
      
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
}

void f_app_tick(void) {

	if ((v_sys_state & bm_APP1TICK) != 0) {

		f_run_app();
    
		CLR_REG(v_sys_state, bm_APP1TICK);
	}

}

boolean f_reg_app_init_cb(fp_app_init_cb hook) {
  
  if (hook == NULL) {
    return FALSE;
  }
  
  if (v_app_init_cb_idx == c_MAX_APPS) {
    /* cannot register anymore callbacks */
    return FALSE;
  }
  
  fp_app_init_cb_arr[v_app_init_cb_idx] = hook;
  v_app_init_cb_idx += 1;
  
  return TRUE;
}

void f_init_app(void) {
	v_app_state = APP_STATE_IDLE;

  uint8_t i;
  for (i = 0; i > c_MAX_APPS; i++) {
    (*fp_app_init_cb_arr[i])(NULL);
  }

	v_sw_timer_ms[c_APP_TIMERID] = 0;
}

boolean v_app_trigger;

void f_init_app_led(void* data) {
  v_app_trigger = FALSE;
	OUT_PORT(DDRB);
	ZERO_PORT(PORTB);
}  

void f_app_uart_cb(void* data) {
	v_app_trigger = TRUE;
}

void f_run_app(void) {

	switch (v_app_state) {

		case APP_STATE_IDLE:
		{
      ////if(GET_PIN(PINB, APP_SWITCH_PIN) == 0) {
      //if (v_sw_timer_ms[c_APP_TIMERID] == 0) {
		  if (v_app_trigger == TRUE) {
        v_app_state = APP_STATE_ON;
        v_app_trigger = FALSE;

        //SET_PIN(PORTB,APP_LED_PIN);

        v_sw_timer_ms[c_APP_TIMERID] = TIMER_VAL_FROM_SEC(3); // App timer for 3 second

        f_uart_put_str_ext("APP state ON");
        f_uart_new_line_ext();
				
        f_lcd_clear();
        f_lcd_put_str("APP state ON");
 
			}
			break;
		}    
		case APP_STATE_ON:
			if (v_sw_timer_ms[c_APP_TIMERID] == 0) {
				v_app_state = APP_STATE_IDLE;

				//CLR_PIN(PORTB,APP_LED_PIN);
				
				//v_sw_timer_ms[c_APP_TIMERID] = TIMER_VAL_FROM_SEC(3); // App timer for 3 second
				
				f_uart_put_str_ext("APP state OFF");
				f_uart_new_line_ext();
				
				f_lcd_clear();
				f_lcd_put_str("APP state OFF");
			}
			break;

		default:
			;
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

/************** main *******************/
int main(void) {
	
	/**
	 * TODO: Run a task for UART and have a buffer for tasks
	 * to read or write data.
	 */
	
	//unsigned char adc_val = 0;
	//char buffer[128];

  f_init_kernel(F_CPU/1000);

	f_config_uart();
	
	f_lcd_init(5, 4, 3, 2, 1, 0, LCD_PIN_NC, LCD_PIN_NC, LCD_PIN_NC, LCD_PIN_NC);
	
	f_lcd_set_cursor(0, 0);
	
	f_lcd_put_str("Kush v-0.1 beta");
	
	f_uart_put_str_ext("Kush");
	f_uart_new_line_ext();

	f_uart_put_str_ext("Co-operative kernel for AVR family");
	f_uart_new_line_ext();
	
	f_uart_put_str_ext("v - 0.1 beta");
	f_uart_new_line_ext();
	
	DELAY_MS(1500);
	
	f_lcd_clear();

	//f_InitADC();
	
	f_init_dbg(FALSE);

  f_reg_app_init_cb(&f_init_app_led);
  f_reg_uart_cb(&f_app_uart_cb, 'c');
	f_init_app();

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
