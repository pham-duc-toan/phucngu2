#include "line_sensors.h"

static uint16_t s_min[N_CH], s_max[N_CH];
static int32_t lastError = 0;

void LineSensors_CalibInit(void)
{
  for (int i = 0; i < N_CH; i++)
  {
    s_min[i] = 4095;
    s_max[i] = 0;
  }
}

void LineSensors_UpdateCalib(const uint16_t *snap)
{
  for (int i = 0; i < N_CH; i++)
  {
    uint16_t r = snap[i];
    if (r < s_min[i])
      s_min[i] = r;
    if (r > s_max[i])
      s_max[i] = r;
  }
}

/* Ki?m tra ch?t l??ng hi?u chu?n - tr? v? 1 n?u t?t, 0 n?u ch?a t?t */
int LineSensors_CalibQuality(void)
{
  int good_channels = 0;
  for (int i = 0; i < N_CH; i++)
  {
    if ((s_max[i] - s_min[i]) > 500)
      good_channels++; // Ng??ng ch?nh l?ch t?i thi?u
  }
  return (good_channels >= (N_CH - 2)); // Ch?p nh?n n?u ?t nh?t 6/8 sensor t?t
}

/* 15% c?a 8000 - gi?m ng??ng nh?y c?m h?n */
#define VSUM_THRESH 1200
/* Ng??ng ?? ph?t hi?n line y?u */
#define VSUM_WEAK_THRESH 800

static uint32_t lostLineCounter = 0;

/* Phát hiện màu line dựa trên giá trị ADC raw */
uint8_t LineSensors_DetectLineColor(const uint16_t *snap)
{
  uint32_t avg_value = 0;
  uint32_t min_value = 4095;
  uint32_t max_value = 0;
  uint8_t active_sensors = 0;

  // Tính trung bình tất cả sensors và tìm min/max
  for (int i = 0; i < N_CH; i++)
  {
    uint16_t raw = snap[i];
    avg_value += raw;

    if (raw > max_value)
      max_value = raw;
    if (raw < min_value)
      min_value = raw;
  }

  avg_value /= N_CH; // Trung bình tất cả sensors

  // Kiểm tra contrast để đảm bảo có line
  uint32_t contrast = max_value - min_value;
  if (contrast < LINE_DETECTION_THRESHOLD)
  {
    return LINE_NONE; // Không đủ contrast để phát hiện line
  }

  // Đếm số sensors phát hiện line (giá trị thấp hơn trung bình đáng kể)
  for (int i = 0; i < N_CH; i++)
  {
    if (snap[i] < (avg_value - 200)) // Sensor thấp hơn trung bình 200 ADC units
    {
      active_sensors++;
    }
  }

  // Phải có ít nhất 1 sensor active mới coi là có line
  if (active_sensors == 0)
  {
    return LINE_NONE;
  }

  // Tính trung bình của các sensors active (có line)
  uint32_t line_avg = 0;
  uint8_t line_sensor_count = 0;
  for (int i = 0; i < N_CH; i++)
  {
    if (snap[i] < (avg_value - 200))
    {
      line_avg += snap[i];
      line_sensor_count++;
    }
  }

  if (line_sensor_count > 0)
  {
    line_avg /= line_sensor_count;
  }
  else
  {
    return LINE_NONE;
  }

  // Phân loại màu dựa trên giá trị ADC trung bình của line
  if (line_avg < COLOR_BLACK_THRESHOLD)
  {
    return LINE_BLACK; // Line đen: phản xạ ít ánh sáng
  }
  else if (line_avg < COLOR_RED_THRESHOLD)
  {
    return LINE_RED; // Line đỏ: phản xạ trung bình
  }
  else if (line_avg < COLOR_WHITE_THRESHOLD)
  {
    return LINE_WHITE; // Line trắng: phản xạ nhiều ánh sáng
  }

  return LINE_NONE;
}

/* Kiểm tra có line hợp lệ (đen hoặc đỏ) không */
uint8_t LineSensors_HasValidLine(const uint16_t *snap)
{
  uint8_t color = LineSensors_DetectLineColor(snap);
  return (color == LINE_BLACK || color == LINE_RED);
}

/* Compute error nâng cao với color detection */
int LineSensors_ComputeErrorAdvanced(const uint16_t *snap)
{
  // Kiểm tra có line hợp lệ không
  if (!LineSensors_HasValidLine(snap))
  {
    lostLineCounter++;
    if (lostLineCounter < 100)
    {
      return (lastError >= 0) ? +1200 : -1200; // Giảm tốc độ quay khi mất line
    }
    else if (lostLineCounter < 300)
    {
      return (lastError >= 0) ? +1500 : -1500; // Tăng dần tốc độ quay
    }
    else
    {
      return 0; // Dừng lại sau 300ms không tìm thấy line hợp lệ
    }
  }

  // Reset counter khi tìm thấy line hợp lệ
  lostLineCounter = 0;

  // Sử dụng algorithm cũ với weighted sum nhưng chỉ với line hợp lệ
  int32_t wsum = 0, vsum = 0;
  uint8_t line_color = LineSensors_DetectLineColor(snap);

  for (int i = 0; i < N_CH; i++)
  {
    uint16_t r = snap[i];
    uint16_t mn = s_min[i], mx = s_max[i];
    if (mx <= mn)
      mx = mn + 1;

    int v = (int)(((int32_t)(r - mn) * 1000) / (mx - mn));
    if (v < 0)
      v = 0;
    if (v > 1000)
      v = 1000;

    // Boost weight cho sensors có line color phù hợp
    if (line_color == LINE_BLACK && r < COLOR_BLACK_THRESHOLD)
    {
      v = (int)(v * 1.2f); // Boost 20% cho black line
    }
    else if (line_color == LINE_RED && r >= COLOR_BLACK_THRESHOLD && r < COLOR_RED_THRESHOLD)
    {
      v = (int)(v * 1.1f); // Boost 10% cho red line
    }

    if (v > 1000)
      v = 1000; // Clamp lại sau boost

    wsum += v * (i * 1000);
    vsum += v;
  }

  int32_t center = ((N_CH - 1) * 1000) / 2;

  if (vsum > VSUM_THRESH)
  {
    int32_t pos = wsum / vsum;
    int32_t e = pos - center;
    lastError = e;
    return (int)e;
  }
  else if (vsum > VSUM_WEAK_THRESH)
  {
    // Line yếu - giảm hệ số phản hồi
    int32_t pos = wsum / vsum;
    int32_t e = pos - center;
    lastError = e;
    return (int)(e * 0.8f);
  }

  // Fallback to old behavior
  return LineSensors_ComputeError(snap);
}

/* Debug function để kiểm tra sensor values */
SensorDebug_t LineSensors_GetDebugInfo(const uint16_t *snap)
{
  SensorDebug_t debug = {0};

  // Copy raw values
  for (int i = 0; i < N_CH; i++)
  {
    debug.raw_values[i] = snap[i];
  }

  // Calculate stats
  uint32_t sum = 0;
  debug.min_val = 4095;
  debug.max_val = 0;

  for (int i = 0; i < N_CH; i++)
  {
    sum += snap[i];
    if (snap[i] < debug.min_val)
      debug.min_val = snap[i];
    if (snap[i] > debug.max_val)
      debug.max_val = snap[i];
  }

  debug.avg_all = sum / N_CH;
  debug.contrast = debug.max_val - debug.min_val;

  // Count active sensors
  debug.active_sensors = 0;
  for (int i = 0; i < N_CH; i++)
  {
    if (snap[i] < (debug.avg_all - 200))
    {
      debug.active_sensors++;
    }
  }

  // Get color and validity
  debug.detected_color = LineSensors_DetectLineColor(snap);
  debug.is_valid = LineSensors_HasValidLine(snap);

  return debug;
}

int LineSensors_ComputeError(const uint16_t *snap)
{
  int32_t wsum = 0, vsum = 0;
  for (int i = 0; i < N_CH; i++)
  {
    uint16_t r = snap[i];
    uint16_t mn = s_min[i], mx = s_max[i];
    if (mx <= mn)
      mx = mn + 1;
    int v = (int)(((int32_t)(r - mn) * 1000) / (mx - mn));
    if (v < 0)
      v = 0;
    if (v > 1000)
      v = 1000;
    wsum += v * (i * 1000);
    vsum += v;
  }
  int32_t center = ((N_CH - 1) * 1000) / 2;

  if (vsum > VSUM_THRESH)
  {
    // Ph?t hi?n line r? r?ng
    lostLineCounter = 0;
    int32_t pos = wsum / vsum;
    int32_t e = pos - center;
    lastError = e;
    return (int)e;
  }
  else if (vsum > VSUM_WEAK_THRESH)
  {
    // Ph?t hi?n line y?u - gi?m h? s? ph?n h?i
    lostLineCounter = 0;
    int32_t pos = wsum / vsum;
    int32_t e = pos - center;
    lastError = e;
    return (int)(e * 0.7f); // Gi?m ?? nh?y khi line y?u
  }
  else
  {
    // M?t line ho?n to?n
    lostLineCounter++;
    if (lostLineCounter < 100)
    {                                          // 100ms ?? th? l?i
      return (lastError >= 0) ? +1500 : -1500; // Gi?m t?c ?? quay
    }
    else if (lostLineCounter < 500)
    {                                          // 0.5s ti?p theo
      return (lastError >= 0) ? +1800 : -1800; // T?ng d?n t?c ?? quay
    }
    else
    {
      return 0; // D?ng l?i sau 0.5s kh?ng t?m th?y line
    }
  }
}
