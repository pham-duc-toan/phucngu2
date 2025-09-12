#ifndef MOTOR_TB6612_H
#define MOTOR_TB6612_H
#include "board.h"

void Motor_InitGPIO_PWM(void);  // c?u h�nh DIR/STBY
void Motor_SetForward(void);    // lu�n ti?n
void Motor_WritePWM(int left, int right); // duty 0..ARR
void Motor_SetCalibration(float left_factor, float right_factor); // H? s? hi?u chu?n
void Motor_WritePWM_Calibrated(int left, int right); // PWM có hi?u chu?n

#endif
