#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"

#define KEY_NONE 0
#define KEY1     1
#define KEY2     2
#define KEY3     3
#define KEY4     4

/* 读一次原始键值，非阻塞 */
uint8_t Key_GetNum(void);
/* 带消抖与按键锁定的扫描，非阻塞；返回本次确认按下的键值，无按键返回 KEY_NONE */
uint8_t Key_Scan(void);

#endif
