#include "LED/LED.h"
#include "stm32f1xx_hal.h"

void onLED(GPIO_TypeDef* GPIOx,uint16_t PINx)
{
	HAL_GPIO_WritePin(GPIOx,PINx,GPIO_PIN_RESET);
}

void offLED(GPIO_TypeDef* GPIOx,uint16_t PINx)
{
	HAL_GPIO_WritePin(GPIOx,PINx,GPIO_PIN_SET);
}

