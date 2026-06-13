/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sht40.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
struct DEVICE_PARAMTER device_paramter = {0};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	float Temperature = 0;
	float Humidity = 0;

	uint32_t ADC_Value = 0;
	float Data = 0;
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
  MX_ADC1_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */
  //操作595使数码管初始全灭
  SN74HC595_Send_Data(SN_DIG,0x00);		//共阴极低电平
  SN74HC595_Send_Data(SN_LED1,0x00);	//内容空
  SN74HC595_Send_Data(SN_LED2,0x00);

  HAL_ADCEx_Calibration_Start(&hadc1);    //ADC校准

  HAL_ADC_Start(&hadc1);     //启动ADC转换
  HAL_ADC_PollForConversion(&hadc1, 50);   //等待转换完成，50为最大等待时间，单位为ms
  if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
  {
	  ADC_Value = HAL_ADC_GetValue(&hadc1);   //获取AD值
	  Data = (ADC_Value*3.3f)/4095.0f;
  }
  HAL_ADC_Stop(&hadc1);
  device_paramter.Vol_Value = (uint16_t)(Data * 100)*2;

  HAL_SuspendTick();	//暂停滴答定时器，防止通过滴答定时器中断唤醒
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); /* 执行WFI指令, 进入睡眠模式 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  HAL_Delay(50);
  while (1)
  {
    /* ===== 超时检查：等第二次点击超时 → 判为单击 ===== */
    if(device_paramter.clickPending &&
        (HAL_GetTick() - device_paramter.firstClickTick > DOUBLE_CLICK_WINDOW))
    {
        device_paramter.clickPending = 0;
        // 没有第二次点击 → 执行单击：读温湿度
        SHT40_Read_RHData(&Temperature, &Humidity);	//读取温湿度
				device_paramter.Temp = Temperature * 10;		//放大温湿度
				device_paramter.Humi = Humidity * 10;
				device_paramter.displayMode = 0;
				HAL_TIM_Base_Start_IT(&htim14);         		//开始定时器，显示两次数据 

        device_paramter.sleepStatus = 0;            //清除休眠标志
    }

    /* ===== 按键事件处理 ===== */
    if(device_paramter.KeyStatus == KEY_SHAKE_STATE)
    {
        HAL_Delay(10);
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET);
            device_paramter.KeyStatus = KEY_NO_PRESS;

            if (device_paramter.clickPending)
            {
                /* 第二次点击在窗口内 → 双击 */
                device_paramter.clickPending = 0;

                HAL_ADC_Stop(&hadc1);                   //确保从停止态启动
                HAL_ADC_Start(&hadc1);
                HAL_ADC_PollForConversion(&hadc1, 50);  //等待转换完成，50为最大等待时间，单位为ms
                ADC_Value = HAL_ADC_GetValue(&hadc1);   //获取AD值
                /* 先乘后除保证精度 Data范围0-3.3V 但实际上有两10K电阻串联故约为1.65 */
                Data = (ADC_Value * 3.3f)/4095.0f;
                device_paramter.Vol_Value = (uint16_t)(Data * 100)*2; // 还原上端电压，厘伏(×100)，1.65V→330

                // 数码管显示电压，tim.c 中根据 displayMode 读取 Vol_Value
                device_paramter.displayMode = 1;

                HAL_TIM_Base_Start_IT(&htim14);
                device_paramter.sleepStatus = 0;
            }
            else{
                /* 第一次点击 → 启动窗口计时 */
                device_paramter.sleepStatus = 0;          // 清除休眠标志，防止立即重入SLEEP
                device_paramter.clickPending = 1;
                device_paramter.firstClickTick = HAL_GetTick();
                // 不 sleep，继续轮询等第二次点击
            }
        }
        else{
            device_paramter.KeyStatus = KEY_NO_PRESS;  // 抖动，忽略
        }
    }
    /* ===== 休眠入口 ===== */
    if (device_paramter.sleepStatus == 1)           //显示结束，进入休眠
    {
        HAL_SuspendTick();   //暂停滴答定时器，防止通过滴答定时器中断唤醒
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); /* 执行WFI指令, 进入睡眠模式 */
    }


    // ShowNum(1,1,device_paramter.Vol_Value / 100);
    // ShowNum(1,2,device_paramter.Vol_Value /10 %10);
    // ShowNum(1,3,device_paramter.Vol_Value % 10);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // ShowNum(1,1,Temp/100);
    // ShowNum(1,2,Temp/ 10 % 10);
    // ShowNum(1,3,Temp%10);

		// ShowNum(2,1,Humi/100);
		// ShowNum(2,2,Humi/ 10 % 10);
		// ShowNum(2,3,Humi%10);
		// HAL_Delay(10);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
