#ifndef __PID_H
#define __PID_H

typedef struct
{
    float kp;
    float ki;
    float kd;

    float target;
    float feedback;

    float error;
    float last_error;

    float integral;

    float output;

    float max_output;
    float max_integral;

}PID_t;

void PID_Init(PID_t *pid,
              float kp,
              float ki,
              float kd,
              float max_out,
              float max_i);

float PID_Calculate(PID_t *pid);

#endif
