#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H
#include "board.h"

// Định nghĩa màu line
#define LINE_NONE 0
#define LINE_BLACK 1
#define LINE_RED 2
#define LINE_WHITE 3

// Threshold phát hiện line - RELAX HƠN ĐỂ DEBUG
#define LINE_DETECTION_THRESHOLD 200 // Giảm để dễ detect line
#define COLOR_BLACK_THRESHOLD 1000   // ADC < 1000: line đen (relax hơn)
#define COLOR_RED_THRESHOLD 2000     // 1000 ≤ ADC < 2000: line đỏ
#define COLOR_WHITE_THRESHOLD 3000   // ADC ≥ 2000: line trắng/nền

// Debug function để kiểm tra sensors
typedef struct
{
  uint16_t raw_values[N_CH];
  uint32_t avg_all;
  uint32_t min_val;
  uint32_t max_val;
  uint32_t contrast;
  uint8_t active_sensors;
  uint8_t detected_color;
  uint8_t is_valid;
} SensorDebug_t;

void LineSensors_CalibInit(void);
void LineSensors_UpdateCalib(const uint16_t *snap);
int LineSensors_CalibQuality(void);                 // Kiểm tra chất lượng hiệu chuẩn
int LineSensors_ComputeError(const uint16_t *snap); // âm: lệch trái, dương: lệch phải

// Thêm các function mới cho phát hiện màu
uint8_t LineSensors_DetectLineColor(const uint16_t *snap);
uint8_t LineSensors_HasValidLine(const uint16_t *snap);
int LineSensors_ComputeErrorAdvanced(const uint16_t *snap); // Tối ưu với color detection

// Debug function
SensorDebug_t LineSensors_GetDebugInfo(const uint16_t *snap);

#endif
