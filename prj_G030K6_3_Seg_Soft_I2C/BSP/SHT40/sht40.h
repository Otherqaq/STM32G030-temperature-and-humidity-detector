/*
 * sht40.h
 *
 *  Created on: 2026年5月20日
 *      Author: COLORFUL
 */

#ifndef SHT40_SHT40_H_
#define SHT40_SHT40_H_

#include "stm32g0xx_hal.h"
#include "sys.h"
#include "main.h"

// GPIO操作宏定义
#define SCL_H()  HAL_GPIO_WritePin(SHT40_SCL_GPIO_Port, SHT40_SCL_Pin, GPIO_PIN_SET)
#define SCL_L()  HAL_GPIO_WritePin(SHT40_SCL_GPIO_Port, SHT40_SCL_Pin, GPIO_PIN_RESET)
#define SDA_H()  HAL_GPIO_WritePin(SHT40_SDA_GPIO_Port, SHT40_SDA_Pin, GPIO_PIN_SET)
#define SDA_L()  HAL_GPIO_WritePin(SHT40_SDA_GPIO_Port, SHT40_SDA_Pin, GPIO_PIN_RESET)
#define SDA_READ() HAL_GPIO_ReadPin(SHT40_SDA_GPIO_Port, SHT40_SDA_Pin)

// 延时函数（调整I2C速度）
#define I2C_DELAY()  delay_us(5)  // 约100kHz

// 开漏输出模式下，可以通过写1来释放总线进行读取
// OD模式下无需切换方向
#define SDA_OUT()  
#define SDA_IN()   

// I2C 函数声明
void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_SendByte(uint8_t dat);
uint8_t I2C_ReceiveByte(uint8_t ack);
uint8_t I2C_WaitAck(void);

uint8_t SHT40_Read_RHData(float *temperature, float *humidity);

#endif /* SHT40_SHT40_H_ */