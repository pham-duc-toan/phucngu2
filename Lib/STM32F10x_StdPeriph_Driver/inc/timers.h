#ifndef TIMERS_H
#define TIMERS_H

#include "stm32f10x.h"
#include <stdint.h>

void Clocks_EnableAll(void);
void TIM4_PWM_Init(void);
void TIM2_Control_IRQ_Init(void);

#endif
