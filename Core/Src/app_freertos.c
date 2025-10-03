/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cli.h"
#include "ms_work.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
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

/* USER CODE END Variables */
/* Definitions for cli */
osThreadId_t cliHandle;
uint32_t cli_Task01[ 512 ];
osStaticThreadDef_t cli_BlocTask01;
const osThreadAttr_t cli_attributes = {
  .name = "cli",
  .stack_mem = &cli_Task01[0],
  .stack_size = sizeof(cli_Task01),
  .cb_mem = &cli_BlocTask01,
  .cb_size = sizeof(cli_BlocTask01),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ms_work */
osThreadId_t ms_workHandle;
uint32_t ms_work_Task02[ 512 ];
osStaticThreadDef_t ms_work_lBlocTask02;
const osThreadAttr_t ms_work_attributes = {
  .name = "ms_work",
  .stack_mem = &ms_work_Task02[0],
  .stack_size = sizeof(ms_work_Task02),
  .cb_mem = &ms_work_lBlocTask02,
  .cb_size = sizeof(ms_work_lBlocTask02),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for uart1_sem */
osSemaphoreId_t uart1_semHandle;
osStaticSemaphoreDef_t uart1_sem_blk;
const osSemaphoreAttr_t uart1_sem_attributes = {
  .name = "uart1_sem",
  .cb_mem = &uart1_sem_blk,
  .cb_size = sizeof(uart1_sem_blk),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */
  /* creation of uart1_sem */
  uart1_semHandle = osSemaphoreNew(1, 1, &uart1_sem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of cli */
  cliHandle = osThreadNew(cli_task, NULL, &cli_attributes);

  /* creation of ms_work */
  ms_workHandle = osThreadNew(ms_work_task, NULL, &ms_work_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_cli_task */
/**
* @brief Function implementing the cli thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_cli_task */
void cli_task(void *argument)
{
  /* USER CODE BEGIN cli */
  /* Infinite loop */
  for(;;)
  {
    cli_proc();
    osDelay(10);
  }
  /* USER CODE END cli */
}

/* USER CODE BEGIN Header_ms_work_task */
/**
* @brief Function implementing the ms_work thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ms_work_task */
void ms_work_task(void *argument)
{
  /* USER CODE BEGIN ms_work */
  /* Infinite loop */
  for(;;)
  {
    ms_proc();
    osDelay(1);
  }
  /* USER CODE END ms_work */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

