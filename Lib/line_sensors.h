#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H
#include "board.h"

// Định nghĩa màu line
#define LINE_NONE 0
#define LINE_BLACK 1
#define LINE_RED 2
#define LINE_WHITE 3

// Threshold phát hiện line
#define LINE_DETECTION_THRESHOLD 300
#define COLOR_BLACK_THRESHOLD 800  // ADC < 800: line đen
#define COLOR_RED_THRESHOLD 2000   // 800 ≤ ADC < 2000: line đỏ
#define COLOR_WHITE_THRESHOLD 3000 // ADC ≥ 2000: line trắng/nền

void LineSensors_CalibInit(void);
void LineSensors_UpdateCalib(const uint16_t *snap);
int LineSensors_CalibQuality(void);                 // Kiểm tra chất lượng hiệu chuẩn
int LineSensors_ComputeError(const uint16_t *snap); // âm: lệch trái, dương: lệch phải

// Thêm các function mới cho phát hiện màu
uint8_t LineSensors_DetectLineColor(const uint16_t *snap);
uint8_t LineSensors_HasValidLine(const uint16_t *snap);
int LineSensors_ComputeErrorAdvanced(const uint16_t *snap); // Tối ưu với color detection

#endif
