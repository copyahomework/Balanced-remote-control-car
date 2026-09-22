#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

#define PC13_PIN 	GPIO_PIN_13
#define PC13_GPIO GPIOC

void onLED(GPIO_TypeDef* GPIOx,uint16_t PINx);

void offLED(GPIO_TypeDef* GPIOx,uint16_t PINx);

#endif
