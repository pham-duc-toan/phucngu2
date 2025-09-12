#ifndef BOARD_H
#define BOARD_H

#include "stm32f10x.h"
#include <stdint.h>

/* ===== H? s?, t?n s? ===== */
#define N_CH            8
#define CONTROL_HZ      1000U
#define ADC_TRIG_HZ     2000U
#define PWM_FREQ        18000U

/* ===== TB6612 pin mapping ===== */
#define DIRA1_PORT      GPIOB
#define DIRA1_PIN       8
#define DIRA2_PORT      GPIOB
#define DIRA2_PIN       9
#define DIRB1_PORT      GPIOB
#define DIRB1_PIN       10
#define DIRB2_PORT      GPIOB
#define DIRB2_PIN       11
#define STBY_PORT       GPIOB
#define STBY_PIN        12

/* ===== Button (PB13 – pull-up n?i, nh?n = 0) ===== */
#define BTN_PORT        GPIOB
#define BTN_PIN         13
#define BTN_IS_PRESSED()   ((BTN_PORT->IDR & (1U<<BTN_PIN)) == 0)

/* ===== Helpers ===== */
static inline void BSRR_RESET(GPIO_TypeDef* port, uint32_t pin){ port->BSRR = (1U<<(pin+16)); }
static inline uint16_t clamp_u16(int v, int lo, int hi){ if(v<lo)v=lo; if(v>hi)v=hi; return (uint16_t)v; }

#endif
