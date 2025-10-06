/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
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
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sdmmc.h"
#include "usart.h"
#include "que_ctl.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* Main thread stack size */
#define FX_APP_THREAD_STACK_SIZE         4096
/* Main thread priority */
#define FX_APP_THREAD_PRIO               10
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Main thread global data structures.  */
TX_THREAD       fx_app_thread;

/* Buffer for FileX FX_MEDIA sector cache. */
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */
static int sd_file_create(sd_req_t *sd_req)
{
    UINT sd_status = 0;

    sd_status = fx_file_create(&sdio_disk, sd_req->route);
    if (sd_status != FX_SUCCESS)
    {
        printr("fail to create file : %d", sd_status);
        return -1;
    }

    sd_status = fx_media_flush(&sdio_disk);
    if (sd_status != FX_SUCCESS)
    {
        printr("file close fail : %d", sd_status);
        return -1;
    }

    return 1;
}

static int sd_file_write(sd_req_t *sd_req)
{
    UINT sd_status = 0;
    FX_FILE file = {0, };
    int ret = 1;

    if (!sd_req->buf)
    {
        printr("buf is null");
        return -1;
    }

    sd_status = fx_file_open(&sdio_disk, &file, sd_req->route, FX_OPEN_FOR_WRITE);
    if (sd_status != FX_SUCCESS)
    {
        printr("file open fail : %d", sd_status);
        return -1;
    }

    sd_status = fx_file_seek(&file, sd_req->seek);
    if (sd_status != FX_SUCCESS)
    {
        printr("file seek fail");
        ret = -1;
        goto close_out;
    }

    sd_status = fx_file_write(&file, sd_req->buf, sd_req->buf_size);
    if (sd_status != FX_SUCCESS)
    {
        printr("file write fail : %d", sd_status);
        ret = -1;
        goto close_out;
    }

close_out:
    sd_status = fx_file_close(&file);
    if (sd_status != FX_SUCCESS)
    {
        printr("file close fail : %d", sd_status);
        return -1;
    }

    sd_status = fx_media_flush(&sdio_disk);
    if (sd_status != FX_SUCCESS)
    {
        printr("file close fail : %d", sd_status);
        return -1;
    }

    return ret;
}

static int sd_file_read(sd_req_t *sd_req)
{
    UINT sd_status = 0;
    ULONG rd_size = 0;
    FX_FILE file = {0, };
    int ret = 1;

    if (!sd_req->buf)
    {
        printr("buf is null");
        ret = -1;
        goto out;
    }

    sd_status = fx_file_open(&sdio_disk, &file, sd_req->route, FX_OPEN_FOR_WRITE);
    if (sd_status != FX_SUCCESS)
    {
        printr("file open fail : %d", sd_status);
        ret = -1;
        goto out;
    }

    sd_status = fx_file_seek(&file, sd_req->seek);
    if (sd_status != FX_SUCCESS)
    {
        printr("file seek fail");
        ret = -1;
        goto close_out;
    }

    sd_status = fx_file_read(&file, sd_req->buf, sd_req->buf_size, &rd_size);
    if (sd_status != FX_SUCCESS)
    {
        printr("file read fail : %d", sd_status);
        ret = -1;
        goto close_out;
    }
    else
    {
        ret = rd_size;
    }
    

close_out:
    sd_status = fx_file_close(&file);
    if (sd_status != FX_SUCCESS)
    {
        printr("file close fail : %d", sd_status);
        return -1;
    }
out:
    if (sd_req->rd_size)
    {
        *(sd_req->rd_size) = ret;
    }

    return ret;
}

static int sd_mkdir(sd_req_t *sd_req)
{
    UINT sd_status = 0;

    sd_status = fx_directory_create(&sdio_disk, sd_req->route);
    if (sd_status != FX_SUCCESS)
    {
        printr("fail to create directory : %d", sd_status);
        return -1;
    }

    sd_status = fx_media_flush(&sdio_disk);
    if (sd_status != FX_SUCCESS)
    {
        printr("file close fail : %d", sd_status);
        return -1;
    }

    return 1;
}

static int sd_remove(sd_req_t *sd_req)
{
    UINT sd_status1 = 0;
    UINT sd_status2 = 0;

    sd_status1 = fx_file_delete(&sdio_disk, sd_req->route);
    if (sd_status1 != FX_SUCCESS)
    {
        sd_status2 = fx_directory_delete(&sdio_disk, sd_req->route);
        if (sd_status2 != FX_SUCCESS)
        {
            printr("fail to delete file or directory : %d, %d", sd_status1, sd_status2);
        }
    }
    
    sd_status1 = fx_media_flush(&sdio_disk);
    if (sd_status1 != FX_SUCCESS)
    {
        printr("file close fail : %d", sd_status1);
        return -1;
    }

    return 1;
}

static void sd_req_proc(sd_req_t *sd_req)
{
    switch (sd_req->req)
    {
    case SD_WRITE:
        sd_file_write(sd_req);
        break;
    case SD_READ:
        sd_file_read(sd_req);
        break;
    case SD_CREATE:
        sd_file_create(sd_req);
        break;
    case SD_MKDIR:
        sd_mkdir(sd_req);
        break;
    case SD_REMOVE:
        sd_remove(sd_req);
        break;
    }

    if (sd_req->req_end)
    {
        *(sd_req->req_end) = true;
    }
    memset(sd_req, 0, sizeof(sd_req_t));
}
/* USER CODE END PFP */

/**
  * @brief  Application FileX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
*/
UINT MX_FileX_Init(VOID *memory_ptr)
{
  UINT ret = FX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  VOID *pointer;

/* USER CODE BEGIN MX_FileX_MEM_POOL */

/* USER CODE END MX_FileX_MEM_POOL */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*Allocate memory for the main thread's stack*/
  ret = tx_byte_allocate(byte_pool, &pointer, FX_APP_THREAD_STACK_SIZE, TX_NO_WAIT);

/* Check FX_APP_THREAD_STACK_SIZE allocation*/
  if (ret != FX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

/* Create the main thread.  */
  ret = tx_thread_create(&fx_app_thread, FX_APP_THREAD_NAME, fx_app_thread_entry, 0, pointer, FX_APP_THREAD_STACK_SIZE,
                         FX_APP_THREAD_PRIO, FX_APP_PREEMPTION_THRESHOLD, FX_APP_THREAD_TIME_SLICE, FX_APP_THREAD_AUTO_START);

/* Check main thread creation */
  if (ret != FX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

/* USER CODE BEGIN MX_FileX_Init */

/* USER CODE END MX_FileX_Init */

/* Initialize FileX.  */
  fx_system_initialize();

/* USER CODE BEGIN MX_FileX_Init 1*/

/* USER CODE END MX_FileX_Init 1*/

  return ret;
}

/**
 * @brief  Main thread entry.
 * @param thread_input: ULONG user argument used by the thread entry
 * @retval none
*/
 void fx_app_thread_entry(ULONG thread_input)
 {

  UINT sd_status = FX_SUCCESS;

/* USER CODE BEGIN fx_app_thread_entry 0*/
  sd_req_t sd_req = {0, };
/* USER CODE END fx_app_thread_entry 0*/

/* Open the SD disk driver */
  sd_status =  fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));

/* Check the media open sd_status */
  if (sd_status != FX_SUCCESS)
  {
     /* USER CODE BEGIN SD DRIVER get info error */
    while(1);
    /* USER CODE END SD DRIVER get info error */
  }

/* USER CODE BEGIN fx_app_thread_entry 1*/
    while(1)
    {
        if (que_pop(&sd_que, &sd_req, sizeof(sd_req)) > 0)
        {
            sd_req_proc(&sd_req);
        }
        tx_thread_sleep(1);
    }
/* USER CODE END fx_app_thread_entry 1*/
  }

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
