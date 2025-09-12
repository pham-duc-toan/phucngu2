#ifndef CONTROL_H
#define CONTROL_H
#include "board.h"

void Control_Init(void);
void Control_Loop_1kHz(void);
void Control_SetMotorTestMode(uint8_t enable);  // B?t/t?t ch? ?? test motor
void Control_SetHighSpeedMode(uint8_t enable);  // B?t/t?t ch? ?? t?c ?? cao

#endif
