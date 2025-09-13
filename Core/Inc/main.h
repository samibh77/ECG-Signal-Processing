/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SDW_RST_Pin GPIO_PIN_13
#define SDW_RST_GPIO_Port GPIOG
#define MCU_ACTIVE_Pin GPIO_PIN_12
#define MCU_ACTIVE_GPIO_Port GPIOG
#define LED3_Pin GPIO_PIN_5
#define LED3_GPIO_Port GPIOD
#define SDW_SCK_Pin GPIO_PIN_3
#define SDW_SCK_GPIO_Port GPIOD
#define LED4_Pin GPIO_PIN_3
#define LED4_GPIO_Port GPIOK
#define LED2_Pin GPIO_PIN_4
#define LED2_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOG
#define TP_IRQ_Pin GPIO_PIN_5
#define TP_IRQ_GPIO_Port GPIOJ
#define TP_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define FRAME_RATE_Pin GPIO_PIN_1
#define FRAME_RATE_GPIO_Port GPIOA
#define USER_BUTTON_Pin GPIO_PIN_0
#define USER_BUTTON_GPIO_Port GPIOA
#define LCD_RST_Pin GPIO_PIN_7
#define LCD_RST_GPIO_Port GPIOH
#define VSYNC_FREQ_Pin GPIO_PIN_2
#define VSYNC_FREQ_GPIO_Port GPIOA
#define RENDER_TIME_Pin GPIO_PIN_6
#define RENDER_TIME_GPIO_Port GPIOA
#define SDW_CS_Pin GPIO_PIN_6
#define SDW_CS_GPIO_Port GPIOH
#define SDW_MISO_Pin GPIO_PIN_14
#define SDW_MISO_GPIO_Port GPIOB
#define SDW_MOSI_Pin GPIO_PIN_15
#define SDW_MOSI_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/**
 * @brief  LCD Display OTM8009A ID
 */
#define LCD_ID ((uint32_t)0)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
