#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdint.h>

/* ============================================================================
 * HỆ THỐNG ĐIỀU KHIỂN TRẠNG THÁI ROBOT LINE FOLLOWER
 * ============================================================================ */

typedef enum
{
  STATE_INIT = 0, // Khởi tạo hệ thống
  STATE_STANDBY,  // Chờ nhấn nút để bắt đầu
  STATE_SCAN,     // Quét line (motor dừng)
  STATE_RUNNING,  // Chạy theo line
  STATE_STOPPED   // Dừng hệ thống
} SystemState_t;

typedef enum
{
  BUTTON_NONE = 0, // Không nhấn
  BUTTON_SINGLE,   // Nhấn 1 lần (vào SCAN mode)
  BUTTON_DOUBLE,   // Nhấn 2 lần (vào RUN mode)
  BUTTON_LONG_HOLD // Nhấn giữ (STOP)
} ButtonEvent_t;

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

void SystemState_Init(void);
void SystemState_Update(void);
SystemState_t SystemState_GetCurrent(void);
uint8_t SystemState_IsRunning(void);
uint8_t SystemState_IsScanning(void);
void SystemState_ForceStop(void);
float SystemState_GetSpeedFactor(void);
uint16_t SystemState_GetBaseSpeed(void);

/* SysTick timer functions */
void SystemState_SysTickHandler(void);
uint32_t SystemState_GetTick(void);
void SystemState_Delay(uint32_t ms);

/* Button handling functions */
ButtonEvent_t SystemState_GetButtonEvent(void);
void SystemState_ProcessButton(void);

/* Debug functions */
const char *SystemState_GetStateName(void);
uint32_t SystemState_GetStateTime(void);

#endif /* SYSTEM_STATE_H */