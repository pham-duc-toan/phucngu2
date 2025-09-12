#ifndef CONTROL_H
#define CONTROL_H
#include "board.h"

void Control_Init(void);
void Control_Loop_1kHz(void);
void Control_SetMotorTestMode(uint8_t enable); // Bật/tắt chế độ test motor
void Control_SetHighSpeedMode(uint8_t enable); // Bật/tắt chế độ tốc độ cao

// Thêm các function mới
uint8_t Control_GetLineDetected(void);            // Kiểm tra có line hợp lệ không
uint8_t Control_GetLineColor(void);               // Lấy màu line hiện tại
void Control_SetRequiredLineColor(uint8_t color); // Đặt màu line yêu cầu (BLACK/RED)

#endif
