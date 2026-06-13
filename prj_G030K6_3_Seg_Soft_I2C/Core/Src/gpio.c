/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
/*
	备注：HC595的8位输出引脚从高到低g~a对应数码管的从高到低g~a a为最低位，在推出数据的最后
		定义共阴极数码管数组，0~9，外加一个小数点
*/
uint8_t sgh_value[11] = {
    0b00111111,   // 0		0x3F
    0b00000110,   // 1      0x06
    0b01011011,   // 2      0x5B
    0b01001111,   // 3      0x4F
    0b01100110,   // 4      0x66
    0b01101101,   // 5      0x6D
    0b01111101,   // 6      0x7D
    0b00000111,   // 7      0x07
    0b01111111,   // 8      0x7F
    0b01101111,   // 9      0x6F
    0b10000000    // 小数点	0x80
};

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SHT40_SDA_Pin|LED1_SER_Pin|GPIO_TEST_Pin|SHT40_SCL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED2_SER_Pin|LED2_RCLK_Pin|LED2_SCLK_Pin|DIG_SER_Pin
                          |DIG_RCLK_Pin|DIG_SCLK_Pin|LED1_RCLK_Pin|LED1_SCLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SHT40_SDA_Pin SHT40_SCL_Pin */
  GPIO_InitStruct.Pin = SHT40_SDA_Pin|SHT40_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LED2_SER_Pin LED2_RCLK_Pin LED2_SCLK_Pin DIG_SER_Pin
                           DIG_RCLK_Pin DIG_SCLK_Pin LED1_RCLK_Pin LED1_SCLK_Pin */
  GPIO_InitStruct.Pin = LED2_SER_Pin|LED2_RCLK_Pin|LED2_SCLK_Pin|DIG_SER_Pin
                          |DIG_RCLK_Pin|DIG_SCLK_Pin|LED1_RCLK_Pin|LED1_SCLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_SER_Pin GPIO_TEST_Pin */
  GPIO_InitStruct.Pin = LED1_SER_Pin|GPIO_TEST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : WAKE_Pin */
  GPIO_InitStruct.Pin = WAKE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(WAKE_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 2 */
void Open_Led(void)
{
    //输出低电平
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
}
/*
	函数内容：关闭LED
	函数参数：无
	返回值：无
*/
void Close_Led(void)
{
    //输出高电平
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
}
/*
	函数内容：翻转LED
	函数参数：无
	返回值：无
*/
void Toggle_LED(void)
{
    //翻转电平
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
}

/*
	函数内容：扫描按键
	函数参数：无
	返回值：如果按键按下，返回TRUE，否则返回FALSE
*/
uint8_t Scanf_Key(void)
{
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5) == GPIO_PIN_RESET)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5) == GPIO_PIN_RESET)
		{
			//一直按住卡在while循环中，循环退出，返回TRUE
			while(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5) == GPIO_PIN_RESET);
			return TRUE;
		}
	}
	return FALSE;
}

/* 64MHz时钟时，每轮循环耗时约10个时钟，延时≈10* ulCount /64 μs */
void SysCtlDelay(unsigned long ulCount)
{
    volatile unsigned long delay = ulCount;  // volatile防止编译器优化
    while (delay--) {
        __NOP();  // 空操作指令，消耗一个时钟周期（加上volatile读改写+分支跳转，约10周期/轮）
        __NOP();  // 空操作指令，消耗一个时钟周期
    }
}


/*
	函数内容：SN74HC595发送数据函数
	函数参数：unsigned char sn_num-哪一个595器件
						unsigned char sendValue--数据值
	返回值：	无
	备注：SN74HC595_Send_Data(SN_DIG,0b00111110);		0b00111110表示DIG1亮
		当sn_num为SN_DIG时，sendValue的8位数据分别对应数码管的DIG1~DIG6，最低位为DIG1
	流程：SN74HC595_Send_Data(SN_DIG,0b00111110);
		SN74HC595_Send_Data(SN_LED1,0b10000000);
		SysCtlDelay(350);
		  SN74HC595_Send_Data(SN_LED1,0x00);	//消影，防止错位
*/
void SN74HC595_Send_Data(unsigned char sn_num,unsigned int sendValue)
{
	uint8_t i = 0;
	if(sn_num == SN_LED1)	//第一个数码管
	{
		for(i = 0;i < 8;i++)
		{
			if(((sendValue << i)&0x80)!=0)	//如果数据第一位是1
			{
				HAL_GPIO_WritePin(LED1_SER_GPIO_Port,LED1_SER_Pin,GPIO_PIN_SET);		//高电平
			}
			else
			{
				HAL_GPIO_WritePin(LED1_SER_GPIO_Port,LED1_SER_Pin,GPIO_PIN_RESET);	//低电平
			}
			HAL_GPIO_WritePin(LED1_SCLK_GPIO_Port,LED1_SCLK_Pin,GPIO_PIN_RESET);	//产生一个SCLK上升沿
			HAL_GPIO_WritePin(LED1_SCLK_GPIO_Port,LED1_SCLK_Pin,GPIO_PIN_SET);
		}

		HAL_GPIO_WritePin(LED1_RCLK_GPIO_Port,LED1_RCLK_Pin,GPIO_PIN_RESET);	//产生一个RCLK上升沿
		HAL_GPIO_WritePin(LED1_RCLK_GPIO_Port,LED1_RCLK_Pin,GPIO_PIN_SET);
	}
    else if(sn_num == SN_LED2)
	{
		for(i = 0;i < 8;i++)
		{
			if(((sendValue << i)&0x80)!=0)	//如果数据第一位是1
			{
				HAL_GPIO_WritePin(LED2_SER_GPIO_Port,LED2_SER_Pin,GPIO_PIN_SET);		//高电平
			}
			else
			{
				HAL_GPIO_WritePin(LED2_SER_GPIO_Port,LED2_SER_Pin,GPIO_PIN_RESET);	//低电平
			}
			HAL_GPIO_WritePin(LED2_SCLK_GPIO_Port,LED2_SCLK_Pin,GPIO_PIN_RESET);	//产生一个SCLK上升沿
			HAL_GPIO_WritePin(LED2_SCLK_GPIO_Port,LED2_SCLK_Pin,GPIO_PIN_SET);
		}

		HAL_GPIO_WritePin(LED2_RCLK_GPIO_Port,LED2_RCLK_Pin,GPIO_PIN_RESET);	//产生一个RCLK上升沿
		HAL_GPIO_WritePin(LED2_RCLK_GPIO_Port,LED2_RCLK_Pin,GPIO_PIN_SET);
	}
    else if(sn_num == SN_DIG)
	{
        //选择数码管显示从DIG1~DIG6 ，DIG1在最左边，DIG6在最右边
        //与SN_LED1和SN_LED2共同配和使用，SN_LED1和SN_LED2控制数码管的段选和内容，DIG1~DIG6控制数码管的位选
		for(i = 0;i < 8;i++)
		{
			if(((sendValue << i)&0x80)!=0)
			{
				HAL_GPIO_WritePin(DIG_SER_GPIO_Port,DIG_SER_Pin,GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(DIG_SER_GPIO_Port,DIG_SER_Pin,GPIO_PIN_RESET);
			}
			HAL_GPIO_WritePin(DIG_SCLK_GPIO_Port,DIG_SCLK_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DIG_SCLK_GPIO_Port,DIG_SCLK_Pin,GPIO_PIN_SET);
		}

		HAL_GPIO_WritePin(DIG_RCLK_GPIO_Port,DIG_RCLK_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIG_RCLK_GPIO_Port,DIG_RCLK_Pin,GPIO_PIN_SET);	
	}
}

/*
	函数内容：数码管显示函数
	函数参数：uint8_t row-第几排数码管，1或2
			uint8_t column-第几位数码管，1~6
			uint8_t value-要显示的值，0~9，小数点为10
	使用示例：ShowNum(1, 2, 1);
			ShowNum(1, 3, 1);
*/
void ShowNum(uint8_t row, uint8_t column, uint8_t value)
{
	if(row == 1)
	{
		switch(column)
		{
			case 1:	//如果是第一排第一个
				SN74HC595_Send_Data(SN_DIG,0b11111110);		//DIG1亮
				SN74HC595_Send_Data(SN_LED1,sgh_value[value]);//显示值对应16进制数
				break;
			case 2:
				SN74HC595_Send_Data(SN_DIG,0b11111101);		//DIG2亮
				SN74HC595_Send_Data(SN_LED1,(sgh_value[value]|0x80));
				break;
			case 3:
				SN74HC595_Send_Data(SN_DIG,0b11111011);		//DIG3亮
				SN74HC595_Send_Data(SN_LED1,sgh_value[value]);
				break;
			default:
				break;
		}
		SysCtlDelay(350);	//大致延时50us
		SN74HC595_Send_Data(SN_LED1,0x00);	//消影，防止错位
	}
	else
	{
		switch(column)
		{
			case 1:
				SN74HC595_Send_Data(SN_DIG,0b11110111);		//DIG4亮
				SN74HC595_Send_Data(SN_LED2,sgh_value[value]);
				break;
			case 2:
				SN74HC595_Send_Data(SN_DIG,0b11101111);		//DIG5亮
				SN74HC595_Send_Data(SN_LED2,(sgh_value[value]|0x80));
				break;
			case 3:
				SN74HC595_Send_Data(SN_DIG,0b11011111);		//DIG6亮
				SN74HC595_Send_Data(SN_LED2,sgh_value[value]);
				break;
			default:
				break;
		}
		SysCtlDelay(350);	//大致延时50us
		SN74HC595_Send_Data(SN_LED2,0x00);	//消影，防止错位
	}
}


/*
	函数内容：GPIO 下降沿中断服务函数
	函数参数：uint16_t GPIO_Pin--哪一个GPIO
	返回值：	无
*/
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	//后续可以在这里设置标志位 之后在外部去做相关松手判断操作
	if(GPIO_Pin == WAKE_Pin)
	{
		device_paramter.KeyStatus = KEY_SHAKE_STATE;	//设置按键状态为消抖状态
		HAL_ResumeTick();								//恢复滴答定时器

		if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5) == GPIO_PIN_RESET)
		{
			//执行翻转LED函数，在中断服务函数中不建议做死循环或长时间等待操作
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
		}
	}

}

/* USER CODE END 2 */
