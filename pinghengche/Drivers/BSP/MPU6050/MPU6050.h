#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f1xx_hal.h"

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
void MPU6050_ReadReg(uint8_t RegAddress,uint8_t *Data,uint16_t count);
void MPU6050_Init(void);
void MPU6050_GetData(volatile int16_t *AccX,volatile int16_t *AccY,volatile int16_t *AccZ, 
						volatile int16_t *GyroX,volatile int16_t *GyroY,volatile int16_t *GyroZ);

#endif
