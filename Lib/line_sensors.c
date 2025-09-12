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

/* 15% của 8000 - ngưỡng để phát hiện line - theo code gốc đạt giải */
#define VSUM_THRESH 1200
/* Ngưỡng để phát hiện line yếu */
#define VSUM_WEAK_THRESH 800

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
