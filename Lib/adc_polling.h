#ifndef ADC_POLLING_H
#define ADC_POLLING_H

#include <stdint.h>
#include "board.h"

/* ============================================================================
 * ADC POLLING MODULE - THAY THẾ DMA
 * ============================================================================
 * Đọc trực tiếp ADC channels PA0-PA7 cho line sensors
 * Không sử dụng DMA, polling trực tiếp trong CPU
 */

#define ADC_CHANNELS 8

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

void ADC_Polling_Init(void);
uint16_t ADC_ReadChannel(uint8_t channel);
void ADC_ReadAllChannels(uint16_t *buffer);
uint16_t ADC_GetChannelValue(uint8_t channel);

/* Debug functions */
void ADC_PrintChannelValues(void);

#endif /* ADC_POLLING_H */