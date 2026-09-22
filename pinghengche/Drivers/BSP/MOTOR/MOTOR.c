#include "stm32f1xx_hal.h"
#include "tim.h"
#include "MOTOR/MOTOR.h"

/**
  * 函    数：Motor初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // 开启 PA0 (左电机 PWM)
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); // 开启 PA1 (右电机 PWM)
}

/**
  * 函    数：直流电机设置速度
  * 参    数：PWM 要设置的速度，范围：-100~100
  * 返 回 值：无
  */
void Motor_SetLPWM(int8_t PWM)
{
	
		if (PWM >= 0)							//如果设置正转的速度值
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12,GPIO_PIN_SET);	//PB12置高电平
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,GPIO_PIN_RESET);	//PB13置低电平，设置方向为正转
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, PWM);				//PWM设置为速度值
		}
		else									//否则，即设置反转的速度值
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,GPIO_PIN_SET);	//PB13置低电平
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12,GPIO_PIN_RESET);	//PB12置高电平，设置方向为反转
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, -PWM);				//PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
		}
}

void Motor_SetRPWM(int8_t PWM)
{
	if (PWM >= 0)							//如果设置正转的速度值   **注意：使小车前进的方向规定为正，则要与另一个轮子极性相反
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);	//PB14置低电平
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15,GPIO_PIN_SET);	//PB15置高电平，设置方向为正转
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, PWM);				//PWM设置为速度值
	}
	else									//否则，即设置反转的速度值
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15,GPIO_PIN_RESET);	//PB15置低电平
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);	//PB14置高电平，设置方向为反转
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, -PWM);				//PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
	}
}
