/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define KEY_NO_PRESS 		0		//没有按键按下
#define KEY_SHAKE_STATE 1		//按键进入消抖状态


#define TRUE 	0x01	// 0x01，十进制1
#define FALSE 	0x02

#define SN_LED1 0x01
#define SN_LED2 0x02
#define SN_DIG  0x03
/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
void Open_Led(void);
void Close_Led(void);
void Toggle_LED(void);

uint8_t Scanf_Key(void);

void SysCtlDelay(unsigned long ulCount);
void SN74HC595_Send_Data(unsigned char sn_num,unsigned int sendValue);
void ShowNum(uint8_t row, uint8_t column, uint8_t value);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

