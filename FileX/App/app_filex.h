
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.h
  * @author  MCD Application Team
  * @brief   FileX applicative header file
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
#ifndef __APP_FILEX_H__
#define __APP_FILEX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"
#include "fx_stm32_sd_driver.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_FileX_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* Main thread Name */
#ifndef FX_APP_THREAD_NAME
  #define FX_APP_THREAD_NAME "FileX app thread"
#endif

/* Main thread time slice */
#ifndef FX_APP_THREAD_TIME_SLICE
  #define FX_APP_THREAD_TIME_SLICE TX_NO_TIME_SLICE
#endif

/* Main thread auto start */
#ifndef FX_APP_THREAD_AUTO_START
  #define FX_APP_THREAD_AUTO_START TX_AUTO_START
#endif

/* Main thread preemption threshold */
#ifndef FX_APP_PREEMPTION_THRESHOLD
  #define FX_APP_PREEMPTION_THRESHOLD FX_APP_THREAD_PRIO
#endif

/* fx sd volume name */
#ifndef FX_SD_VOLUME_NAME
  #define FX_SD_VOLUME_NAME "STM32_SDIO_DISK"
#endif
/* fx sd number of FATs */
#ifndef FX_SD_NUMBER_OF_FATS
  #define FX_SD_NUMBER_OF_FATS                1
#endif

/* fx sd Hidden sectors */
#ifndef FX_SD_HIDDEN_SECTORS
  #define FX_SD_HIDDEN_SECTORS               0
#endif

/* USER CODE BEGIN PD */
typedef enum{
    SD_WRITE,
    SD_READ,
    SD_CREATE,
    SD_MKDIR,
    SD_REMOVE,
}SD_REQUEST;
typedef struct
{
    char route[FILE_ROUTE_LEN];
    uint8_t *buf;
    int *rd_size;
    size_t buf_size;
    uint32_t seek;
    SD_REQUEST req;
    bool *req_end;
}sd_req_t;

typedef struct
{
    char route[FILE_ROUTE_LEN];
    FX_FILE file;
    uint8_t opt; /* @ref FX_OPEN_FOR_READ_FAST */
}sd_handle_t;

#define sd_req_end_wait(sd_req) while(!(*(((sd_req_t*) sd_req)->req_end))) \
{                          \
    tx_thread_relinquish();\
}
int sd_open(sd_handle_t *sd);
int sd_write(sd_handle_t *sd, uint32_t seek, uint8_t *buf, size_t buf_size);
int sd_read(sd_handle_t *sd, uint32_t seek, uint8_t *buf, size_t buf_size);
int sd_close(sd_handle_t *sd);
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
#ifdef __cplusplus
}
#endif
#endif /* __APP_FILEX_H__ */
