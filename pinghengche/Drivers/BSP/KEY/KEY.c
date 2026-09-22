#include "stm32f1xx_hal.h"
#include "KEY/KEY.h"

uint8_t Key_GetNum(void)
{
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == 0) return KEY1;
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0) return KEY2;
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == 0) return KEY3;
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 0) return KEY4;
    return KEY_NONE;
}

uint8_t Key_Scan(void)
{
	//按键消抖
	static uint8_t key_buf[3] = {KEY_NONE, KEY_NONE, KEY_NONE};
	static uint8_t key_locked = 0;

	key_buf[0] = key_buf[1];
	key_buf[1] = key_buf[2];
	key_buf[2] = Key_GetNum();

	if ((key_buf[0] == key_buf[1]) && (key_buf[1] == key_buf[2]))
	{
			uint8_t key = key_buf[2];
			if (key == KEY_NONE)
			{
					key_locked = 0;         /* 稳定松开，解锁 */
			}
			else if (!key_locked)
			{
					key_locked = 1;         /* 单次触发，按下期间只返回一次 */
					return key;
			}
	}
	return KEY_NONE;
}
