#include "board.h"
#include "adc_dma.h"
#include "timers.h"
#include "motor_tb6612.h"
#include "line_sensors.h"
#include "control.h"
#include "button.h"

int main(void)
{
  Clocks_EnableAll();

  Sensors_GPIO_InitAnalog(); // PA0..PA7
  Motor_InitGPIO_PWM();      // PB8..PB12 + STBY
  Button_Init();             // PB13

  TIM4_PWM_Init();     // PWM 18 kHz
  ADC_DMA_Init();      // ADC + DMA
  TIM3_Trigger_Init(); // TRGO 2 kHz
  ADC_DMA_Start();

  LineSensors_CalibInit();
  Control_Init();

  /* CẤU HÌNH ROBOT CHỈ CHẠY VỚI LINE ĐEN HOẶC ĐỎ */
  Control_SetRequiredLineColor(LINE_BLACK | LINE_RED); // Chấp nhận cả đen và đỏ

  /* CHẾ ĐỘ TEST MOTOR: Nhấn giữ nút trong 3 giây khi khởi động */
  uint32_t start_time = 0;
  while (start_time < 3000)
  {
    if (BTN_IS_PRESSED())
    {
      Control_SetMotorTestMode(1); // Bật chế độ test motor
      break;
    }
    start_time++;
    for (volatile int i = 0; i < 10000; i++)
      ; // Delay ~1ms
  }

  TIM2_Control_IRQ_Init(); // 1 kHz: Button + Control

  Button_SetRunEnabled(0); // khởi đầu: dừng

  /* VÒNG LẶP CHÍNH: KIỂM TRA LINE HỢP LỆ TRƯỚC KHI CHO PHÉP START */
  while (1)
  {
    /* Chỉ cho phép start khi:
       1. Nhấn button PB13
       2. Phát hiện line đen hoặc đỏ */
    if (BTN_IS_PRESSED() && !Button_RunEnabled())
    {
      // Kiểm tra có line hợp lệ không
      if (Control_GetLineDetected())
      {
        Button_SetRunEnabled(1); // Cho phép chạy

        // Delay để tránh bounce
        for (volatile int i = 0; i < 100000; i++)
          ;

        // Chờ thả button
        while (BTN_IS_PRESSED())
        {
          for (volatile int i = 0; i < 1000; i++)
            ;
        }
      }
      else
      {
        // Không có line hợp lệ - delay ngắn rồi thử lại
        for (volatile int i = 0; i < 50000; i++)
          ;
        while (BTN_IS_PRESSED())
        {
          for (volatile int i = 0; i < 1000; i++)
            ;
        }
      }
    }

    /* Dừng khi nhấn button lần nữa */
    if (BTN_IS_PRESSED() && Button_RunEnabled())
    {
      Button_SetRunEnabled(0); // Dừng

      // Delay và chờ thả button
      for (volatile int i = 0; i < 100000; i++)
        ;
      while (BTN_IS_PRESSED())
      {
        for (volatile int i = 0; i < 1000; i++)
          ;
      }
    }

    __WFI();
  }
}
