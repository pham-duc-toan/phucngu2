#include "system_state.h"
#include "button.h"
#include "line_sensors.h"
#include "line_params.h"
#include "motor_tb6612.h"

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================ */

static SystemState_t current_state = STATE_INIT;
static SystemState_t previous_state = STATE_INIT;

/* SysTick timer variables */
static volatile uint32_t systick_counter = 0;

/* Button timing variables */
static uint32_t button_press_start = 0;
static uint8_t button_was_pressed = 0;
static ButtonEvent_t last_button_event = BUTTON_NONE;

/* State timing */
static uint32_t state_enter_time = 0;
static uint32_t calibration_start_time = 0;

/* Speed reduction factor */
static const float SPEED_FACTOR = 0.6f; // 60% tốc độ

/* ============================================================================
 * SYSTICK TIMER FUNCTIONS
 * ============================================================================ */

void SystemState_SysTickHandler(void)
{
  systick_counter++;
}

uint32_t SystemState_GetTick(void)
{
  return systick_counter;
}

void SystemState_Delay(uint32_t ms)
{
  uint32_t start = SystemState_GetTick();
  while ((SystemState_GetTick() - start) < ms)
  {
    // Wait
  }
}

/* ============================================================================
 * BUTTON HANDLING FUNCTIONS
 * ============================================================================ */

void SystemState_ProcessButton(void)
{
  uint8_t button_current = Button_GetState();
  uint32_t current_time = SystemState_GetTick();

  // Phát hiện nhấn nút
  if (button_current && !button_was_pressed)
  {
    // Bắt đầu nhấn
    button_press_start = current_time;
    button_was_pressed = 1;
    last_button_event = BUTTON_NONE;
  }

  // Phát hiện thả nút
  if (!button_current && button_was_pressed)
  {
    uint32_t press_duration = current_time - button_press_start;
    button_was_pressed = 0;

    // Phân loại loại nhấn
    if (press_duration >= 3000) // >= 3s
    {
      last_button_event = BUTTON_LONG_3S;
    }
    else if (press_duration >= 1000 && press_duration < 2500) // 1-2.5s
    {
      last_button_event = BUTTON_LONG_STOP;
    }
    else if (press_duration < 1000) // < 1s
    {
      last_button_event = BUTTON_SHORT;
    }
  }
}

ButtonEvent_t SystemState_GetButtonEvent(void)
{
  ButtonEvent_t event = last_button_event;
  last_button_event = BUTTON_NONE; // Clear event after reading
  return event;
}

/* ============================================================================
 * STATE MACHINE FUNCTIONS
 * ============================================================================ */

void SystemState_Init(void)
{
  current_state = STATE_INIT;
  state_enter_time = SystemState_GetTick();
  systick_counter = 0;

  // Khởi tạo motor ở trạng thái dừng
  Motor_SetSpeed(MOTOR_LEFT, 0);
  Motor_SetSpeed(MOTOR_RIGHT, 0);

  // Khởi tạo sensors
  LineSensors_CalibInit();
}

void SystemState_Update(void)
{
  uint32_t current_time = SystemState_GetTick();
  uint32_t state_time = current_time - state_enter_time;

  // Xử lý button events
  SystemState_ProcessButton();
  ButtonEvent_t button_event = SystemState_GetButtonEvent();

  SystemState_t next_state = current_state;

  switch (current_state)
  {
  case STATE_INIT:
    // Chuyển sang STANDBY sau khi khởi tạo xong
    if (state_time > 500) // 500ms để ổn định
    {
      next_state = STATE_STANDBY;
    }
    break;

  case STATE_STANDBY:
    // Đợi nhấn giữ 3s để bắt đầu
    if (button_event == BUTTON_LONG_3S)
    {
      next_state = STATE_CALIBRATION;
      calibration_start_time = current_time;
    }
    break;

  case STATE_CALIBRATION:
    // Quét và học vạch line trong 3-8s
    // Motor KHÔNG quay, chỉ quét sensor
    Motor_SetSpeed(MOTOR_LEFT, 0);
    Motor_SetSpeed(MOTOR_RIGHT, 0);

    // Cập nhật calibration
    // (sensor data sẽ được xử lý trong main loop)

    // Chuyển sang RUNNING nếu:
    // - Nhấn nhẹ (cho phép chuyển sớm)
    // - Hoặc đã đủ 8s
    if (button_event == BUTTON_SHORT || state_time >= 8000)
    {
      next_state = STATE_RUNNING;
    }

    // Dừng nếu nhấn giữ
    if (button_event == BUTTON_LONG_STOP)
    {
      next_state = STATE_STOPPED;
    }
    break;

  case STATE_RUNNING:
    // Robot chạy theo line với tốc độ 60%
    // (Motor control sẽ được xử lý trong main loop)

    // Dừng nếu nhấn giữ 1-2s
    if (button_event == BUTTON_LONG_STOP)
    {
      next_state = STATE_STOPPED;
    }
    break;

  case STATE_STOPPED:
    // Dừng motor
    Motor_SetSpeed(MOTOR_LEFT, 0);
    Motor_SetSpeed(MOTOR_RIGHT, 0);

    // Quay lại STANDBY nếu nhấn giữ 3s
    if (button_event == BUTTON_LONG_3S)
    {
      next_state = STATE_STANDBY;
    }
    break;
  }

  // Cập nhật state nếu có thay đổi
  if (next_state != current_state)
  {
    previous_state = current_state;
    current_state = next_state;
    state_enter_time = current_time;
  }
}

/* ============================================================================
 * PUBLIC GETTER FUNCTIONS
 * ============================================================================ */

SystemState_t SystemState_GetCurrent(void)
{
  return current_state;
}

uint8_t SystemState_IsRunning(void)
{
  return (current_state == STATE_RUNNING);
}

uint8_t SystemState_IsCalibrating(void)
{
  return (current_state == STATE_CALIBRATION);
}

void SystemState_ForceStop(void)
{
  current_state = STATE_STOPPED;
  state_enter_time = SystemState_GetTick();
  Motor_SetSpeed(MOTOR_LEFT, 0);
  Motor_SetSpeed(MOTOR_RIGHT, 0);
}

float SystemState_GetSpeedFactor(void)
{
  return SPEED_FACTOR;
}

/* ============================================================================
 * DEBUG FUNCTIONS
 * ============================================================================ */

const char *SystemState_GetStateName(void)
{
  switch (current_state)
  {
  case STATE_INIT:
    return "INIT";
  case STATE_STANDBY:
    return "STANDBY";
  case STATE_CALIBRATION:
    return "CALIBRATION";
  case STATE_RUNNING:
    return "RUNNING";
  case STATE_STOPPED:
    return "STOPPED";
  default:
    return "UNKNOWN";
  }
}

uint32_t SystemState_GetStateTime(void)
{
  return SystemState_GetTick() - state_enter_time;
}