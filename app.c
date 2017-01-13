/*
 * app.c
 *
 * Created: 24-02-2014 08:35:32
 *  Author: JP
 */ 

#include "kernel.h"
#include "app.h"
#include "uart.h"
#include "lcd.h"

volatile app_state_t v_app_state;

uint8_t v_app_timerid;

void f_app_uart_cb(void* data) {
	SET_REG(v_app_state, bm_APP_TRIGGER);
}

void f_init_app_led(void* data) {

  ZERO_REG(v_app_state);
  v_app_timerid = 0;
  
  /* register sw timer for app */
  v_app_timerid = f_get_free_timer();
  if (v_app_timerid == 0xFF)
  {
    return;
  }
  
  SET_REG_BIT(v_app_state, e_APP_STATE_IDLE);
  v_app_state |= 0x01;
	
  return;
}  

void f_run_app(void* data) {
  
  switch (v_app_state & bm_APP_STATE) {

    case (1 << e_APP_STATE_IDLE):
    {
      if ( (v_app_state & bm_APP_TRIGGER) != 0 ) {
        /* set app state as ON */
			  CLR_REG_BIT(v_app_state, e_APP_STATE_IDLE);
			  SET_REG_BIT(v_app_state, e_APP_STATE_ON);
			  /* clear app trigger */
			  CLR_REG(v_app_state, bm_APP_TRIGGER);

        f_set_timer(v_app_timerid, c_APP_TIMEOUT_MS);

        f_uart_put_str("APP state ON");
        f_uart_new_line();
				
        f_lcd_clear();
        f_lcd_put_str("APP state ON");
			}
			break;
		}    
		case (1 << e_APP_STATE_ON):
		{
			if (f_check_timer(v_app_timerid) == 0) {

				/* set app state as IDLE */
				CLR_REG_BIT(v_app_state, e_APP_STATE_ON);
        SET_REG_BIT(v_app_state, e_APP_STATE_IDLE);
        
        f_uart_put_str("APP state IDLE");
				f_uart_new_line();
				
				f_lcd_clear();
				f_lcd_put_str("APP state IDLE");

        /* if app trigger has been set, clear it */
				cli();
				CLR_REG(v_app_state, bm_APP_TRIGGER);
				sei();				
			}
			break;
		}    
		default:
			;
	}
}
