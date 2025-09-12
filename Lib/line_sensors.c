#include "line_sensors.h"
#include "line_params.h" // File tham số để dễ tuning

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

/* Kiểm tra chất lượng hiệu chuẩn - trả về 1 nếu tốt, 0 nếu chưa tốt */
int LineSensors_CalibQuality(void)
{
  int good_channels = 0;
  for (int i = 0; i < N_CH; i++)
  {
    if ((s_max[i] - s_min[i]) > MIN_CALIB_DIFF_PARAM) // Sử dụng tham số từ file
      good_channels++;                                // Ngưỡng chênh lệch tối thiểu
  }
  return (good_channels >= (N_CH - 2)); // Chấp nhận nếu ít nhất 6/8 sensor tốt
}

/* Sử dụng tham số từ file line_params.h để dễ tuning */
#define VSUM_THRESH VSUM_THRESH_PARAM           // = 1800
#define VSUM_WEAK_THRESH VSUM_WEAK_THRESH_PARAM // = 1000

static uint32_t lostLineCounter = 0;

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

  // Count active sensors (dựa vào VSUM thay vì color detection)
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
    vsum += v;
  }

  debug.active_sensors = (vsum > VSUM_THRESH) ? 1 : 0;
  debug.computed_error = LineSensors_ComputeError(snap);
  debug.is_valid = (vsum > VSUM_WEAK_THRESH);

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
    // Phát hiện line rõ ràng
    lostLineCounter = 0;
    int32_t pos = wsum / vsum;
    int32_t e = pos - center;
    lastError = e;
    return (int)e;
  }
  else if (vsum > VSUM_WEAK_THRESH)
  {
    // Phát hiện line yếu - giảm hệ số phản hồi
    lostLineCounter = 0;
    int32_t pos = wsum / vsum;
    int32_t e = pos - center;
    lastError = e;
    return (int)(e * 0.7f); // Giảm độ nhạy khi line yếu
  }
  else
  {
    // *** THAY ĐỔI: NGHIÊM KHẮC HƠN - KHÔNG TÌM LINE KHI KHÔNG CÓ ***
    lostLineCounter++;

    // CHỈ cho phép tìm line trong thời gian rất ngắn
    if (lostLineCounter < LOST_LINE_TIME_MS_PARAM) // Sử dụng tham số từ file
    {
      return (lastError >= 0) ? +SEARCH_SPEED_PARAM : -SEARCH_SPEED_PARAM; // Sử dụng tham số từ file
    }
    else
    {
      return 0; // Dừng ngay sau thời gian quy định - NGHIÊM KHẮC
    }
  }
}
