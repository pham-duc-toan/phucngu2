#include "button.h"

static uint8_t  btn_prev = 1;
static uint16_t btn_deb = 0, btn_hold = 0;
static uint8_t  btn_long_fired = 0;
static uint8_t  run_enabled = 0;

void Button_Init(void){
  /* PB13 input pull-up: MODE=00, CNF=10 */
  GPIOB->CRH &= ~(0xF << ((13-8)*4));
  GPIOB->CRH |=  (0x8 << ((13-8)*4));
  GPIOB->ODR |= (1U<<BTN_PIN);
}

uint8_t Button_RunEnabled(void){ return run_enabled; }
void    Button_SetRunEnabled(uint8_t en){ run_enabled = en; }

/* G?i m?i 1ms trong TIM2 IRQ */
void Button_Task_1ms(void){
  const uint16_t DB_MS=20, LONG_MS=1200;
  uint8_t level = BTN_IS_PRESSED()?0:1;

  if(level != btn_prev){
    if(++btn_deb >= DB_MS){
      btn_prev = level; btn_deb=0;
      if(level==0){ btn_hold=0; btn_long_fired=0; }
      else{ if(!btn_long_fired) run_enabled = 1; } // nh?n ng?n ? START
    }
  }else{
    btn_deb = 0;
    if(level==0){
      if(btn_hold<60000) btn_hold++;
      if(!btn_long_fired && btn_hold>=LONG_MS){
        run_enabled = 0;             // nh?n gi? ? STOP
        btn_long_fired = 1;
      }
    }
  }
}
