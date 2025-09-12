#include "control.h"
#include "pid.h"
#include "adc_dma.h"
#include "line_sensors.h"
#include "motor_tb6612.h"
#include "button.h"

static PID_t pid = {
  .Kp=1.5f, .Ki=0.05f, .Kd=0.08f, .Ts=1.0f/CONTROL_HZ,
  .I=0, .D=0, .last_e=0, .d_alpha=0.85f, .u_min=-1500.0f, .u_max=1500.0f
};

/* Slew-rate limiter cho PWM (counts/ms) */
static const uint16_t CCR_SLEW_MAX = 80;
static uint16_t c1_now=0, c2_now=0, c1_tg=0, c2_tg=0;

/* Ch? ?? test motor */
static uint8_t motor_test_mode = 0;
/* Ch? ?? t?c ?? cao - có th? b?t/t?t */
static uint8_t high_speed_mode = 1;  // M?c ??nh b?t t?c ?? cao

void Control_SetMotorTestMode(uint8_t enable){
  motor_test_mode = enable;
}

void Control_SetHighSpeedMode(uint8_t enable){
  high_speed_mode = enable;
}

void Control_Init(void){
  PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);
  LineSensors_CalibInit();
  Motor_SetForward();
  
  /* HI?U CHU?N MOTOR - ?I?U CH?NH C�C H? S? N�Y ?? C�N B?NG 2 B�NH */
  /* N?u bánh tr�i nhanh h?n: gi?m left_factor
     N?u bánh ph?i nhanh h?n: gi?m right_factor */
  Motor_SetCalibration(1.0f, 1.0f);  // B?t ??u v?i 1.0 (kh?ng hi?u chu?n)
}

void Control_Loop_1kHz(void){
  /* Snapshot ADC */
  uint16_t snap[N_CH];
  for(int i=0;i<N_CH;i++) snap[i]=adcBuf[i];

  /* ~8s d?u t? hi?u chu?n - t?ng th?i gian ?? hi?u chu?n t?t h?n */
  static uint32_t t=0;
  static uint8_t calib_done = 0;
  
  if(t<8000 && !calib_done) {
    LineSensors_UpdateCalib(snap);
    if(t > 3000 && LineSensors_CalibQuality()) {
      calib_done = 1;  // Hi?u chu?n s?m n?u ch?t l??ng ?? t?t
    }
  }
  t++;

  if(!Button_RunEnabled()){
    c1_tg=c2_tg=0;
    /* Reset PID khi d?ng */
    PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);
  }else if(motor_test_mode){
    /* CH? ?? TEST MOTOR - ch?y th?ng v?i t?c ?? cao ?? ki?m tra c�n b?ng */
    c1_tg = c2_tg = 3000;  // T?ng t? 1500 l�n 3000 (~75% max)
  }else{
    int e = LineSensors_ComputeError(snap);
    float u = PID_Update(&pid, (float)e);

    /* BASE ?i?u ch?nh linh ho?t theo l?i - T?NG T?C ?? L�N G?N MAX */
    int BASE, LOW_SPEED, MED_SPEED, HIGH_SPEED;
    
    if(high_speed_mode) {
      // Ch? ?? t?c ?? cao
      BASE = 3200; HIGH_SPEED = 3600; MED_SPEED = 2800; LOW_SPEED = 2400;
    } else {
      // Ch? ?? t?c ?? b?nh th??ng
      BASE = 1600; HIGH_SPEED = 1800; MED_SPEED = 1400; LOW_SPEED = 1200;
    }
    
    int abs_e = (e >= 0) ? e : -e;
    
    if (abs_e > 1800) {
        BASE = LOW_SPEED;   // T?c ?? th?p khi l?i l?n (cua g?t)
    } else if (abs_e > 1200) {
        BASE = MED_SPEED;   // T?c ?? trung b?nh
    } else if (abs_e < 300) {
        BASE = HIGH_SPEED;  // T?c ?? cao khi ?? ch?nh x?c
    }

    int L = BASE - (int)u;
    int R = BASE + (int)u;

    if(L<0)L=0; if(L>(int)TIM4->ARR)L=TIM4->ARR;
    if(R<0)R=0; if(R>(int)TIM4->ARR)R=TIM4->ARR;

    Motor_SetForward();
    c1_tg=(uint16_t)L; c2_tg=(uint16_t)R;
  }

  /* Slew-rate v? target - ?i?u ch?nh cho t?c ?? cao */
  uint16_t CCR_SLEW_CURRENT = 100;  // T?ng t? 60 l�n 100 cho thay ??i nhanh h?n
  int d1=(int)c1_tg-(int)c1_now; if(d1> (int)CCR_SLEW_CURRENT)d1= CCR_SLEW_CURRENT; if(d1<-(int)CCR_SLEW_CURRENT)d1=-(int)CCR_SLEW_CURRENT; c1_now=(uint16_t)((int)c1_now+d1);
  int d2=(int)c2_tg-(int)c2_now; if(d2> (int)CCR_SLEW_CURRENT)d2= CCR_SLEW_CURRENT; if(d2<-(int)CCR_SLEW_CURRENT)d2=-(int)CCR_SLEW_CURRENT; c2_now=(uint16_t)((int)c2_now+d2);

  Motor_WritePWM_Calibrated(c1_now, c2_now);  // S? d?ng PWM có hi?u chu?n
}
