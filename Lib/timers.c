#include "timers.h"
#include "control.h"
#include "button.h"

void Clocks_EnableAll(void){
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_ADC1EN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
  RCC->AHBENR  |= RCC_AHBENR_DMA1EN;
}

void TIM4_PWM_Init(void){
  /* PB6, PB7 AF-PP 50MHz */
  GPIOB->CRL &= ~((0xFUL << (6U*4U)) | (0xFUL << (7U*4U)));
	GPIOB->CRL |=  ((0xBUL << (6U*4U)) | (0xBUL << (7U*4U)));

  TIM4->PSC = 0;
  TIM4->ARR = 4000 - 1;               // ~18 kHz
  TIM4->CCMR1 = 0;
  TIM4->CCMR1 |= (6<<4)|TIM_CCMR1_OC1PE;
  TIM4->CCMR1 |= (6<<12)|TIM_CCMR1_OC2PE;
  TIM4->CCER  = TIM_CCER_CC1E | TIM_CCER_CC2E;
  TIM4->CR1   = TIM_CR1_ARPE | TIM_CR1_CEN;
  TIM4->CCR1=0; TIM4->CCR2=0;
}

void TIM3_Trigger_Init(void){
  /* 72e6 / ((PSC+1)*(ARR+1)) = 2000 */
  TIM3->PSC = 7199; TIM3->ARR = 5 - 1;
  TIM3->CR2 &= ~TIM_CR2_MMS; TIM3->CR2 |= (0x2<<4); // TRGO = Update
  TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM2_Control_IRQ_Init(void){
  TIM2->PSC = 7199; TIM2->ARR = 10 - 1;       // 1 kHz
  TIM2->DIER |= TIM_DIER_UIE; TIM2->CR1 |= TIM_CR1_CEN;
  NVIC_SetPriority(TIM2_IRQn, 1); NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void){
  if (TIM2->SR & TIM_SR_UIF){
    TIM2->SR &= ~TIM_SR_UIF;
    Button_Task_1ms();
    Control_Loop_1kHz();
  }
}
