#ifndef BUTTON_H
#define BUTTON_H
#include "board.h"

void    Button_Init(void);
void    Button_Task_1ms(void);    // g?i trong TIM2 IRQ
uint8_t Button_RunEnabled(void);
void    Button_SetRunEnabled(uint8_t en);

#endif
