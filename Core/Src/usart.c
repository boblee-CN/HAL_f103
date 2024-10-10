/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
/* USER CODE END 0 */

UART_HandleTypeDef huart1;

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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
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

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void UART_SendByte(uint8_t byte,uint32_t Timeout)
{
    HAL_UART_Transmit(&huart1, &byte, 1, HAL_MAX_DELAY);
}

// 将 uint8_t 数组中的 16 进制数转换为 ASCII 字符串并发送
void UART_SendHexArray(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size)
{
    char hexString[3]; // 用于存储一个字节的两个 ASCII 字符和一个终止符
    //hexString[2] = '\0'; // 字符串终止符

    for (uint16_t i = 0; i < size; i++)
    {
        // 将crc数组里每个字节转换为两个 ASCII 字符
        snprintf(hexString, sizeof(hexString), "%02X", data[i]);

        // 发送字符串
        UART_Sendhex(huart, hexString);
    }
}

void UART_Sendhex(UART_HandleTypeDef *huart, const char *str)
{
    HAL_UART_Transmit(huart, (uint8_t *)str, 2, HAL_MAX_DELAY);
}

// 函数定义
void UART_SendString(UART_HandleTypeDef *huart, const char *str)
{
    while (*str)
    {
        HAL_UART_Transmit(huart, (uint8_t *)str, 1, HAL_MAX_DELAY);
        str++;
    }
}

void UART_SendUTF8String(UART_HandleTypeDef *huart, const char *str)
{		
    HAL_UART_Transmit(huart, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

void UART_SendIntString(UART_HandleTypeDef *huart, int value)
{
    char buffer[50];
    int length;

    // 将整数转换为字符串
    length = snprintf(buffer, sizeof(buffer), "%d\r\n", value);

    // 发送字符串到 UART
    HAL_UART_Transmit(huart, (uint8_t*)buffer, length, HAL_MAX_DELAY);
}
/* USER CODE END 1 */
