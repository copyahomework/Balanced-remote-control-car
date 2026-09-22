#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f1xx_hal.h"

void Motor_Init(void);
void Motor_SetLPWM(int8_t PWM);
void Motor_SetRPWM(int8_t PWM);

#endif
