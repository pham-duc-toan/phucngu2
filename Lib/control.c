#include "control.h"
#include "pid.h"
#include "adc_polling.h" // Thay thế adc_dma
#include "line_sensors.h"
#include "line_params.h" // File tham số để dễ tuning
#include "motor_tb6612.h"
#include "button.h"
#include "system_state.h" // State machine system

static PID_t pid = {
    .Kp = 1.5f, .Ki = 0.05f, .Kd = 0.08f, .Ts = 1.0f / CONTROL_HZ, .I = 0, .D = 0, .last_e = 0, .d_alpha = 0.85f, .u_min = -1500.0f, .u_max = 1500.0f};

/* Slew-rate limiter cho PWM (counts/ms) */
static const uint16_t CCR_SLEW_MAX = 80;
static uint16_t c1_now = 0, c2_now = 0, c1_tg = 0, c2_tg = 0;

/* Chế độ test motor */
static uint8_t motor_test_mode = 0;
/* Chế độ tốc độ cao - có thể bật/tắt */
static uint8_t high_speed_mode = 1; // Mặc định bật tốc độ cao

/* Debug variables - để kiểm tra trong debugger */
static SensorDebug_t debug_info;
static uint16_t debug_snap[N_CH];
static uint32_t debug_counter = 0;
static uint8_t line_detected = 0; // Trạng thái phát hiện line

void Control_SetMotorTestMode(uint8_t enable)
{
  motor_test_mode = enable;
}

void Control_SetHighSpeedMode(uint8_t enable)
{
  high_speed_mode = enable;
}

uint8_t Control_GetLineDetected(void)
{
  // SIMPLE: chỉ cần sensor có phát hiện gì đó là coi như có line
  return (debug_info.is_valid != 0);
}

void Control_DebugInfo(void)
{
  // Function for debugging - có thể gọi trong debugger để xem thông tin
  // debug_info chứa thông tin sensor
  // debug_snap chứa raw ADC values
  // debug_counter = số lần gọi Control_Loop_1kHz
}

void Control_Init(void)
{
  PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);
  LineSensors_CalibInit();
  Motor_SetForward();

  /* HIỆU CHUẨN MOTOR - ĐIỀU CHỈNH CÁC HỆ SỐ NÀY ĐỂ CÂN BẰNG 2 BÁNH */
  /* Nếu bánh trái nhanh hơn: giảm left_factor
     Nếu bánh phải nhanh hơn: giảm right_factor */
  Motor_SetCalibration(1.0f, 1.0f); // Bắt đầu với 1.0 (không hiệu chuẩn)
}

void Control_Loop_1kHz(void)
{
  /* Update state machine */
  SystemState_Update();

  /* Snapshot ADC - đọc trực tiếp thay vì dùng DMA */
  uint16_t snap[N_CH];
  ADC_ReadAllChannels(snap);

  /* DEBUG: Copy data cho debug */
  debug_counter++;
  for (int i = 0; i < N_CH; i++)
    debug_snap[i] = snap[i];
  debug_info = LineSensors_GetDebugInfo(snap);

  /* Handle different states */
  SystemState_t current_state = SystemState_GetCurrent();

  switch (current_state)
  {
  case STATE_INIT:
  case STATE_STANDBY:
  case STATE_STOPPED:
    // Dừng motor
    c1_tg = c2_tg = 0;
    c1_now = c2_now = 0;
    /* Reset PID khi dừng */
    PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);
    break;

  case STATE_SCAN:
    // Chế độ scan - motor dừng, chỉ cập nhật sensor calibration
    c1_tg = c2_tg = 0;
    c1_now = c2_now = 0;

    // Cập nhật calibration
    LineSensors_UpdateCalib(snap);
    break;

  case STATE_RUNNING:
    // Chế độ chạy - xử lý line following với tốc độ 3000

    if (motor_test_mode)
    {
      /* CHẾ ĐỘ TEST MOTOR - chạy thẳng với tốc độ 3000 */
      uint16_t base_speed = SystemState_GetBaseSpeed();
      c1_tg = c2_tg = base_speed;
    }
    else
    {
      // *** LOGIC KIỂM TRA LINE CHẶT CHẼ ***
      line_detected = debug_info.is_valid;

      // THÊM KIỂM TRA KHẮT KHE: Không có line nếu contrast quá thấp
      if (debug_info.contrast < MIN_CONTRAST_PARAM)
      {
        line_detected = 0;
      }

      // THÊM KIỂM TRA: Không có line nếu tất cả sensor đều cao (nền trắng)
      if (debug_info.avg_all > MAX_AVG_WHITE_PARAM)
      {
        line_detected = 0;
      }

      // KIỂM TRA CUỐI CÙNG: Tính error và nếu = 0 thì chắc chắn không có line
      int test_error = LineSensors_ComputeError(snap);
      if (test_error == 0)
      {
        line_detected = 0;
      }

      if (!line_detected)
      {
        // KHÔNG CÓ LINE HỢP LỆ - DỪNG NGAY
        c1_tg = c2_tg = 0;
        return;
      }

      // SIMPLE LINE FOLLOWING logic - với tốc độ 60%
      int e = LineSensors_ComputeError(snap);

      /* Dừng nếu error = 0 (mất line hoàn toàn) */
      if (e == 0)
      {
        c1_tg = c2_tg = 0;
        float u = PID_Update(&pid, (float)e);
        uint16_t base_speed = SystemState_GetBaseSpeed(); // 3000

        /* BASE điều chỉnh với tốc độ 3000 */
        int BASE, LOW_SPEED, MED_SPEED, HIGH_SPEED, ULTRA_HIGH_SPEED;

        if (high_speed_mode)
        {
          // Chế độ tốc độ cao với base 3000
          BASE = base_speed;                   // 3000
          ULTRA_HIGH_SPEED = base_speed + 800; // 3800
          HIGH_SPEED = base_speed + 400;       // 3400
          MED_SPEED = base_speed;              // 3000
          LOW_SPEED = base_speed - 600;        // 2400
        }
        else
        {
          // Chế độ tốc độ bình thường
          BASE = base_speed - 1200;            // 1800
          ULTRA_HIGH_SPEED = base_speed - 800; // 2200
          HIGH_SPEED = base_speed - 1000;      // 2000
          MED_SPEED = base_speed - 1400;       // 1600
          LOW_SPEED = base_speed - 1800;       // 1200
        }

        int ae = (e < 0) ? -e : e;

        if (ae < 500)
          BASE = ULTRA_HIGH_SPEED;
        else if (ae < 1000)
          BASE = HIGH_SPEED;
        else if (ae < 2000)
          BASE = MED_SPEED;
        else
          BASE = LOW_SPEED;

        // Calculate motor speeds with safe clamping
        int temp_c1 = BASE - (int)u;
        int temp_c2 = BASE + (int)u;

        if (temp_c1 < 0)
          temp_c1 = 0;
        if (temp_c2 < 0)
          temp_c2 = 0;
        if (temp_c1 > 4095)
          temp_c1 = 4095;
        if (temp_c2 > 4095)
          temp_c2 = 4095;

        c1_tg = (uint16_t)temp_c1;
        c2_tg = (uint16_t)temp_c2;
      }
      break;
    }

    /* Slew-rate về target */
    uint16_t CCR_SLEW_CURRENT = CCR_SLEW_MAX;
    int d1 = (int)c1_tg - (int)c1_now;
    if (d1 > (int)CCR_SLEW_CURRENT)
      d1 = CCR_SLEW_CURRENT;
    if (d1 < -(int)CCR_SLEW_CURRENT)
      d1 = -(int)CCR_SLEW_CURRENT;
    c1_now = (uint16_t)((int)c1_now + d1);

    int d2 = (int)c2_tg - (int)c2_now;
    if (d2 > (int)CCR_SLEW_CURRENT)
      d2 = CCR_SLEW_CURRENT;
    if (d2 < -(int)CCR_SLEW_CURRENT)
      d2 = -(int)CCR_SLEW_CURRENT;
    c2_now = (uint16_t)((int)c2_now + d2);

    /* Ghi PWM - sử dụng calibrated để cân bằng motor */
    Motor_WritePWM_Calibrated(c1_now, c2_now);
  }