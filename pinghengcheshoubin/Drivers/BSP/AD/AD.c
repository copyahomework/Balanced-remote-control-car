#include "main.h"
#include "adc.h"
#include "ad.h"

/*
 * ADC1 is configured by CubeMX for software-triggered, single conversions.
 * Before each conversion, select rank 1 dynamically so PA0 .. PA3 can be
 * sampled one at a time, just as in the original Standard Peripheral Library
 * implementation.
 */
uint16_t AD_GetValue(uint32_t ADC_Channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t value = 0U;

    if (ADC_Channel > ADC_CHANNEL_3)
    {
        return 0U;
    }

    sConfig.Channel = ADC_Channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return 0U;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return 0U;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 5U) == HAL_OK)
    {
        value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    (void)HAL_ADC_Stop(&hadc1);
    return value;
}

float AD_GetVoltage(uint16_t raw)
{
    return ((float)raw * AD_VREF) / 4095.0f;
}
