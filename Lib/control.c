#include "control.h"
#include "pid.h"
#include "adc_dma.h"
#include "line_sensors.h"
#include "motor_tb6612.h"
#include "button.h"

static PID_t pid = {
    .Kp = 1.5f, .Ki = 0.05f, .Kd = 0.08f, .Ts = 1.0f / CONTROL_HZ, .I = 0, .D = 0, .last_e = 0, .d_alpha = 0.85f, .u_min = -1500.0f, .u_max = 1500.0f};

/* Slew-rate limiter cho PWM (counts/ms) */
static const uint16_t CCR_SLEW_MAX = 80;
static uint16_t c1_now = 0, c2_now = 0, c1_tg = 0, c2_tg = 0;

/* Ch? ?? test motor */
static uint8_t motor_test_mode = 0;
/* Ch? ?? t?c ?? cao - có th? b?t/t?t */
static uint8_t high_speed_mode = 1; // M?c ??nh b?t t?c ?? cao

/* Biến theo dõi line color */
static uint8_t current_line_color = LINE_NONE;
static uint8_t required_line_color = LINE_BLACK; // FORCE CHỈ LINE ĐEN
static uint8_t line_detected = 0;
static uint32_t lost_valid_line_time = 0;

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
  return line_detected;
}

uint8_t Control_GetLineColor(void)
{
  return current_line_color;
}

void Control_SetRequiredLineColor(uint8_t color)
{
  required_line_color = color;
}

/* DEBUG FUNCTION - gọi từ debugger để kiểm tra trạng thái */
void Control_DebugInfo(void)
{
  // Đặt breakpoint ở đây và check các biến:
  // debug_info.raw_values[0..7] - giá trị ADC của 8 sensors
  // debug_info.avg_all - trung bình tất cả sensors
  // debug_info.contrast - độ tương phản
  // debug_info.active_sensors - số sensors active
  // debug_info.detected_color - màu được detect (0=NONE, 1=BLACK, 2=RED)
  // debug_info.is_valid - có line hợp lệ không
  // current_line_color - màu hiện tại
  // line_detected - có line được detect không
  // Button_RunEnabled() - robot có được enable không
  // c1_tg, c2_tg - target PWM values
  volatile int dummy = 0; // Để compiler không optimize
}

void Control_Init(void)
{
  PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);
  LineSensors_CalibInit();
  Motor_SetForward();

  /* HI?U CHU?N MOTOR - ?I?U CH?NH C�C H? S? N�Y ?? C�N B?NG 2 B�NH */
  /* N?u bánh tr�i nhanh h?n: gi?m left_factor
     N?u bánh ph?i nhanh h?n: gi?m right_factor */
  Motor_SetCalibration(1.0f, 1.0f); // B?t ??u v?i 1.0 (kh?ng hi?u chu?n)
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

  /* Phát hiện màu line và kiểm tra tính hợp lệ */
  current_line_color = LineSensors_DetectLineColor(snap);
  // TEMP SIMPLE cho debug - chỉ cần có line là chạy
  line_detected = LineSensors_HasValidLine(snap);
  // Original: line_detected = LineSensors_HasValidLine(snap) && ((current_line_color & required_line_color) != 0);

  /* SAFETY OVERRIDE: Nếu không phải line đen thì FORCE STOP ngay */
  // TEMP DISABLE cho debug
  /*if (current_line_color != LINE_BLACK)
  {
    line_detected = 0; // Force không detect
  }*/

  /* ULTIMATE SAFETY: HARD CHECK cho giấy trắng */
  uint32_t avg_all = 0;
  for (int i = 0; i < N_CH; i++)
  {
    avg_all += snap[i];
  }
  avg_all /= N_CH;

  // CHỈ force stop nếu THỰC SỰ là giấy trắng (tất cả sensors đều cao)
  if (avg_all > 3000) // Tăng threshold từ 2000 lên 3000
  {
    line_detected = 0;
    current_line_color = LINE_NONE;
  }

  /* ~8s đầu tự hiệu chuẩn - tăng thời gian để hiệu chuẩn tốt hơn */
  static uint32_t t = 0;
  static uint8_t calib_done = 0;

  if (t < 8000 && !calib_done)
  {
    LineSensors_UpdateCalib(snap);
    if (t > 3000 && LineSensors_CalibQuality())
    {
      calib_done = 1; // Hiệu chuẩn sớm nếu chất lượng đủ tốt
    }
  }
  t++;

  if (!Button_RunEnabled())
  {
    c1_tg = c2_tg = 0;
    c1_now = c2_now = 0; // Force current về 0
    lost_valid_line_time = 0;
    /* Reset PID khi dừng */
    PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);
  }
  else if (motor_test_mode)
  {
    /* CHẾ ĐỘ TEST MOTOR - chạy thẳng với tốc độ cao để kiểm tra cân bằng */
    c1_tg = c2_tg = 3000; // Tăng từ 1500 lên 3000 (~75% max)
  }
  else
  {
    /* KIỂM TRA LINE HỢP LỆ TRƯỚC KHI CHẠY */
    if (!line_detected)
    {
      lost_valid_line_time++;

      /* Dừng robot ngay lập tức nếu không có line hợp lệ */
      c1_tg = c2_tg = 0;   // Dừng hoàn toàn
      c1_now = c2_now = 0; // Force cả current về 0 để bypass slew rate

      /* Disable run nếu mất line hợp lệ quá lâu */
      if (lost_valid_line_time > 2000) // 2 giây
      {
        Button_SetRunEnabled(0); // Tự động disable để bắt buộc user nhấn button lại
        lost_valid_line_time = 0;
      }

      /* Reset PID */
      PID_Init(&pid, pid.Kp, pid.Ki, pid.Kd, pid.Ts, pid.d_alpha, pid.u_min, pid.u_max);

      /* Force write PWM 0 để đảm bảo motor dừng */
      Motor_WritePWM_Calibrated(0, 0);
      return;
    }

    /* Reset counter khi tìm thấy line hợp lệ */
    lost_valid_line_time = 0;

    /* SỬ DỤNG ADVANCED ERROR COMPUTATION */
    int e = LineSensors_ComputeErrorAdvanced(snap);

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

    /* Boost thêm tốc độ cho line màu đen vì dễ detect hơn */
    if (current_line_color == LINE_BLACK && abs_e < 500)
    {
      BASE = (int)(BASE * 1.05f); // Tăng 5% cho black line
    }

    int L = BASE - (int)u;
    int R = BASE + (int)u;

    if (L < 0)
      L = 0;
    if (L > (int)TIM4->ARR)
      L = TIM4->ARR;
    if (R < 0)
      R = 0;
    if (R > (int)TIM4->ARR)
      R = TIM4->ARR;

    Motor_SetForward();
    c1_tg = (uint16_t)L;
    c2_tg = (uint16_t)R;
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

  /* SAFETY CHECK CUỐI CÙNG: Force stop nếu không có line hợp lệ */
  if (!Button_RunEnabled() || (!line_detected && !motor_test_mode))
  {
    c1_now = c2_now = 0;
  }

  /* MEGA KILL SWITCH: HARD STOP nếu không phải line đen */
  // TEMP DISABLE cho debug
  /*if (current_line_color != LINE_BLACK)
  {
    c1_now = c2_now = 0;
  }*/

  /* ULTIMATE KILL SWITCH: HARD STOP nếu ở test mode nhưng không có line */
  // TEMP DISABLE cho debug
  /*if (motor_test_mode && current_line_color != LINE_BLACK)
  {
    c1_now = c2_now = 0;
    motor_test_mode = 0; // Disable test mode
  }*/
  {
    c1_now = c2_now = 0;
  }

  Motor_WritePWM_Calibrated(c1_now, c2_now); // Sử dụng PWM có hiệu chuẩn
}
