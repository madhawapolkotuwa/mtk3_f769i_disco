/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : STM32F769I-DISCOVERY + uT-Kernel 3.0 BSP2 - hardware init
  *
  * Structure follows STM32CubeMX output. The uT-Kernel BSP2 integration point
  * is the single call to knl_start_mtkernel() in USER CODE 2 (BSP2 manual 4.2.3).
  * Everything before it is ordinary STM32Cube initialisation; the kernel takes
  * over SysTick, PendSV and the vector table from that point on.
  *
  * Copyright (c) 2016 STMicroelectronics.
  * Copyright (c) 2026 Madhawa Polkotuwa (uT-Kernel BSP2 integration).
  * SPDX-License-Identifier: BSD-3-Clause
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* Private function prototypes -----------------------------------------------*/
static void MPU_Config(void);
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
void knl_start_mtkernel(void);		/* uT-Kernel 3.0 BSP2 entry (sys_start.c) */
/* USER CODE END PFP */

int main(void)
{
  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache (as in ST's STM32F769I-Discovery template) */
  SCB_EnableICache();
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the HAL
   * time base (TIM6, see stm32f7xx_hal_timebase_tim.c - NOT SysTick, because
   * SysTick becomes the uT-Kernel system timer). */
  HAL_Init();

  /* Configure the system clock: 216 MHz from 25 MHz HSE */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  {
    /* Pre-kernel checkpoint, sent with the HAL driver (not T-Monitor). */
    static const char msg[] = "\r\n[boot] STM32F769I-DISCO: HAL init done, starting uT-Kernel\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg) - 1, 100);
  }

  knl_start_mtkernel();		/* Start uT-Kernel 3.0 - never returns */
  /* USER CODE END 2 */

  while (1)
  {
  }
}

/**
  * @brief System Clock Configuration
  *   HSE 25 MHz (X2 crystal) -> PLLM /25 = 1 MHz -> PLLN x432 = 432 MHz VCO
  *   SYSCLK = VCO / PLLP(2) = 216 MHz, PLL48 = VCO / PLLQ(9) = 48 MHz
  *   HCLK = 216 MHz, APB1 = /4 = 54 MHz, APB2 = /2 = 108 MHz
  *   Over-drive ON, VOS scale 1, 7 Flash wait states.
  *   Values from STM32CubeF7 V1.17.4 Projects/STM32F769I-Discovery/Templates.
  *   (NUCLEO-F767ZI instead uses the 8 MHz ST-LINK MCO in HSE bypass mode.)
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Activate the Over-Drive mode to reach 216 MHz */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization: ST-LINK VCP, 115200 8N1, no flow control.
  *   tm_com_init() (BSP2 libtm) later re-programs the same settings directly.
  */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization: LD1 (PJ13), LD2 (PJ5) outputs, B1 USER (PA0) input.
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();

  /* LEDs off at start */
  HAL_GPIO_WritePin(GPIOJ, LD1_RED_Pin | LD2_GREEN_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = LD1_RED_Pin | LD2_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /* B1 USER button has an external pull-down on the board; no internal pull */
  GPIO_InitStruct.Pin = B_USER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B_USER_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief MPU: background region forbidding speculative access to the
  *   external memory windows (FMC/QSPI, 0x60000000-0xDFFFFFFF) that are not
  *   initialised in this project. Same as ST's F769I-Discovery template.
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  HAL_MPU_Disable();

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  This function is executed in case of error occurrence.
  *         Fast-blinks LD1 (red) with a busy loop (no tick needed).
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(LD1_RED_GPIO_Port, LD1_RED_Pin);
    for (volatile uint32_t i = 0; i < 2000000; i++) { }
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif /* USE_FULL_ASSERT */
