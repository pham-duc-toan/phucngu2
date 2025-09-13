#include "system_state.h"
#include "button.h"
#include "line_sensors.h"
#include "line_params.h"
#include "motor_tb6612.h"
#include "board.h"

/* Motor constants */
#define MOTOR_LEFT 0
#define MOTOR_RIGHT 1

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
static uint32_t last_button_release_time = 0;
static uint8_t button_click_count = 0;
static uint32_t double_click_timeout = 500; // 500ms cho double click

/* State timing */
static uint32_t state_enter_time = 0;

/* Speed factor - tăng lên 3000 */
static const float SPEED_FACTOR = 1.0f;  // 100% tốc độ
static const uint16_t BASE_SPEED = 3000; // Tốc độ cơ bản 3000

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
  uint8_t button_current = BTN_IS_PRESSED();
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
    if (press_duration >= 1000) // >= 1s = long hold (STOP)
    {
      last_button_event = BUTTON_LONG_HOLD;
      button_click_count = 0; // Reset click count
    }
    else if (press_duration < 1000) // < 1s = short press
    {
      // Xử lý single/double click
      if (button_click_count == 0)
      {
        // First click
        button_click_count = 1;
        last_button_release_time = current_time;
      }
      else if (button_click_count == 1)
      {
        // Second click
        if ((current_time - last_button_release_time) < double_click_timeout)
        {
          last_button_event = BUTTON_DOUBLE;
          button_click_count = 0;
        }
        else
        {
          // Too slow, treat as new first click
          button_click_count = 1;
          last_button_release_time = current_time;
        }
      }
    }
  }

  // Timeout cho single click
  if (button_click_count == 1 &&
      (current_time - last_button_release_time) >= double_click_timeout)
  {
    last_button_event = BUTTON_SINGLE;
    button_click_count = 0;
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
  Motor_WritePWM(0, 0);

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
    // Nhấn 1 lần: vào SCAN mode
    if (button_event == BUTTON_SINGLE)
    {
      next_state = STATE_SCAN;
    }
    // Nhấn 2 lần: vào RUN mode trực tiếp
    else if (button_event == BUTTON_DOUBLE)
    {
      next_state = STATE_RUNNING;
    }
    break;

  case STATE_SCAN:
    // Quét line, motor KHÔNG chạy
    Motor_WritePWM(0, 0);

    // Nhấn 1 lần: tiếp tục SCAN
    // Nhấn 2 lần: chuyển sang RUN
    if (button_event == BUTTON_DOUBLE)
    {
      next_state = STATE_RUNNING;
    }
    // Nhấn giữ: dừng
    else if (button_event == BUTTON_LONG_HOLD)
    {
      next_state = STATE_STOPPED;
    }
    break;

  case STATE_RUNNING:
    // Robot chạy theo line
    // (Motor control sẽ được xử lý trong main loop)

    // Nhấn giữ: dừng
    if (button_event == BUTTON_LONG_HOLD)
    {
      next_state = STATE_STOPPED;
    }
    // Nhấn 1 lần: về SCAN mode
    else if (button_event == BUTTON_SINGLE)
    {
      next_state = STATE_SCAN;
    }
    break;

  case STATE_STOPPED:
    // Dừng motor
    Motor_WritePWM(0, 0);

    // Nhấn 1 lần: vào SCAN
    if (button_event == BUTTON_SINGLE)
    {
      next_state = STATE_SCAN;
    }
    // Nhấn 2 lần: vào RUN
    else if (button_event == BUTTON_DOUBLE)
    {
      next_state = STATE_RUNNING;
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

uint8_t SystemState_IsScanning(void)
{
  return (current_state == STATE_SCAN);
}

void SystemState_ForceStop(void)
{
  current_state = STATE_STOPPED;
  state_enter_time = SystemState_GetTick();
  Motor_WritePWM(0, 0);
}

float SystemState_GetSpeedFactor(void)
{
  return SPEED_FACTOR;
}

uint16_t SystemState_GetBaseSpeed(void)
{
  return BASE_SPEED;
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
  case STATE_SCAN:
    return "SCAN";
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