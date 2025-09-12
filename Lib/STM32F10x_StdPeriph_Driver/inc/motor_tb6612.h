#ifndef MOTOR_TB6612_H
#define MOTOR_TB6612_H
#include "stm32f10x.h"
#include "board.h"
#include <stdint.h>

void Motor_InitGPIO_PWM(void);
void Motor_SetDirLeft(int forward);
void Motor_SetDirRight(int forward);
void Motor_WriteRaw(int left_duty, int right_duty);     // 0..ARR (không d?o chi?u)
void Motor_WriteSigned(int left_signed, int right_signed); // duty có d?u ? t? d?o chi?u

#endif
