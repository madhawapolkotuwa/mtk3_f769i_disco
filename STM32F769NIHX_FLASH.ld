/*
******************************************************************************
**  STM32F769NIHx linker script (STM32CubeIDE layout) + uT-Kernel 3.0 BSP2
**
**  FLASH : 2048 KB @ 0x08000000 (AXIM)
**  RAM   :  512 KB @ 0x20000000 (DTCM 128 KB + SRAM1 368 KB + SRAM2 16 KB)
**
**  Same internal memory map as STM32F767ZI (NUCLEO-F767ZI).
**  External SDRAM (0xC0000000, 16 MB) and QSPI flash (0x90000000, 64 MB) on
**  the STM32F769I-DISCOVERY are not used.
**
**  uT-Kernel memory usage (see sys_start.c / cpu/stm32f7/sysdef.h):
**    .data/.bss/boot stack are placed from 0x20000000 (DTCM) by this script.
**    The kernel system memory (task stacks, control blocks) is taken from
**    max(INTERNAL_RAM_START=0x20020000, _end) up to 0x20080000.
******************************************************************************
*/
/* Copyright (c) 2016 STMicroelectronics (CubeIDE linker script layout).
 * Copyright (c) 2026 Madhawa Polkotuwa (uT-Kernel BSP2 sections).
 * SPDX-License-Identifier: BSD-3-Clause */

ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);	/* end of "RAM" Ram type memory */

_Min_Heap_Size = 0x200;		/* required amount of heap  */
_Min_Stack_Size = 0x400;	/* required amount of stack */

MEMORY
{
  RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 512K
  FLASH    (rx)    : ORIGIN = 0x8000000,   LENGTH = 2048K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM (READONLY) : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array (READONLY) :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array (READONLY) :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array (READONLY) :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    *(.RamFunc)
    *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* uT-Kernel object/control-block tables declared with Noinit() (machine.h).
   * Not zeroed by the startup code; the kernel initialises them itself.
   * (Without this rule ld places them here anyway as an orphan section.) */
  .noinit (NOLOAD) :
  {
    . = ALIGN(8);
    *(.noinit)
    *(.noinit*)
    . = ALIGN(8);
  } >RAM

  /* ---- uT-Kernel 3.0 BSP2 (same as mtk3_bsp2/etc/linker/mtkernel.ld) ----
   * knl_exctbl: RAM exception vector table (512-byte aligned, filled at
   * run time by knl_start_mtkernel()), therefore NOLOAD.
   * Placed before _end so the kernel system memory never overlaps it. */
  .mtk_bsp2 (NOLOAD) :
  {
    *(.mtk_exctbl)
    __mtk3_SYSMEM_START = .;
  } >RAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Remove information from the compiler libraries (only sections not
   * already placed above, e.g. .comment) */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
