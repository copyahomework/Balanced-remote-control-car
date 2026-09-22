#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"

#define KEY_NONE 0
#define KEY1     1
#define KEY2     2
#define KEY3     3
#define KEY4     4
#define KEY5     5
#define KEY6     6
#define KEY7     7
#define KEY8     8
#define KEY9     9
#define KEY10    10
#define KEY11    11
#define KEY12    12

/* 带消抖与按键锁定的扫描，非阻塞；返回本次确认按下的键值，无按键返回 KEY_NONE */
uint8_t Key_Scan(void);

#endif
