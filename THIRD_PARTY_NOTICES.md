# Licenses and third-party notices

This repository combines code under several licenses. Each component keeps its own license.

| Path | Component | Copyright | License |
|---|---|---|---|
| `mtk3_bsp2/` (copy of [tron-forum/mtk3_bsp2](https://github.com/tron-forum/mtk3_bsp2) v1.00.04) | μT-Kernel 3.0 BSP2, including the STM32F769I-DISCOVERY board files added by this port | Ken Sakamura / TRON Forum | [T-License 2.2](https://www.tron.org/download/index.php?route=information/information&information_id=79) |
| `mtk3_bsp2/mtkernel/` (submodule of the official [tron-forum/mtkernel_3](https://github.com/tron-forum/mtkernel_3), not copied) | μT-Kernel 3.0 | Ken Sakamura / TRON Forum | T-License 2.2 |
| `Drivers/STM32F7xx_HAL_Driver/` | STM32CubeF7 HAL driver (from STM32CubeF7 V1.17.4) | STMicroelectronics | BSD-3-Clause, see `Drivers/STM32F7xx_HAL_Driver/LICENSE.txt` |
| `Drivers/CMSIS/Device/ST/STM32F7xx/` | STM32F7xx CMSIS device headers | STMicroelectronics | Apache-2.0, see `Drivers/CMSIS/Device/ST/STM32F7xx/LICENSE.txt` |
| `Drivers/CMSIS/Include/` | CMSIS-Core | Arm Limited | Apache-2.0, see `Drivers/CMSIS/LICENSE.txt` |
| `Core/Startup/startup_stm32f769xx.s`, `Core/Src/system_stm32f7xx.c` | CMSIS device templates (unmodified) | STMicroelectronics | Apache-2.0 (CMSIS device component, see `Drivers/CMSIS/Device/ST/STM32F7xx/LICENSE.txt`) |
| `Core/Src/stm32f7xx_hal_timebase_tim.c`, `Core/Inc/stm32f7xx_hal_conf.h` | HAL templates (unmodified apart from the enabled modules in `stm32f7xx_hal_conf.h`) | STMicroelectronics | BSD-3-Clause (HAL component, see `Drivers/STM32F7xx_HAL_Driver/LICENSE.txt`) |
| `Core/Src/main.c`, `Core/Src/stm32f7xx_hal_msp.c`, `Core/Src/stm32f7xx_it.c`, `Core/Inc/main.h`, `Core/Inc/stm32f7xx_it.h`, `STM32F769NIHX_FLASH.ld` | STM32Cube project files. The clock and MPU configuration are taken from STM32CubeF7 `Projects/STM32F769I-Discovery/Templates`. | STMicroelectronics, Madhawa Polkotuwa | BSD-3-Clause |
| `application/`, `Makefile`, `.github/`, `README.md`, and other files without their own header | Project code and documentation | Madhawa Polkotuwa | MIT, see `LICENSE` |

The T-License 2.2 places conditions on redistribution, such as keeping the copyright notices and stating modifications. Read it before you redistribute the `mtk3_bsp2` part. Files added or changed for this port say so in their header (`STM32F769I-DISCOVERY port: derived from ...`).

## Trademarks

TRON, T-Kernel and μT-Kernel are names or trademarks of their respective owners. STM32 and STM32Cube are trademarks of STMicroelectronics. This project is not affiliated with or endorsed by TRON Forum or STMicroelectronics.
