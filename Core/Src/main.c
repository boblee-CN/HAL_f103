/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include "ws2812Frame.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t rxData[18]; //发送18位数据包含2个0x的crc校验码
size_t  rxSize = sizeof(rxData);
uint8_t txData[16];//只要前16位
size_t  txSize = sizeof(txData); // Subtract 1 to exclude null terminator
const char *model1 = "颜色控制";  //  Chinese
const char *model2 = "呼吸灯 ";  //  Chinese
const char *model3 = "流水灯 ";  //  Chinese
const char *model4 = "跑马灯 ";  //  Chinese

uint16_t crc_temple;
uint8_t crc[10];
uint8_t *p_to_rxData = &rxData[1];

//uint8_t color[24];
//uint8_t color1[24];

uint32_t sum;
uint8_t reset[24];
uint8_t one[2] = {0xFF,0xFF};

uint8_t breath_whileflag = 0;
uint8_t puma_whileflag = 0;
uint8_t stream_whileflag = 0;
//uint16_t lenth;
//uint16_t number;
//WS2812_INIT(my_ws2812,1,color,WS2812SendMassge);
WS2812_INIT(my2_ws2812,1,reset,WS2812SendMassge);
//uint8_t countflag=0;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void InvertUint8(unsigned char* DesBuf, unsigned char* SrcBuf);
void InvertUint16(unsigned short* DesBuf, unsigned short* SrcBuf);
unsigned short CRC16_CCITT(unsigned char* puchMsg, unsigned int usDataLen);
uint32_t multiplyLastTwoHexPairs(uint8_t *data);
//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
//void WS2811_Set_RGB(uint8_t r,uint8_t g, uint8_t b, uint16_t len);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
		MX_GPIO_Init();
		MX_DMA_Init();
		MX_USART1_UART_Init();
		MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	//UART_SendByte(0x48,HAL_MAX_DELAY);
		//设置rgb为0的数据帧
		SetWSColor(&my2_ws2812, 0, 0, 0, 0);
		
		//HAL_UART_Transmit(&huart1, my2_ws2812.sendBuff, 24, 0xFFFF);
		
		//灯带reset
		HAL_SPI_Transmit_DMA(&hspi1, reset, 24);
		HAL_Delay(1);
		HAL_SPI_Transmit_DMA(&hspi1, my2_ws2812.sendBuff, 24);
		while(!(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_READY));
		
		HAL_UART_Receive_IT(&huart1, rxData, rxSize);
	 /* USER CODE END 2 */
		
  while (1)
  {
//#if 0
		//延时1段时间，防止新的rxData数据没有接受
		//HAL_Delay(100);
		if(rxData[0] == 0x48)
    {
			  
				//crc校验
				memcpy(txData, rxData, txSize);
				crc_temple = CRC16_CCITT(txData,txSize);
				//电脑串口发送数据是高位先行
				crc[0] = (crc_temple >>8 );   //高8位
				crc[1] = (crc_temple & 0xFF);	//低8位
				//ascii码发送16进制数
				//UART_SendByte(rxData[16],HAL_MAX_DELAY);
				//UART_SendByte(rxData[17],HAL_MAX_DELAY);
				//UART_SendByte(crc[0],HAL_MAX_DELAY);
				//UART_SendByte(crc[1],HAL_MAX_DELAY);
				UART_SendString(&huart1, "Model_Set:");
				switch(rxData[4])
				{
					case 0:UART_SendUTF8String(&huart1, model1);break;
					case 1:UART_SendUTF8String(&huart1, model2);break;
					case 2:UART_SendUTF8String(&huart1, model3);break;
					case 3:UART_SendUTF8String(&huart1, model4);break;
				}
				UART_SendString(&huart1, "\t");
				UART_SendString(&huart1, "CRC_RESULT:");
				if(crc[0] == rxData[16] && crc[1] == rxData[17])
				{
					UART_SendString(&huart1, "TRUE");
				}
				else
				{
					UART_SendString(&huart1, "FALSE");
				}
				//UART_SendHexArray(&huart1, crc, 2);
				rxData[0] = 0x00; 
				
			
			//新数据前关灯,将数据位拉低
				if(sum != 0)
				{
					//当灯数量到768时，for循环会卡
					for(uint8_t i=0;i<(sum/3);i++)
						{
						HAL_SPI_Transmit(&hspi1, my2_ws2812.sendBuff, 24,HAL_MAX_DELAY);
						}
					//HAL_SPI_Transmit(&hspi1, zero, 24,HAL_MAX_DELAY);
				}
						//HAL_Delay(100);
				
			switch(rxData[4])
			{	
				case 0:
						//模式1 颜色控制
						if(rxData[4] == 0x00)
						{
							sum = multiplyLastTwoHexPairs(rxData);
							//发送控灯数量
							UART_SendString(&huart1, "\t");
							UART_SendString(&huart1, "LIGHT_Number:");
							UART_SendIntString(&huart1,sum);
							if(sum != 0)
							{ 
								uint8_t color[(sum/3)*24];
								WS2812_INIT(my_ws2812,1,color,WS2812SendMassge);
								for(uint32_t i=0;i<(sum/3);i++)
								{
									SetWSColor(&my_ws2812, i, rxData[9],  rxData[10],  rxData[11]);  // RGB
								//SetWSColor(&my_ws2812, 1, 255, 0, 0);  // 将第二个灯珠设为绿色
								}
								// Send "ok" after receiving the complete packet
								HAL_SPI_Transmit(&hspi1, my_ws2812.sendBuff, sum/3*24,HAL_MAX_DELAY);
							}
						}
						break;
						
				case 1:
						//模式2 呼吸灯
						if(rxData[4] == 0x01)
						{
							breath_whileflag=0;
							sum = multiplyLastTwoHexPairs(rxData);
							//发送控灯数量
							UART_SendString(&huart1, "\t");
							UART_SendString(&huart1, "LIGHT_Number:");
							UART_SendIntString(&huart1,sum);
							if(sum != 0)
							{ 
								

								
								uint8_t color[(sum/3)*24];
								WS2812_INIT(my_ws2812,1,color,WS2812SendMassge);
								while(1)
								{		
												if (breath_whileflag)
												{
														// 清除标志
																breath_whileflag = 0;

														// 退出循环或执行其他操作
														break;
												}
									
											//pwm 上升
											for(uint8_t j=10;j>0;j--)
												{
														for(uint32_t i=0;i<(sum/3);i++)
														{
															SetWSColor(&my_ws2812, i, rxData[9]/j,  rxData[10]/j,  rxData[11]/j);  // RGB
														}
														
														HAL_SPI_Transmit(&hspi1, my_ws2812.sendBuff, sum/3*24,HAL_MAX_DELAY);
																		
														HAL_Delay(100);
														
														if (breath_whileflag)
														{
																break;
														}
												}
												
											if (breath_whileflag)
												{
														// 清除标志
																breath_whileflag = 0;

														// 退出循环或执行其他操作
														break;
												}
			
											//pwm下降
											for(uint8_t j=1;j<11;j++)
											{
													for(uint32_t i=0;i<(sum/3);i++)
													{
														SetWSColor(&my_ws2812, i, rxData[9]/j,  rxData[10]/j,  rxData[11]/j);  // RGB
													}
													
													HAL_SPI_Transmit(&hspi1, my_ws2812.sendBuff, sum/3*24,HAL_MAX_DELAY);
													
												
				
													HAL_Delay(100);
													if (breath_whileflag)
														{
																break;
														}
											}
											
								}
						 }
							
						}
						break;
						
				case 2:				
						//模式3 流水灯
						if(rxData[4] == 0x02)
						{
							stream_whileflag=0;
							sum = multiplyLastTwoHexPairs(rxData);
							//发送控灯数量
							UART_SendString(&huart1, "\t");
							UART_SendString(&huart1, "LIGHT_Number:");
							UART_SendIntString(&huart1,sum);
							if(sum != 0)
							{ 
								uint8_t color[(sum/3)*24];
								WS2812_INIT(my_ws2812,1,color,WS2812SendMassge);
								while(1)
								{
									if (stream_whileflag)
												{
														// 清除标志
																stream_whileflag = 0;

														// 退出循环或执行其他操作
														break;
												}
									
									for(uint32_t i=0;i<(sum/3);i++)
									{
											SetWSColor(&my_ws2812, i, rxData[9],  rxData[10],  rxData[11]);  // RGB
											HAL_SPI_Transmit(&hspi1, my_ws2812.sendBuff, (i+1)*24,HAL_MAX_DELAY);
											HAL_Delay(200);
										
											if (stream_whileflag)
													{								
															break;
													}
									}
									
									if (stream_whileflag)
												{
														// 清除标志
																stream_whileflag = 0;

														// 退出循环或执行其他操作
														break;
												}
									for(int32_t j=(sum/3)-1;j>=0;j--)
									{
											SetWSColor(&my_ws2812, j, 0,  0,  0);  // RGB
											HAL_SPI_Transmit(&hspi1, my_ws2812.sendBuff, (sum/3)*24,HAL_MAX_DELAY);
											HAL_Delay(200);
											if (stream_whileflag)
													{								
															break;
													}
									}					
								}
								
							}
								
								
						}
						break;
						
				case 3:		
						//模式4 跑马灯
						if(rxData[4] == 0x03)
						{
							puma_whileflag=0;
							sum = multiplyLastTwoHexPairs(rxData);
							//发送控灯数量
							UART_SendString(&huart1, "\t");
							UART_SendString(&huart1, "LIGHT_Number:");
							UART_SendIntString(&huart1,sum);
							if(sum != 0)
							{ 
								uint8_t color[(sum/3)*24];
								WS2812_INIT(my_ws2812,1,color,WS2812SendMassge);
								while(1)
								{
									if (puma_whileflag)
												{
														// 清除标志
																puma_whileflag = 0;

														// 退出循环或执行其他操作
														break;
												}
									
									for(uint32_t i=0;i<(sum/3);i++)
									{
											//先把之前设置的灯带reset
											for(uint32_t j=0;j<(sum/3);j++)
											{
													SetWSColor(&my_ws2812, j, 0,  0,  0);  // RGB
											}
											
											SetWSColor(&my_ws2812, i, rxData[9],  rxData[10],  rxData[11]);  // RGB
											HAL_SPI_Transmit(&hspi1, my_ws2812.sendBuff, (sum/3)*24,HAL_MAX_DELAY);
											HAL_Delay(200);
											if (puma_whileflag)
												{								
														break;
												}
									}
									
									
								}
								
							}		
						}
						break;		
					}
				
				}
		}
}
	
    /* USER CODE BEGIN 3 */
 
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 处理接收到的数据
        // 可以在这里启动新的接收中断
				puma_whileflag = 1;
				breath_whileflag = 1;
				stream_whileflag = 1;
			
        HAL_UART_Receive_IT(&huart1, rxData, rxSize);
    }
}


uint32_t multiplyLastTwoHexPairs(uint8_t *data)
{
    // 获取最后4个字节，将它们组合成两个 16 位整数
    uint16_t num1 = (data[12] << 8) | data[13]; // 组合高低位，0xFF01
    uint16_t num2 = (data[14] << 8) | data[15]; // 组合高低位，0x3221

    // 计算乘积
    uint32_t product = (uint32_t)num1 * (uint32_t)num2;

    return product;
}


void InvertUint8(unsigned char* DesBuf, unsigned char* SrcBuf)
{
    int i;
    unsigned char temp = 0;

    for (i = 0; i < 8; i++)
    {
        if (SrcBuf[0] & (1 << i))
        {
            temp |= 1 << (7 - i);
        }
    }
    DesBuf[0] = temp;
}

void InvertUint16(unsigned short* DesBuf, unsigned short* SrcBuf)
{
    int i;
    unsigned short temp = 0;

    for (i = 0; i < 16; i++)
    {
        if (SrcBuf[0] & (1 << i))
        {
            temp |= 1 << (15 - i);
        }
    }
    DesBuf[0] = temp;
}

unsigned short CRC16_CCITT(unsigned char* puchMsg, unsigned int usDataLen)
{
    unsigned short wCRCin = 0x0000;
    unsigned short wCPoly = 0x1021;
    unsigned char wChar = 0;

    while (usDataLen--)
    {
        wChar = *(puchMsg++);
        InvertUint8(&wChar, &wChar);
        wCRCin ^= (wChar << 8);

        for (int i = 0; i < 8; i++)
        {
            if (wCRCin & 0x8000)
            {
							wCRCin = (wCRCin << 1) ^ wCPoly;//CRC的第一位舍去
            }
            else
            {
                wCRCin = wCRCin << 1;
            }
        }
    }
    InvertUint16(&wCRCin, &wCRCin);
    return (wCRCin);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
