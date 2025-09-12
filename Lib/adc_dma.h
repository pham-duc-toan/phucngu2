#ifndef ADC_DMA_H
#define ADC_DMA_H
#include "board.h"

extern volatile uint16_t adcBuf[N_CH];

void Sensors_GPIO_InitAnalog(void);   // PA0..PA7 analog
void ADC_DMA_Init(void);              // ADC1 + DMA circular (scan 8 kênh, trigger TIM3)
void ADC_DMA_Start(void);             // Power-up + calib

#endif
