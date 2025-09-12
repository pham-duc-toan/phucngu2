#include "pid.h"

void PID_Init(PID_t* p, float Kp, float Ki, float Kd, float Ts, float d_alpha, float umin, float umax){
  p->Kp=Kp; p->Ki=Ki; p->Kd=Kd; p->Ts=Ts;
  p->I=0; p->D=0; p->last_e=0;
  p->d_alpha=d_alpha; p->u_min=umin; p->u_max=umax;
}

float PID_Update(PID_t* p, float e){
  float de = (e - p->last_e)/p->Ts;
  p->D = p->d_alpha*p->D + (1.0f-p->d_alpha)*de;

  float P = p->Kp * e;
  float I_next = p->I + p->Ki * e * p->Ts;
  float u = P + I_next + p->Kd * p->D;

  if (u > p->u_max){
    u = p->u_max;
    if (e < 0) p->I = I_next;
  } else if (u < p->u_min){
    u = p->u_min;
    if (e > 0) p->I = I_next;
  } else {
    p->I = I_next;
  }
  p->last_e = e;
  return u;
}
