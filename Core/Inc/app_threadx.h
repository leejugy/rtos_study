/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.h
  * @author  MCD Application Team
  * @brief   ThreadX applicative header file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_THREADX_H
#define __APP_THREADX_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern TX_SEMAPHORE uart1_sem;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Main thread defines -------------------------------------------------------*/
/* USER CODE BEGIN MTD */
static inline int sem_wait(TX_SEMAPHORE *sem)
{
    if (sem == NULL)
    {
        return -1;
    }
    return tx_semaphore_get(sem, TX_WAIT_FOREVER);
}

static inline int sem_post(TX_SEMAPHORE *sem)
{
    if (sem == NULL)
    {
        return -1;
    }
    return tx_semaphore_put(sem);
}

static inline bool check_expired(uint32_t *old_tick, uint32_t goal_tick)
{
    bool ret = (tx_time_get() - *old_tick >= goal_tick);
    if (ret)
    {
        *old_tick = tx_time_get();
    }
    return ret;
}

static inline bool check_not_expired(uint32_t *old_tick, uint32_t goal_tick)
{
    return !check_expired(old_tick, goal_tick);
}
/* USER CODE END MTD */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT App_ThreadX_Init(VOID *memory_ptr);
void MX_ThreadX_Init(void);
void valueNotSetted(ULONG thread_input);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_THREADX_H */
