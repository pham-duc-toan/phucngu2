#ifndef PID_H
#define PID_H
#include <stdint.h>

typedef struct {
  float Kp, Ki, Kd, Ts;
  float I, D, last_e;
  float d_alpha;            // l?c d?o hàm
  float u_min, u_max;       // k?p d?u ra (turn)
} PID_t;

void  PID_Init(PID_t* p, float Kp, float Ki, float Kd, float Ts, float d_alpha, float umin, float umax);
float PID_Update(PID_t* p, float e);

#endif
