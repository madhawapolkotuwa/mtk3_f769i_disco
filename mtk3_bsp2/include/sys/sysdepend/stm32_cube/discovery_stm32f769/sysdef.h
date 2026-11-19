/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.0 BSP 2.0
 *
 *    Copyright (C) 2023-2024 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2024/08.
 *
 *    STM32F769I-DISCOVERY port: derived from nucleo_stm32f767/sysdef.h
 *----------------------------------------------------------------------
 */

/*
 *	sysdef.h
 *
 *	System dependencies definition (STM32Cube STM32F769I-DISCOVERY)
 *	Included also from assembler program.
 *
 *	The STM32F769NI has the same internal memory map as the STM32F767ZI
 *	(2MB Flash, DTCM 128KB, SRAM1 368KB, SRAM2 16KB) and the same number
 *	of interrupt vectors (110: DSI_IRQn occupies vector 98, which is
 *	reserved on the F767). So the STM32F7 CPU definition is used as is.
 *
 *	The kernel uses only internal SRAM1+SRAM2 for its system memory.
 *	External SDRAM (FMC, 0xC0000000) and Quad-SPI flash (0x90000000)
 *	on this board are NOT used by the kernel.
 */

#ifndef _MTKBSP_SYS_SYSDEF_DEPEND_H_
#define _MTKBSP_SYS_SYSDEF_DEPEND_H_

/* CPU-dependent definition */
#include <sys/sysdepend/stm32_cube/cpu/stm32f7/sysdef.h>

#endif /* _MTKBSP_TK_SYSDEF_DEPEND_H_ */
