/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED/OLED.h"
#include "KEY/KEY.h"
#include "AD/AD.h"
#include "NRF24L01/NRF24L01.h"
#include "cmsis_os.h"
#include "adc.h"      
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
#define DeathValue 100
#define Flexibility 100

volatile uint8_t Keynum;						//按键键值
volatile uint16_t adc_val;					//adc数值保存
volatile uint16_t AD_LH,AD_LV,AD_RH,AD_RV;//用于保存12位ADC原始数据
volatile int16_t LH,LV,RH,RV;//用于保存映射后的数据


osMutexId_t NRFMutexHandle;
const osMutexAttr_t NRFMutex_attributes = {
    .name = "NRFMutex"
};
/* USER CODE END Variables */
/* Definitions for OLED_Task */
osThreadId_t OLED_TaskHandle;
const osThreadAttr_t OLED_Task_attributes = {
  .name = "OLED_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KEY_Task */
osThreadId_t KEY_TaskHandle;
const osThreadAttr_t KEY_Task_attributes = {
  .name = "KEY_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for AD_Task */
osThreadId_t AD_TaskHandle;
const osThreadAttr_t AD_Task_attributes = {
  .name = "AD_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for NRF_Task */
osThreadId_t NRF_TaskHandle;
const osThreadAttr_t NRF_Task_attributes = {
  .name = "NRF_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
int16_t DataProcess(uint16_t ADValue);
/* USER CODE END FunctionPrototypes */

void OLEDTask(void *argument);
void KEYTask(void *argument);
void ADTask(void *argument);
void NRFTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	NRF24L01_Init();
	NRFMutexHandle = osMutexNew(&NRFMutex_attributes);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of OLED_Task */
  OLED_TaskHandle = osThreadNew(OLEDTask, NULL, &OLED_Task_attributes);

  /* creation of KEY_Task */
  KEY_TaskHandle = osThreadNew(KEYTask, NULL, &KEY_Task_attributes);

  /* creation of AD_Task */
  AD_TaskHandle = osThreadNew(ADTask, NULL, &AD_Task_attributes);

  /* creation of NRF_Task */
  NRF_TaskHandle = osThreadNew(NRFTask, NULL, &NRF_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_OLEDTask */
/**
  * @brief  Function implementing the OLED_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_OLEDTask */
void OLEDTask(void *argument)
{
  /* USER CODE BEGIN OLEDTask */
	uint32_t ticks = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    ticks += 10; 
    osDelayUntil(ticks); 

		OLED_Printf(0,0,OLED_6X8,"LH:%4d",LH);
		OLED_Printf(0,16,OLED_6X8,"LV:%4d",LV);
		OLED_Printf(0,32,OLED_6X8,"RH:%4d",RH);
		OLED_Printf(0,48,OLED_6X8,"RV:%4d",RV);
		
		OLED_Printf(60,0,OLED_6X8,"AD_LH:%4d",AD_LH);
		OLED_Printf(60,16,OLED_6X8,"AD_LV:%4d",AD_LV);
		OLED_Printf(60,32,OLED_6X8,"AD_RH:%4d",AD_RH);
		OLED_Printf(60,48,OLED_6X8,"AD_RV:%4d",AD_RV);
		
		OLED_Update();
  }
  /* USER CODE END OLEDTask */
}

/* USER CODE BEGIN Header_KEYTask */
/**
* @brief Function implementing the KEY_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KEYTask */
void KEYTask(void *argument)
{
  /* USER CODE BEGIN KEYTask */
  uint32_t ticks = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    ticks += 1; // 设定绝对周期为 1ms
    osDelayUntil(ticks); // 无论任务执行了多久，都会在固定的 1ms 节点唤醒
		
		switch(Key_Scan())
		{
			case 1: 																break;
			case 2:																	break;
			case 3: 																break;
			case 4: 																break;
			default:																break;
		}
		
  }
  /* USER CODE END KEYTask */
}

/* USER CODE BEGIN Header_ADTask */
/**
* @brief Function implementing the AD_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ADTask */
void ADTask(void *argument)
{
  /* USER CODE BEGIN ADTask */
  uint32_t ticks = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    ticks += 10; // 采样间隔 10ms
    osDelayUntil(ticks); 
		
    // 读取通道，等待超时 5ms
    AD_LH = AD_GetValue(ADC_CHANNEL_0);
		AD_LV = AD_GetValue(ADC_CHANNEL_1);
		AD_RH = AD_GetValue(ADC_CHANNEL_2);
		AD_RV = AD_GetValue(ADC_CHANNEL_3);    

    LH = DataProcess(AD_LH);
		LV = DataProcess(AD_LV);
		RH = DataProcess(AD_RH);
		RV = DataProcess(AD_RV);
		
  }
  /* USER CODE END ADTask */
}

/* USER CODE BEGIN Header_NRFTask */
/**
* @brief Function implementing the NRF_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_NRFTask */
void NRFTask(void *argument)
{
  /* USER CODE BEGIN NRFTask */
	uint32_t ticks = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    ticks += 100; // 采样间隔 100ms
    osDelayUntil(ticks); 
		
		uint8_t ret = NRF24L01_Receive();

		if (ret == 1)
		{
					
				
		}
//		else if (ret == 2 || ret == 3)
//		{
//				/*驱动已自动重新初始化，可按需重试或记录*/
//		}

		/*-----------发送部分-----------*/
		NRF24L01_TxPacket[0] = LH;
		NRF24L01_TxPacket[1] = LV;
		NRF24L01_TxPacket[2] = RH;
		NRF24L01_TxPacket[3] = RV;
		
		osMutexAcquire(NRFMutexHandle, osWaitForever);
			
				NRF24L01_Send();
			
		osMutexRelease(NRFMutexHandle);
  }
  /* USER CODE END NRFTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

//将原始值限定在200~-200
int16_t DataProcess(uint16_t ADValue)
{
    int32_t value = (int32_t)ADValue - 2048;

    if (value > DeathValue)
    {
        return (int16_t)((value - DeathValue) * (Flexibility + 1) / (2047 - DeathValue));
    }

    if (value < -DeathValue)
    {
        return (int16_t)((value + DeathValue) * (Flexibility + 1) / (2048 - DeathValue));
    }

    return 0;
}

/* USER CODE END Application */

