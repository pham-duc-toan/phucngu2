#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H
#include "board.h"

// SIMPLE LINE DETECTION - theo code gốc đạt giải
// Không cần phân biệt màu, chỉ cần phát hiện có line hay không

// Debug function để kiểm tra sensors
typedef struct
{
  uint16_t raw_values[N_CH];
  uint32_t avg_all;
  uint32_t min_val;
  uint32_t max_val;
  uint32_t contrast;
  uint8_t active_sensors;
  int computed_error;
  uint8_t is_valid;
} SensorDebug_t;

void LineSensors_CalibInit(void);
void LineSensors_UpdateCalib(const uint16_t *snap);
int LineSensors_CalibQuality(void);                 // Kiểm tra chất lượng hiệu chuẩn
int LineSensors_ComputeError(const uint16_t *snap); // âm: lệch trái, dương: lệch phải

// Debug function
SensorDebug_t LineSensors_GetDebugInfo(const uint16_t *snap);

#endif
