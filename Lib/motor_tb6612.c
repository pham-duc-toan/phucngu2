#include "motor_tb6612.h"

/* H? s? hi?u chu?n motor - m?c ??nh c? 2 bánh nh? nhau */
static float motor_left_factor = 1.0f;
static float motor_right_factor = 1.0f;

static inline void set_dir_left(int forward){
  if(forward){
    DIRA1_PORT->BSRR = (1U<<DIRA1_PIN);
    BSRR_RESET(DIRA2_PORT, DIRA2_PIN);
  } else {
    BSRR_RESET(DIRA1_PORT, DIRA1_PIN);
    DIRA2_PORT->BSRR = (1U<<DIRA2_PIN);
  }
}
static inline void set_dir_right(int forward){
  if(forward){
    DIRB1_PORT->BSRR = (1U<<DIRB1_PIN);
    BSRR_RESET(DIRB2_PORT, DIRB2_PIN);
  } else {
    BSRR_RESET(DIRB1_PORT, DIRB1_PIN);
    DIRB2_PORT->BSRR = (1U<<DIRB2_PIN);
  }
}

void Motor_InitGPIO_PWM(void){
  /* PB8..PB12 output push-pull 50MHz */
  GPIOB->CRH &= ~((0xF<<((8-8)*4))|(0xF<<((9-8)*4))|(0xF<<((10-8)*4))|(0xF<<((11-8)*4))|(0xF<<((12-8)*4)));
  GPIOB->CRH |=  ((0x3<<((8-8)*4))|(0x3<<((9-8)*4))|(0x3<<((10-8)*4))|(0x3<<((11-8)*4))|(0x3<<((12-8)*4)));

  /* B?t STBY */
  STBY_PORT->BSRR = (1U<<STBY_PIN);
}

void Motor_SetForward(void){
  set_dir_left(1); set_dir_right(1);
}

void Motor_WritePWM(int left, int right){
  TIM4->CCR1 = clamp_u16(left,  0, TIM4->ARR);
  TIM4->CCR2 = clamp_u16(right, 0, TIM4->ARR);
}

void Motor_SetCalibration(float left_factor, float right_factor){
  motor_left_factor = left_factor;
  motor_right_factor = right_factor;
}

void Motor_WritePWM_Calibrated(int left, int right){
  /* áp dụng hệ số hiệu chuẩn */
  int left_cal = (int)(left * motor_left_factor);
  int right_cal = (int)(right * motor_right_factor);
  
  /* Giới hạn giá trị */
  TIM4->CCR1 = clamp_u16(left_cal,  0, TIM4->ARR);
  TIM4->CCR2 = clamp_u16(right_cal, 0, TIM4->ARR);
}
