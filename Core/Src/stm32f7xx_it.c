/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @brief   Interrupt Service Routines (STM32CubeMX layout).
  *
  * uT-Kernel note:
  *  - PendSV (vector 14) and SysTick (vector 15) are replaced by the kernel
  *    (knl_dispatch_entry / knl_systim_inthdr) in its RAM copy of the vector
  *    table, see mtk3_bsp2/sysdepend/stm32_cube/cpu/core/armv7m/interrupt.c.
  *    They are therefore NOT defined here.
  *  - SVC is not used by uT-Kernel 3.0 on ARMv7-M (service calls are plain
  *    function calls).
  *  - TIM6_DAC_IRQHandler (HAL time base) is in stm32f7xx_hal_timebase_tim.c.
  *    It is not registered with tk_def_int() and must never call tk_* APIs.
  *
  * Copyright (c) 2016 STMicroelectronics.
  * Copyright (c) 2026 Madhawa Polkotuwa (uT-Kernel BSP2 integration).
  * SPDX-License-Identifier: BSD-3-Clause
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "stm32f7xx_it.h"

void NMI_Handler(void)
{
  while (1) { }
}

void HardFault_Handler(void)
{
  while (1) { }
}

void MemManage_Handler(void)
{
  while (1) { }
}

void BusFault_Handler(void)
{
  while (1) { }
}

void UsageFault_Handler(void)
{
  while (1) { }
}

void DebugMon_Handler(void)
{
}
