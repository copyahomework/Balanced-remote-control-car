#include "ENCODER/ENCODER.h"
#include "tim.h" // 包含CubeMX生成的tim.h，里面有 htim3
#include "stm32f1xx_hal.h"

void Encoder_Init(void)
{
	// 启动编码器接口（相当于标准库的 TIM_Cmd(TIM3, ENABLE)）
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
	
	// 清零计数器，确保初始状态为0
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	__HAL_TIM_SET_COUNTER(&htim4, 0);
}

int16_t Encoder_Get(uint8_t n)
{
	if(n == 1)
	{
    int16_t Temp;
    // 读取计数器值（强转为 int16_t 可以自动处理正反转的 0~65535 补码问题）
    Temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    // 清零计数器
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    return Temp;
	}
	
	else if(n == 2)
	{
		int16_t Temp;
    // 读取计数器值（强转为 int16_t 可以自动处理正反转的 0~65535 补码问题）
    Temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
    // 清零计数器
    __HAL_TIM_SET_COUNTER(&htim4, 0);
		return -Temp;
	}
	
	else return 0;
}
