/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
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
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "cli.h"
#include "ms_work.h"
#include "que_ctl.h"
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
/* USER CODE BEGIN PV */
/* cli task */
TX_SEMAPHORE uart1_sem = {0, };
TX_SEMAPHORE sd_que_sem = {0, };
TX_THREAD cli_tcb = {0, };
ULONG cli_task_buf[1024] = {0, };
/* ms task */
TX_THREAD ms_tcb = {0, };
ULONG ms_task_buf[1024] = {0, };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID cli_thread(ULONG id)
{
    (void) id;
    while(1)
    {
        cli_proc();
        tx_thread_sleep(10);
    }
}

VOID ms_thread(ULONG id)
{
    (void) id;
    while(1)
    {
        ms_proc();
        tx_thread_sleep(1);
    }
}
/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;

  /* USER CODE BEGIN App_ThreadX_MEM_POOL */
    que_init();
    uart_init();
  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */
    if (tx_thread_create(&cli_tcb, "cli_tcb", 
        cli_thread, (ULONG)NULL, cli_task_buf, sizeof(cli_task_buf), 
        15, 10, 100, TX_AUTO_START) != TX_SUCCESS)
    {
        printfail("create cli task");
    }

    if (tx_thread_create(&ms_tcb, "ms_tcb", 
        ms_thread, (ULONG)NULL, ms_task_buf, sizeof(ms_task_buf), 
        15, 10, 100, TX_AUTO_START) != TX_SUCCESS)
    {
        printfail("create cli task");
    }

  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
