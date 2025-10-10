/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "app_threadx.h"
#include "status.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */
#define UART_TRX_BUF_SIZE (1 << 8)
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */
typedef enum
{
    UART_MODE_DMA,
    UART_MODE_IT,
    UART_MODE_POLLING,
}UART_MODE;

typedef enum
{
    UART1_IDX,
    UART_IDX_MAX,
}UART_IDX;

typedef struct
{
    uint8_t buf[UART_TRX_BUF_SIZE];
    int front;
    int rear;
}uart_rx_t;

typedef struct
{
    uint8_t buf[UART_TRX_BUF_SIZE];
    bool sending;
}uart_tx_t;

typedef struct 
{
    UART_HandleTypeDef *handle;
    uart_rx_t rx;
    uart_tx_t tx;
    UART_MODE tx_mode;
    UART_MODE rx_mode;
    TX_SEMAPHORE *sem;
}uart_t;

void prints(char *fmt, ...);
void printu(char *fmt, ...);

static inline void print_dmesg(char *fmt, ...)                                                      
{
    char buf[UART_TRX_BUF_SIZE] = {0, };
    va_list va = {0, };
    va_start(va, fmt);                                              
    vsnprintf(buf, sizeof(buf), fmt, va);
    if (status_get_int(STATUS_INTEGER_DMESG))                                      
    {                                                                              
        uint32_t tick = HAL_GetTick();                                             
        prints("[%8d.%03d] %s\r\n", tick / 1000, tick % 1000, buf); 
    }
    va_end(va);                                                             
}

#define pbold(str) "\x1b[1m" str "\x1b[0m"
#define printok(fmt, ...)   printu("[\x1b[32m  OK  \x1b[0m] "fmt"\r\n", ##__VA_ARGS__)
#define printfail(fmt, ...) printu("[\x1b[31m FAIL \x1b[0m] "fmt"\r\n", ##__VA_ARGS__)
#define printdepend(fmt, ...) printu("[\x1b[33;5mDEPEND\x1b[0m] "fmt"\r\n", ##__VA_ARGS__)

#define printr(fmt, ...) prints("[\x1b[31;1mERR, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
#define printg(fmt, ...) prints("[\x1b[32;1mSUCC, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
#define printd(fmt, ...) prints("[\x1b[34;1mDBG, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
#define printy(fmt, ...) prints("[\x1b[32;1mWARN, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)

void uart_init();
int uart_recv(UART_IDX idx, uint8_t *buf, uint16_t buf_size);
int uart_send(UART_IDX idx, uint8_t *buf, uint16_t buf_size);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

