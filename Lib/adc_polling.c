#include "adc_polling.h"
#include "stm32f10x.h"

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================ */

static uint16_t adc_values[ADC_CHANNELS] = {0};

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

static void ADC_GPIO_Init(void)
{
  // Enable GPIOA clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  // Configure PA0-PA7 as analog inputs
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void ADC_Config_Init(void)
{
  // Enable ADC1 clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_InitTypeDef ADC_InitStructure;

  // ADC1 configuration
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  // Enable ADC1
  ADC_Cmd(ADC1, ENABLE);

  // ADC1 reset calibration register
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1))
    ;

  // ADC1 calibration start
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1))
    ;
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

void ADC_Polling_Init(void)
{
  ADC_GPIO_Init();
  ADC_Config_Init();

  // Read initial values
  ADC_ReadAllChannels(adc_values);
}

uint16_t ADC_ReadChannel(uint8_t channel)
{
  if (channel >= ADC_CHANNELS)
    return 0;

  // Select channel
  ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_55Cycles5);

  // Start conversion
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  // Wait for conversion to complete
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
    ;

  // Read conversion result
  uint16_t result = ADC_GetConversionValue(ADC1);

  // Clear EOC flag
  ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

  return result;
}

void ADC_ReadAllChannels(uint16_t *buffer)
{
  for (int i = 0; i < ADC_CHANNELS; i++)
  {
    buffer[i] = ADC_ReadChannel(i);
    adc_values[i] = buffer[i];
  }
}

uint16_t ADC_GetChannelValue(uint8_t channel)
{
  if (channel >= ADC_CHANNELS)
    return 0;
  return adc_values[channel];
}

void ADC_PrintChannelValues(void)
{
  // For debugging - can be used with printf or debugger
  // adc_values[0..7] contains current sensor readings
}