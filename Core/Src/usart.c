/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "app_freertos.h"
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
DMA_NodeTypeDef Node_GPDMA1_Channel0;
DMA_QListTypeDef List_GPDMA1_Channel0;
DMA_HandleTypeDef handle_GPDMA1_Channel0;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  DMA_NodeConfTypeDef NodeConfig= {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* GPDMA1_REQUEST_USART1_RX Init */
    NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request = GPDMA1_REQUEST_USART1_RX;
    NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength = 1;
    NodeConfig.Init.DestBurstLength = 1;
    NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT1|DMA_DEST_ALLOCATED_PORT1;
    NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel0, NULL, &Node_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel0.Instance = GPDMA1_Channel0;
    handle_GPDMA1_Channel0.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel0.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel0.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
    handle_GPDMA1_Channel0.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel0.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel0, &List_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel0);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
uart_t uart[UART_IDX_MAX] = {
    [UART1_IDX].handle = &huart1,
    [UART1_IDX].rx_mode = UART_MODE_DMA,
    [UART1_IDX].tx_mode = UART_MODE_POLLING,
    [UART1_IDX].sem = &uart1_semHandle,
};

static void uart_dma_rx_init(uart_t *ut)
{
    if (ut->rx_mode == UART_MODE_DMA)
    {
        if (HAL_UART_Receive_DMA(
            ut->handle, ut->rx.buf, sizeof(ut->rx.buf)) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

static int uart_dma_recv(uart_t *ut, uint8_t *buf, uint16_t buf_size)
{
    if (ut->rx_mode != UART_MODE_DMA)
    {
        return -1;
    }

    int idx = 0;

    ut->rx.rear = sizeof(ut->rx.buf) - __HAL_DMA_GET_COUNTER(ut->handle->hdmarx);
    if (ut->rx.rear == ut->rx.front)
    {
        return -2;
    }

    for (idx = 0; idx < buf_size; idx++)
    {
        buf[idx] = ut->rx.buf[ut->rx.front++];
        ut->rx.front &= (UART_TRX_BUF_SIZE - 1);
        if (ut->rx.rear == ut->rx.front)
        {
            break;
        }
    }
    return idx + 1;
}

static int uart_dma_send(uart_t *ut, uint8_t *buf, uint16_t buf_size)
{
    if (ut->tx_mode != UART_MODE_DMA)
    {
        return -1;
    }

    if (UART_TRX_BUF_SIZE < buf_size)
    {
        return -2;
    }

    if (ut->tx.sending)
    {
        return -3;
    }

    memcpy(ut->tx.buf, buf, buf_size);
    if (HAL_UART_Transmit_DMA(
        ut->handle, ut->tx.buf, buf_size) != HAL_OK)
    {
        return -1;
    }
    ut->tx.sending = true;
    return buf_size;
}

static void uart_it_rx_init(uart_t *ut)
{
    if (ut->rx_mode == UART_MODE_IT)
    {
        if (HAL_UART_Receive_IT(
            ut->handle, ut->rx.buf, sizeof(ut->rx.buf)) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

static int uart_it_send(uart_t *ut, uint8_t *buf, uint16_t buf_size)
{
    if (ut->tx_mode != UART_MODE_IT)
    {
        return -1;
    }

    if (UART_TRX_BUF_SIZE < buf_size)
    {
        return -2;
    }

    if (ut->tx.sending)
    {
        return -3;
    }

    memcpy(ut->tx.buf, buf, buf_size);
    if (HAL_UART_Transmit_IT(
        ut->handle, ut->tx.buf, buf_size) != HAL_OK)
    {
        return -1;
    }
    ut->tx.sending = true;
    return buf_size;
}


static int uart_it_recv(uart_t *ut, uint8_t *buf, uint16_t buf_size)
{
    if (ut->rx_mode != UART_MODE_IT)
    {
        return -1;
    }

    int idx = 0;

    ut->rx.rear = ut->handle->RxXferSize - ut->handle->RxXferCount;
    if (ut->rx.rear == ut->rx.front)
    {
        return -2;
    }
    for (idx = 0; idx < buf_size; idx++)
    {
        buf[idx] = ut->rx.buf[ut->rx.front++];
        ut->rx.front &= (UART_TRX_BUF_SIZE - 1);
        if (ut->rx.rear == ut->rx.front)
        {
            break;
        }
    }
    return idx + 1;
}

static int uart_poll_recv(uart_t *ut, uint8_t *buf, uint16_t buf_size)
{
    if (ut->rx_mode != UART_MODE_POLLING)
    {
        return -1;
    }

    if (HAL_UART_Receive(
        ut->handle, buf, buf_size, 100) != HAL_OK)
    {
        return -1;
    }

    return ut->handle->RxXferSize - ut->handle->RxXferCount;
}

static int uart_poll_send(uart_t *ut, uint8_t *buf, uint16_t buf_size)
{
    if (ut->tx_mode != UART_MODE_POLLING)
    {
        return -1;
    }

    if (HAL_UART_Transmit(
        ut->handle, buf, buf_size, 100) != HAL_OK)
    {
        return -1;
    }

    return buf_size;
}

static void __uart_init(uart_t *ut)
{
    switch (ut->rx_mode)
    {
    case UART_MODE_DMA:
        uart_dma_rx_init(ut);
        break;

    case UART_MODE_IT:
        uart_it_rx_init(ut);
    
    case UART_MODE_POLLING:
    default:
        break;
    }

    memset(&ut->tx, 0, sizeof(ut->tx));
    memset(&ut->rx, 0, sizeof(ut->rx));
}

void uart_init()
{
    int idx = 0;

    for (idx = 0; idx < UART_IDX_MAX; idx++)
    {
        __uart_init(&uart[idx]);
    }
    printok("UART init success");
}

int uart_recv(UART_IDX idx, uint8_t *buf, uint16_t buf_size)
{
    int ret = 0;

    sem_wait(uart[idx].sem);
    switch (uart[idx].rx_mode)
    {
    case UART_MODE_DMA:
        ret = uart_dma_recv(&uart[idx], buf, buf_size);
        break;

    case UART_MODE_IT:
        ret = uart_it_recv(&uart[idx], buf, buf_size);
        break;
    
    case UART_MODE_POLLING:
        ret = uart_poll_recv(&uart[idx], buf, buf_size);
        break;

    default:
        ret = -1;
        break;
    }
    sem_post(uart[idx].sem);
    return ret;
}

int uart_send(UART_IDX idx, uint8_t *buf, uint16_t buf_size)
{
    int ret = 0;

    sem_wait(uart[idx].sem);
    switch (uart[idx].tx_mode)
    {
    case UART_MODE_DMA:
        ret = uart_dma_send(&uart[idx], buf, buf_size);
        break;

    case UART_MODE_IT:
        ret = uart_it_send(&uart[idx], buf, buf_size);
        break;
    
    case UART_MODE_POLLING:
        ret = uart_poll_send(&uart[idx], buf, buf_size);
        break;

    default:
        ret = -1;
        break;
    }
    sem_post(uart[idx].sem);
    return ret;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    int idx = 0;

    for (idx = 0; idx < UART_IDX_MAX; idx++)
    {
        if (uart[idx].handle == huart)
        {
            switch (uart[idx].rx_mode)
            {
            case UART_MODE_IT:
                uart_it_rx_init(&uart[idx]);
                break;
            
            case UART_MODE_DMA:
            case UART_MODE_POLLING:
            default:
                break;
            }
            break;
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    int idx = 0;

    for (idx = 0; idx < UART_IDX_MAX; idx++)
    {
        if (uart[idx].handle == huart)
        {
            uart[idx].tx.sending = false;
            break;
        }
    }
}

/* thread safe printf */
void prints(char *fmt, ...)
{
    char pt_buf[UART_TRX_BUF_SIZE] = {0, };
    va_list va = {0, };

    va_start(va, fmt);
    vsnprintf(pt_buf, sizeof(pt_buf), fmt, va);
    va_end(va);

    while (uart[UART1_IDX].tx.sending);
    uart_send(UART1_IDX, (uint8_t *)pt_buf, strlen(pt_buf));
}

/* thread unsafe printf */
void printu(char *fmt, ...)
{
    char pt_buf[UART_TRX_BUF_SIZE] = {0, };
    va_list va = {0, };

    va_start(va, fmt);
    vsnprintf(pt_buf, sizeof(pt_buf), fmt, va);
    va_end(va);

    HAL_UART_Transmit(uart[UART1_IDX].handle, (uint8_t *)pt_buf, strlen(pt_buf), 100);
}
/* USER CODE END 1 */
