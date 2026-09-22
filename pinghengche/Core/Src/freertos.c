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
#include "LED/LED.h"
#include "KEY/KEY.h"
#include "MPU6050/MPU6050.h"
#include "MOTOR/MOTOR.h"
#include "ENCODER/ENCODER.h"
#include "SERIAL/SERIAL.h"
#include <math.h>
#include "PID.h"
#include <string.h>
#include <stdlib.h>
#include "NRF24L01/NRF24L01.h"
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
volatile int16_t AX,AY,AZ,GX,GY,GZ;//MPU6050结果数值
volatile float AngleAcc;//加速度计角度
volatile float AngleGyro;//陀螺仪角度
volatile int16_t PWML,PWMR;//左右PWM输入值
volatile int16_t AvePWM,DifPWM;//平均PWM和差分PWM
volatile float SpeedL,SpeedR;//左右电机速度值
volatile float AveSpeed,DifSpeed;//平均Speed和差分Speed
volatile float Angle;//倾角
volatile uint16_t RunFlag = 0;//平衡车功能运行标志位

osMutexId_t NRFMutexHandle;
const osMutexAttr_t NRFMutex_attributes = {
    .name = "NRFMutex"
};

volatile PID_t AnglePID = {//角度环PID参数
	.Kp = 5,
	.Ki = 0.25,
	.Kd = 5,
	
	.OutMax = 100,
	.OutMin = -100,
	
	.Outoffset = 3,
	
	.ErrorIntMax = 200,
	.ErrorIntMin = -200,
};

volatile PID_t SpeedPID = {//速度环PID参数
	.Kp = 3,
	.Ki = 0.05,
	.Kd = 0,
	
	.OutMax = 20,
	.OutMin = -20,
	
	.ErrorIntMax = 50,
	.ErrorIntMin = -50,
};

volatile PID_t TurnPID = {//转向环PID参数
	.Kp = 4,
	.Ki = 3.0,
	.Kd = 0,
	
	.OutMax = 50,
	.OutMin = -50,
	
	.ErrorIntMax = 10,
	.ErrorIntMin = -10,
};
/* USER CODE END Variables */
/* Definitions for OLED_Task */
osThreadId_t OLED_TaskHandle;
const osThreadAttr_t OLED_Task_attributes = {
  .name = "OLED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KEY_Task */
osThreadId_t KEY_TaskHandle;
const osThreadAttr_t KEY_Task_attributes = {
  .name = "KEY_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for AnglePID_Task */
osThreadId_t AnglePID_TaskHandle;
const osThreadAttr_t AnglePID_Task_attributes = {
  .name = "AnglePID_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SpeedPID_Task */
osThreadId_t SpeedPID_TaskHandle;
const osThreadAttr_t SpeedPID_Task_attributes = {
  .name = "SpeedPID_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SERIAL_Task */
osThreadId_t SERIAL_TaskHandle;
const osThreadAttr_t SERIAL_Task_attributes = {
  .name = "SERIAL_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Turn_Task */
osThreadId_t Turn_TaskHandle;
const osThreadAttr_t Turn_Task_attributes = {
  .name = "Turn_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for NRF_Task */
osThreadId_t NRF_TaskHandle;
const osThreadAttr_t NRF_Task_attributes = {
  .name = "NRF_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void OLEDTask(void *argument);
void KEYTask(void *argument);
void AnglePIDTask(void *argument);
void SpeedPIDTask(void *argument);
void SERIALTask(void *argument);
void TurnTask(void *argument);
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

  /* creation of AnglePID_Task */
  AnglePID_TaskHandle = osThreadNew(AnglePIDTask, NULL, &AnglePID_Task_attributes);

  /* creation of SpeedPID_Task */
  SpeedPID_TaskHandle = osThreadNew(SpeedPIDTask, NULL, &SpeedPID_Task_attributes);

  /* creation of SERIAL_Task */
  SERIAL_TaskHandle = osThreadNew(SERIALTask, NULL, &SERIAL_Task_attributes);

  /* creation of Turn_Task */
  Turn_TaskHandle = osThreadNew(TurnTask, NULL, &Turn_Task_attributes);

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
    ticks += 50; // 设定绝对周期为 50ms
    osDelayUntil(ticks); // 无论任务执行了多久，都会在固定的 50ms 节点唤醒
		
		OLED_Clear();
		OLED_Printf(0, 0, OLED_6X8, "Angle");											
		OLED_Printf(0, 8, OLED_6X8, "P:%05.2f", AnglePID.Kp);					//鏄剧ずKp
		OLED_Printf(0, 16, OLED_6X8, "I:%05.2f", AnglePID.Ki);				//鏄剧ずKi		
		OLED_Printf(0, 24, OLED_6X8, "D:%05.2f", AnglePID.Kd);				//鏄剧ずKd
		OLED_Printf(0, 32, OLED_6X8, "T:%05.2f", AnglePID.Target);		//鏄剧ず璁惧畾鍊�
		OLED_Printf(0, 40, OLED_6X8, "A:%05.2f", Angle);			//鏄剧ず瑙掑害浠ｆ浛瀹為檯鍊硷紙鍦ㄩ潪骞宠　鍔熻兘鐘舵€佷篃鑳藉埛鏂帮級
		OLED_Printf(0, 48, OLED_6X8, "O:%05.2f", AnglePID.Out);			//鏄剧ず杈撳嚭鍊�
		
		OLED_Printf(50, 0, OLED_6X8, "Speed");											
		OLED_Printf(50, 8, OLED_6X8, "%05.2f", SpeedPID.Kp);					//鏄剧ずKp
		OLED_Printf(50, 16, OLED_6X8, "%05.2f", SpeedPID.Ki);				//鏄剧ずKi		
		OLED_Printf(50, 24, OLED_6X8, "%05.2f", SpeedPID.Kd);				//鏄剧ずKd
		OLED_Printf(50, 32, OLED_6X8, "%05.2f", SpeedPID.Target);		//鏄剧ず璁惧畾鍊�
		OLED_Printf(50, 40, OLED_6X8, "%05.2f", AveSpeed);			//鏄剧ず骞冲潎閫熷害浠ｆ浛瀹為檯鍊硷紙鍦ㄩ潪骞宠　鍔熻兘鐘舵€佷篃鑳藉埛鏂帮級
		OLED_Printf(50, 48, OLED_6X8, "%05.2f", SpeedPID.Out);			//鏄剧ず杈撳嚭鍊�
		
		OLED_Printf(88, 0, OLED_6X8, "Turn");											
		OLED_Printf(88, 8, OLED_6X8, "%05.2f", TurnPID.Kp);					//鏄剧ずKp
		OLED_Printf(88, 16, OLED_6X8, "%05.2f", TurnPID.Ki);				//鏄剧ずKi		
		OLED_Printf(88, 24, OLED_6X8, "%05.2f", TurnPID.Kd);				//鏄剧ずKd
		OLED_Printf(88, 32, OLED_6X8, "%05.2f", TurnPID.Target);		//鏄剧ず璁惧畾鍊�
		OLED_Printf(88, 40, OLED_6X8, "%05.2f", DifSpeed);			//鏄剧ず宸垎閫熷害瀹為檯鍊硷紙鍦ㄩ潪骞宠　鍔熻兘鐘舵€佷篃鑳藉埛鏂帮級
		OLED_Printf(88, 48, OLED_6X8, "%05.2f", TurnPID.Out);			//鏄剧ず杈撳嚭鍊�
		
		OLED_Printf(0, 56, OLED_6X8, "GY:%d", GY);			//鏄剧ず杈撳嚭鍊�

		/*OLED鏇存柊*/
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
    ticks += 1; // 设定绝对周期为 10ms
    osDelayUntil(ticks); // 无论任务执行了多久，都会在固定的 10ms 节点唤醒
		
		//显示平衡车状态
		if(RunFlag) 
		{
			onLED(GPIOC,GPIO_PIN_13);
		}
		else
		{
			offLED(GPIOC,GPIO_PIN_13);
		}
		
		switch(Key_Scan())
		{
			case 1: 
				if(RunFlag)
				{
					RunFlag = 0;
				}
				else{
					PID_Init(&AnglePID);
					PID_Init(&SpeedPID);
					PID_Init(&TurnPID);
					RunFlag = 1;
				}
			break;
			case 2:																	break;
			case 3: 																break;
			case 4: 																break;
			default:																break;
		}
  }
  /* USER CODE END KEYTask */
}

/* USER CODE BEGIN Header_AnglePIDTask */
/**
* @brief Function implementing the AnglePID_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AnglePIDTask */
void AnglePIDTask(void *argument)
{
  /* USER CODE BEGIN AnglePIDTask */
	uint32_t ticks = osKernelGetTickCount();
	float alpha = 0.01;//用于计算角度值时的加权参数
  /* Infinite loop */
  for(;;)
  {
    ticks += 10; // 设定绝对周期为 10ms
    osDelayUntil(ticks); // 无论任务执行了多久，都会在固定的 10ms 节点唤醒
		
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
		
		GY -= 14;

		AngleAcc = -atan2( AX , AZ ) / 3.1415926535 * 180;
		
		
		AngleGyro =Angle + GY / 32768.0 * 2000 * 0.01;
		
		Angle = alpha * AngleAcc + (1 - alpha) * AngleGyro;
		
		//当小车倒地后自动停止平衡车功能
		if(Angle >= 50 || Angle <= -50)
		{
			RunFlag = 0;
		}			
		
		//当RunFlag置一时启动平衡车功能
		if(RunFlag)
		{
			AnglePID.Actual = Angle;
			PID_Update(&AnglePID);
			AvePWM = -AnglePID.Out;
			
			PWML =  AvePWM + DifPWM / 2;
			PWMR =  AvePWM - DifPWM / 2;
			
			if(PWML > 100) {PWML = 100;} else if(PWML < -100){PWML = -100;}
			if(PWMR > 100) {PWMR = 100;} else if(PWMR < -100){PWMR = -100;}
			
			Motor_SetLPWM(PWML);
			Motor_SetRPWM(PWMR);
		}
		else
		{
			Motor_SetLPWM(0);
			Motor_SetRPWM(0);
		}
  }
  /* USER CODE END AnglePIDTask */
}

/* USER CODE BEGIN Header_SpeedPIDTask */
/**
* @brief Function implementing the SpeedPID_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SpeedPIDTask */
void SpeedPIDTask(void *argument)
{
  /* USER CODE BEGIN SpeedPIDTask */
	uint32_t ticks = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    ticks += 50; // 设定绝对周期为 50ms
    osDelayUntil(ticks); // 无论任务执行了多久，都会在固定的 50ms 节点唤醒
		
		//1为左侧编码器，编码器有44个极性，0.05为读取周期，9.27666为减速比，单位rad/s
		SpeedL = Encoder_Get(1) / 44.0 / 0.05 / 9.27666;
		SpeedR = Encoder_Get(2) / 44.0 / 0.05 / 9.27666;
		
		AveSpeed = ( SpeedL + SpeedR ) / 2;
		DifSpeed =  SpeedL - SpeedR;
		
		//当RunFlag置一时启动平衡车功能
		if(RunFlag)
		{
			SpeedPID.Actual = AveSpeed;
			PID_Update(&SpeedPID);
			AnglePID.Target = SpeedPID.Out;
		}
		
  }
  /* USER CODE END SpeedPIDTask */
}

/* USER CODE BEGIN Header_SERIALTask */
/**
* @brief Function implementing the SERIAL_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SERIALTask */
void SERIALTask(void *argument)
{
  /* USER CODE BEGIN SERIALTask */
	uint32_t ticks = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
		ticks += 1; // 设定绝对周期为 1ms
    osDelayUntil(ticks); // 无论任务执行了多久，都会在固定的 1ms 节点唤醒
		
    Serial_Process();
		Serial_Printf("[plot,%f,%f,%f]",AnglePID.ErrorInt,SpeedPID.ErrorInt,TurnPID.ErrorInt);
  }
  /* USER CODE END SERIALTask */
}

/* USER CODE BEGIN Header_TurnTask */
/**
* @brief Function implementing the Turn_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TurnTask */
void TurnTask(void *argument)
{
  /* USER CODE BEGIN TurnTask */
	uint32_t ticks = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    ticks += 50; // 设定绝对周期为 10ms
    osDelayUntil(ticks); // 无论任务执行了多久，都会在固定的 10ms 节点唤醒
		
		if(RunFlag)
		{
			TurnPID.Actual = DifSpeed;
			PID_Update(&TurnPID);
			DifPWM = TurnPID.Out;
		}
		
  }
  /* USER CODE END TurnTask */
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
    ticks += 50; // 采样间隔 100ms
    osDelayUntil(ticks); 
		
		uint8_t ret = NRF24L01_Receive();
		
		if (ret == 1)
		{
			int8_t LH = NRF24L01_RxPacket[0];
			int8_t LV = NRF24L01_RxPacket[1];
			int8_t RH = NRF24L01_RxPacket[2];
			int8_t RV = NRF24L01_RxPacket[3];
			 
			taskENTER_CRITICAL();
			
			SpeedPID.Target = LV / 40.0;   /* 前后 */
			TurnPID.Target = RH / 25.0;    /* 转弯 */
			
			(void)LH; (void)RV;
			taskEXIT_CRITICAL();
		}
		
//		else if (ret == 2 || ret == 3)
//		{
//				/*驱动已自动重新初始化，可按需重试或记录*/
//		}

		/*-----------发送部分-----------*/
		
		
  }
  /* USER CODE END NRFTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* 覆盖 SERIAL.c 中的 __weak Serial_OnCommand */
void Serial_OnCommand(char *cmd)
{
    if (cmd == NULL || *cmd == '\0') return;

    /* 注意：SERIAL.c 传入的 cmd 已经去掉了首尾的 '[' 和 ']'，并且 \r \n \t \\ 已被还原 */

    char *Tag = strtok(cmd, ",");
    if (Tag == NULL) return;

    if (strcmp(Tag, "key") == 0)
    {
        char *Name   = strtok(NULL, ",");
        char *Action = strtok(NULL, ",");
        (void)Name; (void)Action;
        /* 这里可以执行按键动作 */
    }
    else if (strcmp(Tag, "slider") == 0)
    {
        char *Name  = strtok(NULL, ",");
        char *Value = strtok(NULL, ",");
        if (Name == NULL || Value == NULL) return;

        /* 更新 PID 参数时进入临界区，避免 MOTORTask 正在 PID_Update 中途被打断 */
        taskENTER_CRITICAL();

        if (strcmp(Name, "AngleKp") == 0)
        {
            AnglePID.Kp = atof(Value);
        }
        else if (strcmp(Name, "AngleKi") == 0)
        {
            AnglePID.Ki = atof(Value);
        }
        else if (strcmp(Name, "AngleKd") == 0)
        {
            AnglePID.Kd = atof(Value);
        }
				else if (strcmp(Name, "SpeedKp") == 0)
        {
            SpeedPID.Kp = atof(Value);
        }
        else if (strcmp(Name, "SpeedKi") == 0)
        {
            SpeedPID.Ki = atof(Value);
        }
        else if (strcmp(Name, "SpeedKd") == 0)
        {
            SpeedPID.Kd = atof(Value);
        }
				else if (strcmp(Name, "TurnKp") == 0)
        {
            TurnPID.Kp = atof(Value);
        }
        else if (strcmp(Name, "TurnKi") == 0)
        {
            TurnPID.Ki = atof(Value);
        }
        else if (strcmp(Name, "TurnKd") == 0)
        {
            TurnPID.Kd = atof(Value);
        }
				else if (strcmp(Name, "Offset") == 0)
        {
            AnglePID.Outoffset = atof(Value);
        }

        taskEXIT_CRITICAL();
    }
    else if (strcmp(Tag, "joystick") == 0)
    {
        char *sLH = strtok(NULL, ",");
        char *sLV = strtok(NULL, ",");
        char *sRH = strtok(NULL, ",");
        char *sRV = strtok(NULL, ",");
        if (sLH == NULL || sLV == NULL || sRH == NULL || sRV == NULL) return;

        int8_t LH = (int8_t)atoi(sLH);
        int8_t LV = (int8_t)atoi(sLV);
        int8_t RH = (int8_t)atoi(sRH);
        int8_t RV = (int8_t)atoi(sRV);
        (void)LH; (void)RV;   /* 暂时没用 */

        taskENTER_CRITICAL();
			
        SpeedPID.Target = LV / 40.0;   /* 前后 */
        TurnPID.Target = RH / 25.0;    /* 转弯 */
			
        taskEXIT_CRITICAL();
			
    }

    /* 其余命令不处理，不回调默认实现（默认实现是回显，容易刷屏） */
}

/* USER CODE END Application */

