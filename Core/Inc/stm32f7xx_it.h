/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f7xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
  *
  * Copyright (c) 2016 STMicroelectronics.
  * Copyright (c) 2026 Madhawa Polkotuwa (uT-Kernel BSP2 integration).
  * SPDX-License-Identifier: BSD-3-Clause
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __STM32F7xx_IT_H
#define __STM32F7xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_IT_H */
