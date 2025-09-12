#include "board.h"
#include "adc_dma.h"
#include "timers.h"
#include "motor_tb6612.h"
#include "line_sensors.h"
#include "control.h"
#include "button.h"

int main(void){
  Clocks_EnableAll();

  Sensors_GPIO_InitAnalog();   // PA0..PA7
  Motor_InitGPIO_PWM();        // PB8..PB12 + STBY
  Button_Init();               // PB13

  TIM4_PWM_Init();             // PWM 18 kHz
  ADC_DMA_Init();              // ADC + DMA
  TIM3_Trigger_Init();         // TRGO 2 kHz
  ADC_DMA_Start();

  LineSensors_CalibInit();
  Control_Init();

  /* CH? ?? TEST MOTOR: Nh?n gi? nút trong 3 giây khi kh?i ??ng */
  uint32_t start_time = 0;
  while(start_time < 3000){
    if(BTN_IS_PRESSED()){
      Control_SetMotorTestMode(1);  // B?t ch? ?? test motor
      break;
    }
    start_time++;
    for(volatile int i=0; i<10000; i++);  // Delay ~1ms
  }

  TIM2_Control_IRQ_Init();     // 1 kHz: Button + Control

  Button_SetRunEnabled(0);     // kh?i d?u: d?ng, nh?n PB13 d? START

  while(1){ __WFI(); }
}
