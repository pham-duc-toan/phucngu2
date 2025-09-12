#ifndef ADC_DMA_H
#define ADC_DMA_H
#include "stm32f10x.h"
#include "board.h"

void ADC_DMA_Init(void);     // ADC1 + DMA1 Ch1 + sequence 8 kênh
void ADC_DMA_Start(void);    // b?t d?u (sau khi c?u hình timer trigger)
void TIM3_Trigger_Init(void);// 2 kHz TRGO

#endif
