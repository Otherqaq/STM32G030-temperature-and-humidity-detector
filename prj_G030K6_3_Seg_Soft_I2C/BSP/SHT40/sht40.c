/*
 * sht40.c
 *
 *  Created on: 2026年5月20日
 *      Author: COLORFUL
 */

#include "sht40.h"


// 起始信号：SCL高电平时，SDA从高到低跳变
void I2C_Start(void)
{
    SDA_OUT();
    SDA_H();
    SCL_H();
    I2C_DELAY();
    SDA_L(); 
    I2C_DELAY();
    SCL_L();
}


// 停止信号：SCL高电平时，SDA从低到高跳变
void I2C_Stop(void)
{
    SDA_OUT();
    SCL_L();
    SDA_L();
    I2C_DELAY();
    SCL_H();
    I2C_DELAY();
    SDA_H();
    I2C_DELAY();
}

// 发送一个字节，返回ACK/NACK
uint8_t I2C_SendByte(uint8_t dat)
{
    uint8_t i;
    
    SDA_OUT();
    SCL_L();
    
    for(i = 0; i < 8; i++) {
        if(dat & 0x80) {
            SDA_H();
        } else {
            SDA_L();
        }
        I2C_DELAY();
        SCL_H();
        I2C_DELAY();
        SCL_L();
        dat <<= 1;
    }
    
    return I2C_WaitAck();
}
// 接收一个字节，ack=1发送ACK，ack=0发送NACK
uint8_t I2C_ReceiveByte(uint8_t ack)
{
    uint8_t i, dat = 0;
    
    SDA_H();        // 确保释放SDA线，由从机控制
    I2C_DELAY();
    
    for(i = 0; i < 8; i++) {
        SCL_L();
        I2C_DELAY();
        SCL_H();
        I2C_DELAY();
        dat <<= 1;
        if(SDA_READ()) {
            dat |= 0x01;
        }
    }
    
    SCL_L();
    I2C_DELAY();
    
    if(ack) {
        SDA_L();    // 发送ACK
    } else {
        SDA_H();    // 发送NACK
    }
    I2C_DELAY();
    SCL_H();
    I2C_DELAY();
    SCL_L();
    I2C_DELAY();
    SDA_H();        // 结束后释放SDA线
    
    return dat;
}

// 等待ACK
uint8_t I2C_WaitAck(void)
{
    uint8_t ack;
    
    SDA_H();        // 释放SDA线
    I2C_DELAY();
    SCL_H();        // 产生时钟高电平
    I2C_DELAY();
    
    ack = SDA_READ(); // 读取ACK信号
    
    SCL_L();
    I2C_DELAY();
    
    return ack;
}


// SHT40读取温湿度
uint8_t SHT40_Read_RHData(float *temperature, float *humidity)
{
    uint8_t cmd = 0xFD;
    uint8_t data[6] = {0};

    I2C_Start();                    // 起始信号
    if(I2C_SendByte(0x44 << 1)) {   // 发送写地址并检查ACK
        I2C_Stop();
        return 1;                   // NACK
    }
    if(I2C_SendByte(cmd)) {         // 发送测量命令并检查ACK
        I2C_Stop();
        return 2;                   // NACK
    }
    I2C_Stop();                     // 停止信号

    HAL_Delay(10);                  // 等待测量完成

    I2C_Start();                    // 起始信号
    if(I2C_SendByte((0x44 << 1) | 0x01)) { // 发送读地址并检查ACK
        I2C_Stop();
        return 3;                   // NACK
    }

    // 读取6字节数据
    for(int i = 0; i < 5; i++) {
        data[i] = I2C_ReceiveByte(1); // 发送ACK
    }
    data[5] = I2C_ReceiveByte(0);     // 最后一个字节发送NACK

    I2C_Stop();                     // 停止信号

    // 数据转换
    *temperature = (175.0f * (data[0] * 256 + data[1])) / 65535.0f - 45.0f;
    *humidity = (125.0f * (data[3] * 256 + data[4])) / 65535.0f - 6.0f;
    
    return 0; // 成功
}
