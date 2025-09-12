#ifndef CONTROL_H
#define CONTROL_H
#include "board.h"
#include "line_sensors.h"

void Control_Init(void);
void Control_Loop_1kHz(void);
void Control_SetMotorTestMode(uint8_t enable); // Bật/tắt chế độ test motor
void Control_SetHighSpeedMode(uint8_t enable); // Bật/tắt chế độ tốc độ cao

// Simple line detection function
uint8_t Control_GetLineDetected(void); // Kiểm tra có line hợp lệ không

// Debug function
void Control_DebugInfo(void); // Debug function để kiểm tra trạng thái

#endif
