/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file (STM32F769I-DISCOVERY + uT-Kernel 3.0 BSP2)
  *                   Laid out like STM32CubeMX output so the project can be
  *                   regenerated from an .ioc without losing USER CODE.
  *
  * Copyright (c) 2016 STMicroelectronics.
  * Copyright (c) 2026 Madhawa Polkotuwa (uT-Kernel BSP2 integration).
  * SPDX-License-Identifier: BSD-3-Clause
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"

void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
/* Board pin assignment, STM32F769I-DISCOVERY (MB1225).
 * Source: STM32CubeF7 V1.17.4 BSP Drivers/BSP/STM32F769I-Discovery
 *         (stm32f769i_discovery.h) and mbedTLS example main.h (STLINK_USARTx).
 * To be confirmed against UM2033 (see PORTING notes).                       */
#define LD1_RED_Pin            GPIO_PIN_13   /* LD1 red   : PJ13, active high */
#define LD1_RED_GPIO_Port      GPIOJ
#define LD2_GREEN_Pin          GPIO_PIN_5    /* LD2 green : PJ5,  active high */
#define LD2_GREEN_GPIO_Port    GPIOJ
#define B_USER_Pin             GPIO_PIN_0    /* B1 USER (blue) : PA0, high = pressed */
#define B_USER_GPIO_Port       GPIOA
#define STLINK_TX_Pin          GPIO_PIN_9    /* USART1_TX -> ST-LINK VCP : PA9,  AF7 */
#define STLINK_TX_GPIO_Port    GPIOA
#define STLINK_RX_Pin          GPIO_PIN_10   /* USART1_RX <- ST-LINK VCP : PA10, AF7 */
#define STLINK_RX_GPIO_Port    GPIOA

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
