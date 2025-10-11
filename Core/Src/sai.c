/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : SAI.c
  * Description        : This file provides code for the configuration
  *                      of the SAI instances.
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
#include "sai.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
#include "que_ctl.h"
#include "app_filex.h"
#include "pcm5102a.h"

sai_tx_t sai_tx[SAI_TX_IDX_MAX] = {
    [SAI1_TX_IDX].handle = &hsai_BlockA1,
    [SAI1_TX_IDX].tx_flag = SAI_IDLE,
    [SAI1_TX_IDX].buf = {0, },
    [SAI1_TX_IDX].pre_buf = {0, },
    [SAI1_TX_IDX].get_buf = {0, },
    [SAI1_TX_IDX].flag = SAI_TX_IDLE,
    [SAI1_TX_IDX].volume = 30,
    [SAI1_TX_IDX].seek_idx = 0,
    [SAI1_TX_IDX].old_flag = SAI_IDLE,
    [SAI1_TX_IDX].q = &i2s1_tx_que,
    /* function */
    [SAI1_TX_IDX].func.init = pcm_5102a_init,
    [SAI1_TX_IDX].func.mute = pcm_5102a_mute,
    [SAI1_TX_IDX].func.conv_s16_vol = pcm_5102a_s16_conv,
    [SAI1_TX_IDX].func.vol_ctl = pcm_5102a_volume_set,
};

static void __sai_init(sai_tx_t *s_t)
{
    s_t->func.init();
    printok("sai init complete");
}

void sai_init()
{
    __sai_init(&sai_tx[SAI1_TX_IDX]);
}

static int sai_tx_dma_start(sai_tx_t *s_t)
{
    if (HAL_SAI_Transmit_DMA(s_t->handle, s_t->buf, SAI_BUF_SIZE/sizeof(uint16_t)) != HAL_OK)
    {
        return -1;
    }

    return 1;
}

static int sai_tx_dma_stop(sai_tx_t *s_t)
{
    if (HAL_SAI_Abort(s_t->handle) != HAL_OK)
    {
        return -1;
    }

    return 1;
}

static int sai_tx_dma_pause(sai_tx_t *s_t)
{
    return sai_tx_dma_stop(s_t);
}

static int sai_tx_dma_resume(sai_tx_t *s_t)
{
    if (sai_tx_dma_start(s_t) != HAL_OK)
    {
        return -1;
    }

    return 1;
}

static void sai_force_close(sai_tx_t *s_t, bool close_sd)
{
    s_t->flag = SAI_IDLE;
    s_t->old_flag = SAI_IDLE;
    s_t->seek_idx = 0;
    if (close_sd)
    {
        sd_close(&s_t->sd);
    }
    que_flush(s_t->q);
    memset(s_t->sd.route, 0, sizeof(s_t->sd.route));
    memset(s_t->buf, 0, sizeof(s_t->buf));
    sai_tx_dma_stop(s_t);
}

static int wav_header_parse(sai_tx_t *s_t, uint8_t *buf, size_t buf_size)
{
    size_t idx = sizeof(wav_header_t);
    if (idx >= buf_size)
    {
        printg("wav file is too short");
        return -1;
    }

    wav_header_t *header = (wav_header_t *)buf;

    printg("playing WAV: %dBit, %ldhz, %dch", 
            header->bit_per_sample, header->sample_rate,
            header->num_channel);
    
    wav_chunk_t *chunk = (wav_chunk_t *)&buf[idx];
    idx += sizeof(wav_chunk_t);
    if (idx >= buf_size)
    {
        printr("fail to find header");
        return -1;
    }
    
    while (strncmp(chunk->chunk_id, "data", sizeof(chunk->chunk_id)))
    {
        idx += chunk->chunk_size;
        if (idx >= buf_size)
        {
            printr("fail to find header");
            return -1;
        }

        chunk = (wav_chunk_t *)&buf[idx];
        idx += sizeof(wav_chunk_t);
        if (idx >= buf_size)
        {
            printr("fail to find header");
            return -1;
        }
    }

    s_t->seek_idx = idx;
    return 1;
}

static void inline sai_tx_continue(sai_tx_t *s_t)
{
    int len = 0;
    static bool end_flag = false;

    if (s_t->old_flag != s_t->flag)
    {
        s_t->old_flag = s_t->flag;
    }

    switch (s_t->tx_flag)
    {
    case SAI_TX_IDLE:
        if (!que_full(s_t->q))
        {
            len = sd_read(&s_t->sd, s_t->seek_idx, s_t->pre_buf, SAI_PRE_BUF_SIZE);
            if (len < 0)
            {
                printr("read fail");
                sai_force_close(s_t, true);
                return;
            }

            if (len < SAI_PRE_BUF_SIZE && len >= 0)
            {
                end_flag = true;
            }

            if (s_t->func.conv_s16_vol && len > 0)
            {
                s_t->func.conv_s16_vol((int16_t *)s_t->pre_buf, len);
            }

            if (que_push(s_t->q, s_t->pre_buf, SAI_PRE_BUF_SIZE) < 0)
            {
                printr("push fail");
            }

            s_t->seek_idx += len;
        }
        break;

    case SAI_TX_HALF_CPLT:
    case SAI_TX_CPLT:
        if (end_flag)
        {
            s_t->old_flag = s_t->flag;
            s_t->flag = SAI_WAIT_END;
            s_t->end_cnt = tx_time_get();
            end_flag = false;
        }

        s_t->tx_flag = SAI_TX_IDLE;
        break;
    }
}

static void inline sai_tx_resume(sai_tx_t *s_t)
{
    if (s_t->old_flag == SAI_CONTINUE ||
        s_t->old_flag == SAI_NEW_MUSIC ||
        s_t->old_flag == SAI_RESUME)
    {
        s_t->flag = s_t->old_flag;
    }
    else if (s_t->old_flag == SAI_PAUSE)
    {
        sai_tx_dma_resume(s_t);
        s_t->func.mute(false);
        s_t->old_flag = s_t->flag;
        s_t->flag = SAI_CONTINUE;
    }
    else
    {
        s_t->old_flag = SAI_IDLE;
        s_t->flag = SAI_IDLE;
    }
}

static void inline sai_tx_stop(sai_tx_t *s_t)
{
    sai_force_close(s_t, true);
}

static void inline sai_tx_pause(sai_tx_t *s_t)
{
    if (s_t->old_flag == SAI_CONTINUE ||
        s_t->old_flag == SAI_RESUME || 
        s_t->old_flag == SAI_NEW_MUSIC)
    {
        sai_tx_dma_pause(s_t);
        s_t->func.mute(true);
        s_t->old_flag = SAI_PAUSE;
    }
    else if (s_t->old_flag != SAI_PAUSE)
    {
        s_t->old_flag = SAI_IDLE;
        s_t->flag = SAI_IDLE;
    }
}


static void inline sai_tx_new_music(sai_tx_t *s_t)
{
    if (s_t->old_flag == SAI_CONTINUE || 
        s_t->old_flag == SAI_NEW_MUSIC ||
        s_t->old_flag == SAI_RESUME ||
        s_t->old_flag == SAI_PAUSE)
    {
        s_t->flag = s_t->old_flag;
        printr("already in progress");
        return;
    }

    s_t->seek_idx = 0;
    s_t->sd.opt = FX_OPEN_FOR_READ_FAST;

    if (sd_open(&s_t->sd) < 0)
    {
        printr("fail to open file");
        sai_force_close(s_t, false);
        return;
    }

    int len = sd_read(&s_t->sd, s_t->seek_idx, s_t->pre_buf, SAI_PRE_BUF_SIZE);

    if (len < 0)
    {
        printr("fail to read file");
        sai_force_close(s_t, true);
        return;
    }

    if (wav_header_parse(s_t, s_t->pre_buf, len) < 0)
    {
        printr("fail to parse wav");
        sai_force_close(s_t, true);
        return;
    }

    len = sd_read(&s_t->sd, s_t->seek_idx, s_t->pre_buf, SAI_PRE_BUF_SIZE);
    if (len < 0)
    {
        printr("read fail");
        sai_force_close(s_t, true);
        return;
    }

    if (s_t->func.conv_s16_vol)
    {
        s_t->func.conv_s16_vol((int16_t *)s_t->pre_buf, len);
    }
    s_t->func.mute(false);
    s_t->old_flag = s_t->flag;
    s_t->flag = SAI_CONTINUE;
    s_t->seek_idx += len;

    while (!que_full(s_t->q))
    {
        len = sd_read(&s_t->sd, s_t->seek_idx, s_t->pre_buf, SAI_PRE_BUF_SIZE);
        if (len < 0)
        {
            printr("read fail");
            sai_force_close(s_t, true);
            return;
        }

        if (s_t->func.conv_s16_vol)
        {
            s_t->func.conv_s16_vol((int16_t *)s_t->pre_buf, len);
        }

        if (que_push(s_t->q, s_t->pre_buf, SAI_PRE_BUF_SIZE) < 0)
        {
            printr("push fail");
        }

        s_t->seek_idx += len;
    }

    sai_tx_dma_start(s_t);
}

static void inline sai_tx_mute(sai_tx_t *s_t)
{
    s_t->func.mute(true);
    s_t->flag = s_t->old_flag;
}

static void inline sai_tx_up_vol(sai_tx_t *s_t)
{
    if (s_t->volume < 100)
    {
        s_t->volume++;
    }
    else
    {
        printr("vol max");
    }
    s_t->func.vol_ctl(s_t->volume);
    s_t->flag = s_t->old_flag;
}

static void inline sai_tx_down_vol(sai_tx_t *s_t)
{
    if (s_t->volume > 0)
    {
        s_t->volume--;   
    }
    else
    {
        printr("vol min");
    }
    s_t->func.vol_ctl(s_t->volume);
    s_t->flag = s_t->old_flag;
}

static void inline sai_tx_vol_ctl(sai_tx_t *s_t)
{
    if (s_t->volume >= 0 &&
        s_t->volume <= 100)
    {
        s_t->func.vol_ctl(s_t->volume);    
    }
    else
    {
        printr("invalid volume");
    }
    s_t->flag = s_t->old_flag;
}

static void inline sai_tx_wait_end(sai_tx_t *s_t)
{
    if (check_expired(s_t->end_cnt, 1000))
    {
        sai_force_close(s_t, true);
        printg("end sai");
    }
}

static void __sai_tx_proc(sai_tx_t *s_t)
{
    switch (s_t->flag)
    {
    case SAI_IDLE:
        break;
    case SAI_CONTINUE:
        sai_tx_continue(s_t);
        break;
    case SAI_RESUME:
        sai_tx_resume(s_t);
        break;
    case SAI_STOP:
        sai_tx_stop(s_t);
        break;
    case SAI_PAUSE:
        sai_tx_pause(s_t);
        break;
    case SAI_NEW_MUSIC:
        sai_tx_new_music(s_t);
        break;
    case SAI_MUTE:
        sai_tx_mute(s_t);
        break;
    case SAI_UP_VOL:
        sai_tx_up_vol(s_t);
        break;
    case SAI_DOWN_VOL:
        sai_tx_down_vol(s_t);
        break;
    case SAI_VOL_CTL:
        sai_tx_vol_ctl(s_t);
        break;
    case SAI_WAIT_END:
        sai_tx_wait_end(s_t);
        break;
    }
}


static void sai_tx_req_proc(sai_tx_req_t *tx_req, sai_tx_t *s_t)
{
    switch (tx_req->flag)
    {
    case SAI_IDLE:
    case SAI_CONTINUE:
    case SAI_STOP:
    case SAI_PAUSE:
    case SAI_MUTE:
    case SAI_UP_VOL:
    case SAI_DOWN_VOL:
        break;
    case SAI_VOL_CTL:
        if (tx_req->req.volume >= 0 && 
            tx_req->req.volume <= 100)
        {
            s_t->volume = tx_req->req.volume;
        }
        else
        {
            printr("invalid volume");
        }
        break;
    case SAI_NEW_MUSIC:
        strcpy(s_t->sd.route, tx_req->req.route);
        break;
    }

    s_t->old_flag = s_t->flag;
    s_t->flag = tx_req->flag;
}

void sai_tx_proc(sai_tx_t *s_t)
{
    sai_tx_req_t tx_req = {0, };

    if (que_pop(&sai1_tx_que, &tx_req, sizeof(tx_req)) > 0)
    {
        sai_tx_req_proc(&tx_req, s_t);
    }

    __sai_tx_proc(s_t);
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    int idx = 0;

    for (idx = 0; idx < SAI_TX_IDX_MAX; idx++)
    {
        if (sai_tx[idx].handle == hsai)
        {
            que_pop(sai_tx[idx].q, sai_tx[idx].get_buf, SAI_PRE_BUF_SIZE);
            memcpy(&sai_tx[idx].buf[SAI_PRE_BUF_SIZE], sai_tx[idx].get_buf, SAI_PRE_BUF_SIZE);
            sai_tx[idx].tx_flag = SAI_TX_CPLT;
            break;
        }
    } 
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    int idx = 0;

    for (idx = 0; idx < SAI_TX_IDX_MAX; idx++)
    {
        if (sai_tx[idx].handle == hsai)
        {
            que_pop(sai_tx[idx].q, sai_tx[idx].get_buf, SAI_PRE_BUF_SIZE);
            memcpy(&sai_tx[idx].buf, sai_tx[idx].get_buf, SAI_PRE_BUF_SIZE);
            sai_tx[idx].tx_flag = SAI_TX_HALF_CPLT;
            break;
        }
    } 
}
/* USER CODE END 0 */

SAI_HandleTypeDef hsai_BlockA1;
DMA_NodeTypeDef Node_GPDMA1_Channel1;
DMA_QListTypeDef List_GPDMA1_Channel1;
DMA_HandleTypeDef handle_GPDMA1_Channel1;

/* SAI1 init function */
void MX_SAI1_Init(void)
{

  /* USER CODE BEGIN SAI1_Init 0 */

  /* USER CODE END SAI1_Init 0 */

  /* USER CODE BEGIN SAI1_Init 1 */

  /* USER CODE END SAI1_Init 1 */

  hsai_BlockA1.Instance = SAI1_Block_A;
  hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;
  hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_HF;
  hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_44K;
  hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA1.Init.MckOutput = SAI_MCK_OUTPUT_ENABLE;
  hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, 2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SAI1_Init 2 */

  /* USER CODE END SAI1_Init 2 */

}
static uint32_t SAI1_client =0;

void HAL_SAI_MspInit(SAI_HandleTypeDef* saiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  DMA_NodeConfTypeDef NodeConfig;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
/* SAI1 */
    if(saiHandle->Instance==SAI1_Block_A)
    {
    /* SAI1 clock enable */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    PeriphClkInitStruct.PLL3.PLL3Source = RCC_PLL3_SOURCE_HSI;
    PeriphClkInitStruct.PLL3.PLL3M = 10;
    PeriphClkInitStruct.PLL3.PLL3N = 134;
    PeriphClkInitStruct.PLL3.PLL3P = 38;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3_VCIRANGE_3;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3_VCORANGE_WIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.PLL3.PLL3ClockOut = RCC_PLL3_DIVP;
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL3P;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    if (SAI1_client == 0)
    {
       __HAL_RCC_SAI1_CLK_ENABLE();

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(SAI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SAI1_IRQn);
    }
    SAI1_client ++;

    /**SAI1_A_Block_A GPIO Configuration
    PC0     ------> SAI1_MCLK_A
    PC1     ------> SAI1_SD_A
    PC5     ------> SAI1_FS_A
    PC6     ------> SAI1_SCK_A
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Peripheral DMA init*/

    NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request = GPDMA1_REQUEST_SAI1_A;
    NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction = DMA_MEMORY_TO_PERIPH;
    NodeConfig.Init.SrcInc = DMA_SINC_INCREMENTED;
    NodeConfig.Init.DestInc = DMA_DINC_FIXED;
    NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
    NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
    NodeConfig.Init.SrcBurstLength = 1;
    NodeConfig.Init.DestBurstLength = 1;
    NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel1, NULL, &Node_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel1.Instance = GPDMA1_Channel1;
    handle_GPDMA1_Channel1.InitLinkedList.Priority = DMA_HIGH_PRIORITY;
    handle_GPDMA1_Channel1.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel1.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel1.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel1.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel1, &List_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(saiHandle, hdmatx, handle_GPDMA1_Channel1);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel1, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }

    }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef* saiHandle)
{

/* SAI1 */
    if(saiHandle->Instance==SAI1_Block_A)
    {
    SAI1_client --;
    if (SAI1_client == 0)
      {
      /* Peripheral clock disable */
       __HAL_RCC_SAI1_CLK_DISABLE();
      HAL_NVIC_DisableIRQ(SAI1_IRQn);
      }

    /**SAI1_A_Block_A GPIO Configuration
    PC0     ------> SAI1_MCLK_A
    PC1     ------> SAI1_SD_A
    PC5     ------> SAI1_FS_A
    PC6     ------> SAI1_SCK_A
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_6);

    HAL_DMA_DeInit(saiHandle->hdmatx);
    }
}

/**
  * @}
  */

/**
  * @}
  */
