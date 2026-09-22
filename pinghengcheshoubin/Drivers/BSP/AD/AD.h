#ifndef __AD_H
#define __AD_H

#include "main.h"
#include <stdint.h>

/* ADC reference voltage used by AD_GetVoltage(). */
#ifndef AD_VREF
#define AD_VREF (3.3f)
#endif

/*
 * Read one ADC1 regular channel by polling.
 * Supported channels in this board configuration: ADC_CHANNEL_0 .. ADC_CHANNEL_3
 * (PA0 .. PA3).  The return value is the unscaled 12-bit ADC result: 0 .. 4095.
 */
uint16_t AD_GetValue(uint32_t ADC_Channel);
float AD_GetVoltage(uint16_t raw);

#endif 
