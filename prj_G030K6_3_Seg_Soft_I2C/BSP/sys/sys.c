/*
 * sys.c
 *
 *  Created on: 2026年5月20日
 *      Author: COLORFUL
 */
#include "sys.h"

void delay_us(uint32_t _us)
{
    uint32_t ticks;           // 需要的总时钟周期数
    uint32_t told, tnow, tcnt = 0;

    // 计算需要的时钟数 = 延迟微秒数 × 每微秒的时钟数
    ticks = _us * (SystemCoreClock / 1000000);

    // 获取当前的SysTick值（递减计数器）
    told = SysTick->VAL;

    while (1)
    {
        // 重复刷新获取当前的SysTick值
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            // 处理SysTick计数器的回绕
            if (tnow < told)
                tcnt += told - tnow;      // 正常递减
            else
                tcnt += SysTick->LOAD - tnow + told;  // 发生回绕

            told = tnow;

            // 如果达到了需要的时钟数，就退出循环
            if (tcnt >= ticks)
                break;
        }
    }
}
