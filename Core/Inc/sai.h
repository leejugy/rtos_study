/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sai.h
  * @brief   This file contains all the function prototypes for
  *          the sai.c file
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
#ifndef __SAI_H__
#define __SAI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "app_threadx.h"
#include "app_filex.h"
#include "que_ctl.h"
/* USER CODE END Includes */

extern SAI_HandleTypeDef hsai_BlockA1;

/* USER CODE BEGIN Private defines */
/* underrun occur when this buffer size is too short... */
#define SAI_BUF_SIZE (1 << 13)
#define SAI_PRE_BUF_SIZE (SAI_BUF_SIZE >> 1)

typedef enum
{
    SAI1_TX_IDX,
    SAI_TX_IDX_MAX,
}SAI_TX_IDX;

typedef enum
{
    SAI_TX_IDLE,
    SAI_TX_HALF_CPLT,
    SAI_TX_CPLT,
}SAI_TX_FLAGS;

typedef enum
{
    SAI_IDLE,
    SAI_CONTINUE,
    SAI_RESUME,
    SAI_STOP,
    SAI_PAUSE,
    SAI_NEW_MUSIC,
    SAI_MUTE,
    SAI_UP_VOL,
    SAI_DOWN_VOL,
    SAI_WAIT_END,
}SAI_CTL;
/* USER CODE END Private defines */

void MX_SAI1_Init(void);

/* USER CODE BEGIN Prototypes */
typedef struct __attribute__((packed)) 
{
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    char subchunk_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channel;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bit_per_sample;
}wav_header_t;

typedef struct
{
    char chunk_id[4];
    uint32_t chunk_size;
}wav_chunk_t;

typedef struct
{
    void (*init)();
    void (*conv_s16_vol)(int16_t *, size_t);
    void (*vol_ctl)(uint8_t);
    void (*mute)(bool);
}sai_func_t;

typedef struct
{
    SAI_HandleTypeDef *handle;
    sd_handle_t sd;
    que_ctl_t *q;
    uint8_t buf[SAI_BUF_SIZE];
    uint8_t pre_buf[SAI_PRE_BUF_SIZE];
    uint8_t get_buf[SAI_PRE_BUF_SIZE];
    uint8_t tx_flag; /* @ref SAI_FLAGS */
    uint8_t volume;
    uint8_t flag;
    uint8_t old_flag;
    uint32_t seek_idx;
    uint32_t end_cnt;
    sai_func_t func;
}sai_tx_t;

typedef struct
{
    uint8_t flag; /* @ref SAI_CTL */
    char route[FILE_ROUTE_LEN];
}sai_tx_req_t;

extern sai_tx_t sai_tx[SAI_TX_IDX_MAX];

void sai_init();
void sai_tx_proc(sai_tx_t *s_t);

#define sai1_tx_proc() sai_tx_proc(&sai_tx[SAI1_TX_IDX])
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SAI_H__ */

