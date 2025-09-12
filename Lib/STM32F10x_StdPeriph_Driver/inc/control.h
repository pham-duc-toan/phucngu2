#ifndef CONTROL_H
#define CONTROL_H
#include <stdint.h>
#include "pid.h"

void Control_Init(void);
void Control_Loop_1kHz(void);   // g?i trong TIM2_IRQHandler

#endif
