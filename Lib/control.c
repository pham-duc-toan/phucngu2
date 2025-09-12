#include "control.h"
#include "pid.h"
#include "adc_dma.h"
#include "line_sensors.h"
#include "line_params.h" // File tham số để dễ tuning
#include "motor_tb6612.h"
#include "button.h"

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
  /* Snapshot ADC */
  uint16_t snap[N_CH];
  for (int i = 0; i < N_CH; i++)
    snap[i] = adcBuf[i];

  /* DEBUG: Copy data cho debug */
  debug_counter++;
  for (int i = 0; i < N_CH; i++)
    debug_snap[i] = snap[i];
  debug_info = LineSensors_GetDebugInfo(snap);

  /* ~8s đầu tự hiệu chuẩn */
  static uint32_t t = 0;
  static uint8_t calib_done = 0;

  if (t < 8000 && !calib_done)
  {
    LineSensors_UpdateCalib(snap);
    if (t > 3000 && LineSensors_CalibQuality())
    {
      calib_done = 1; // Hiệu chuẩn xong sớm nếu đủ tốt
    }
  }
  t++;

  // *** LOGIC KIỂM TRA LINE CHẶT CHẼ - ĐÂY LÀ CHÌA KHÓA ***
  // Update line detection status dựa vào nhiều tiêu chí
  line_detected = debug_info.is_valid;

  // THÊM KIỂM TRA KHẮT KHE: Không có line nếu contrast quá thấp
  if (debug_info.contrast < MIN_CONTRAST_PARAM) // Sử dụng tham số từ file
  {
    line_detected = 0;
  }

  // THÊM KIỂM TRA: Không có line nếu tất cả sensor đều cao (nền trắng)
  if (debug_info.avg_all > MAX_AVG_WHITE_PARAM) // Sử dụng tham số từ file
  {
    line_detected = 0;
  }

  // KIỂM TRA CUỐI CÙNG: Tính error và nếu = 0 thì chắc chắn không có line
  int test_error = LineSensors_ComputeError(snap);
  if (test_error == 0)
  {
    line_detected = 0;
  }

  if (!Button_RunEnabled())
  {
    c1_tg = c2_tg = 0;
    c1_now = c2_now = 0;
    /* Reset PID khi dừng */
    PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);
  }
  else if (motor_test_mode)
  {
    /* CHẾ ĐỘ TEST MOTOR - chạy thẳng với tốc độ cao để kiểm tra cân bằng */
    c1_tg = c2_tg = 3000;
  }
  else
  {
    // *** KIỂM TRA LINE TRƯỚC KHI CHO PHÉP CHẠY ***
    if (!line_detected)
    {
      // KHÔNG CÓ LINE HỢP LỆ - DỪNG NGAY
      c1_tg = c2_tg = 0;
      return; // Thoát khỏi function, không thực hiện PID
    }

    // SIMPLE LINE FOLLOWING logic - chỉ chạy khi có line hợp lệ
    // Tính error từ sensor data (function này có built-in lost line handling)
    int e = LineSensors_ComputeError(snap);

    /* Dừng nếu error = 0 (mất line hoàn toàn) */
    if (e == 0)
    {
      c1_tg = c2_tg = 0;
      return;
    }

    float u = PID_Update(&pid, (float)e);

    /* BASE điều chỉnh linh hoạt theo lỗi - TĂNG TỐC ĐỘ LÊN GẦN MAX */
    int BASE, LOW_SPEED, MED_SPEED, HIGH_SPEED, ULTRA_HIGH_SPEED;

    if (high_speed_mode)
    {
      // Chế độ tốc độ cao - tối ưu cho line tracking
      BASE = 3400;
      ULTRA_HIGH_SPEED = 3800;
      HIGH_SPEED = 3600;
      MED_SPEED = 3000;
      LOW_SPEED = 2400;
    }
    else
    {
      // Chế độ tốc độ bình thường
      BASE = 1800;
      ULTRA_HIGH_SPEED = 2000;
      HIGH_SPEED = 1800;
      MED_SPEED = 1400;
      LOW_SPEED = 1200;
    }

    int abs_e = (e >= 0) ? e : -e;

    /* Adaptive speed với nhiều mức độ chi tiết hơn */
    if (abs_e > 2000)
    {
      BASE = LOW_SPEED; // Cua rất gắt
    }
    else if (abs_e > 1500)
    {
      BASE = MED_SPEED; // Cua gắt
    }
    else if (abs_e > 800)
    {
      BASE = HIGH_SPEED; // Cua nhẹ
    }
    else if (abs_e < 200)
    {
      BASE = ULTRA_HIGH_SPEED; // Đường thẳng hoàn hảo
    }
    else
    {
      BASE = HIGH_SPEED; // Gần như thẳng
    }

    int L = BASE - (int)u;
    int R = BASE + (int)u;

    c1_tg = clamp_u16(L, 0, 4000);
    c2_tg = clamp_u16(R, 0, 4000);
  }

  /* Slew-rate về target - điều chỉnh cho tốc độ cao */
  uint16_t CCR_SLEW_CURRENT = 120; // Tăng từ 100 lên 120 cho response nhanh hơn
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