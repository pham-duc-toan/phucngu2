#ifndef TIMERS_H
#define TIMERS_H
#include "board.h"

void Clocks_EnableAll(void);
void TIM4_PWM_Init(void);
void TIM3_Trigger_Init(void);
void TIM2_Control_IRQ_Init(void);

#endif
