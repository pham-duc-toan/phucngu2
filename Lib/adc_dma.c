#include "adc_dma.h"

volatile uint16_t adcBuf[N_CH];

void Sensors_GPIO_InitAnalog(void){
  GPIOA->CRL = 0x00000000; // PA0..PA7 analog
}

void ADC_DMA_Init(void){
  /* Clock base */
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
  RCC->AHBENR  |= RCC_AHBENR_DMA1EN;
  /* ADC clk = PCLK2/6 = 12 MHz */
  RCC->CFGR &= ~RCC_CFGR_ADCPRE;
  RCC->CFGR |=  RCC_CFGR_ADCPRE_DIV6;

  /* DMA1 ch1 */
  DMA1_Channel1->CCR  = 0;
  DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
  DMA1_Channel1->CMAR = (uint32_t)adcBuf;
  DMA1_Channel1->CNDTR = N_CH;
  DMA1_Channel1->CCR  |= DMA_CCR1_MINC | DMA_CCR1_CIRC;
  DMA1_Channel1->CCR  |= DMA_CCR1_PSIZE_0 | DMA_CCR1_MSIZE_0; // 16-bit
  DMA1_Channel1->CCR  |= DMA_CCR1_EN;

  /* ADC1: scan + external trigger (TIM3_TRGO) + DMA */
  ADC1->CR1 = ADC_CR1_SCAN;

  ADC1->SMPR2 =
    (7<<(3*0))|(7<<(3*1))|(7<<(3*2))|(7<<(3*3))|
    (7<<(3*4))|(7<<(3*5))|(7<<(3*6))|(7<<(3*7));

  ADC1->SQR1 = (7<<20); // 8 conv
  ADC1->SQR3 = (0<<0)|(1<<5)|(2<<10)|(3<<15)|(4<<20)|(5<<25);
  ADC1->SQR2 = (6<<0)|(7<<5);

  ADC1->CR2  = ADC_CR2_EXTTRIG | ADC_CR2_DMA;
  ADC1->CR2 &= ~ADC_CR2_EXTSEL;
  ADC1->CR2 |=  (0x4 << 17); // TIM3_TRGO
}

void ADC_DMA_Start(void){
  ADC1->CR2 |= ADC_CR2_ADON;
  for(volatile int i=0;i<10000;i++);
  ADC1->CR2 |= ADC_CR2_RSTCAL; while(ADC1->CR2 & ADC_CR2_RSTCAL);
  ADC1->CR2 |= ADC_CR2_CAL;    while(ADC1->CR2 & ADC_CR2_CAL);
}
