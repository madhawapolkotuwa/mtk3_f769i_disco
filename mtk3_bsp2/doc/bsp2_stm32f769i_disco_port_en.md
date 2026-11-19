# μT-Kernel 3.0 BSP2: STM32F769I-DISCOVERY port notes

Base: `mtk3_bsp2` v1.00.04 (commit `1ab52cc`), μT-Kernel `mtkernel_3` v3.00.07 (submodule `435096c`).
Reference board: NUCLEO-F767ZI (`_STM32CUBE_NUCLEO_F767_`).
New board: STM32F769I-DISCOVERY (MB1225), MCU STM32F769NIH6.
Target symbol: **`_STM32CUBE_DISCOVERY_F769_`**

Status: builds cleanly with GNU Arm 13.3.rel1 (STM32CubeCLT 1.18.0). **Not yet verified on hardware.**

---

## 1. How BSP2 is put together

### 1.1 Directory layout

| Path | Role | Board specific? |
|---|---|---|
| `config/config.h` | Kernel configuration (`CNF_*`, `USE_*`): object counts, tick period, FPU, T-Monitor | no |
| `config/config_bsp/stm32_cube/config_bsp.h` | BSP options (sample drivers, static system memory) | per vendor |
| `config/config_tm.h` | T-Monitor console options | no |
| `include/sys/machine.h` | **Board selector**: maps `_STM32CUBE_xxx_` to a board `machine.h` | one `#elif` per board |
| `include/sys/sysdepend/stm32_cube/<board>/` | `machine.h` (identity and directory names), `profile.h` (service profile), `sysdef.h` (forwards to the CPU sysdef) | **yes** |
| `include/sys/sysdepend/stm32_cube/cpu/stm32f7/sysdef.h` | F7 memory map for the kernel, vector count, interrupt priorities, EXTI, TIM2-5 | per series |
| `include/sys/sysdepend/stm32_cube/cpu/core/armv7m/` | SCB, NVIC, SysTick registers, `SYSCLK` = `knl_sysclk` | per core |
| `sysdepend/stm32_cube/cpu/core/armv7m/` | `sys_start.c` (kernel entry), `dispatch.S` (PendSV dispatcher), `interrupt.c`, `sys_timer.h` (SysTick), `cpu_cntl.c` | per core |
| `sysdepend/stm32_cube/lib/libtk/` | `int_armv7m.c` (NVIC API), `cpu/stm32f7/int_stm32f7.c` (EXTI), physical timer | per series |
| `sysdepend/stm32_cube/lib/libtm/<board>/tm_com.c` | **Console UART** (polled, direct registers) | **yes** |
| `sysdepend/stm32_cube/{hw_setting.c,devinit.c,power_save.c}` | Hardware start and shutdown, device driver registration | shared (with `#if` per board) |
| `sysdepend/stm32_cube/device/` | Sample `hal_adc` / `hal_i2c` drivers built on the STM32Cube HAL | shared |
| `mtkernel/` (submodule) | Hardware-independent kernel. BSP2 uses only its common part | no |
| `etc/linker/mtkernel.ld` | Linker fragment for `.mtk_exctbl` | no |

`machine.h` sets three macros, and the generic headers build include paths from them:
```
TARGET_DIR      stm32_cube/discovery_stm32f769   -> sys/sysdepend/TARGET_DIR/{sysdef,profile}.h
TARGET_GRP_DIR  stm32_cube                       -> config_bsp/TARGET_GRP_DIR/config_bsp.h
TARGET_CPU_DIR  stm32f7
```

### 1.2 Build system

BSP2 has **no build system of its own**. The BSP2 manual (`doc/bsp2_stm32_cube_jp.md` §4) works like this:
1. Generate a project with STM32CubeMX. Cube supplies the startup `.s`, the linker script, `system_stm32f7xx.c`, the HAL, and `main.c` with the clock, GPIO and UART initialisation.
2. Clone `mtk3_bsp2` into the project directory.
3. Add the target symbol and four include paths (`mtk3_bsp2`, `mtk3_bsp2/config`, `mtk3_bsp2/include`, `mtk3_bsp2/mtkernel/kernel/knlinc`).
4. Call `knl_start_mtkernel()` from `main()` in `USER CODE BEGIN 2`.

The IDE compiles **every** source file under `mtk3_bsp2`. Code for other boards or vendors drops out through `#if defined(MTKBSP_xxx)` guards at the top of each file. The `Makefile` in this project does the same.

### 1.3 Boot sequence and kernel hooks

```
Reset_Handler (Cube startup_stm32f769xx.s)
  SystemInit -> copy .data / zero .bss -> __libc_init_array -> main()
main()  (Core/Src/main.c)
  MPU_Config, I/D-cache, HAL_Init (HAL time base = TIM6), SystemClock_Config,
  MX_GPIO_Init, MX_USART1_UART_Init
  knl_start_mtkernel()            sysdepend/stm32_cube/cpu/core/armv7m/sys_start.c
    disint, knl_startup_hw
    copy the flash vector table (VTOR) into knl_exctbl[] in RAM, point VTOR at it
    AIRCR priority grouping, SVC pri 0, SysTick pri 1, PendSV pri 15
    knl_sysclk = HAL_RCC_GetSysClockFreq()
    system memory = [max(0x20020000, _end), 0x20080000 - EXC_STACK_SIZE)
    knl_main()                     mtkernel/kernel/sysinit/sysinit.c
      libtm_init -> tm_com_init()  console UART  (board tm_com.c)
      knl_init_Imalloc             kernel heap
      knl_init_device              devinit.c (empty)
      knl_init_interrupt           knl_exctbl[14] = knl_dispatch_entry (PendSV)
                                   knl_exctbl[15] = knl_systim_inthdr  (SysTick)
      knl_init_object
      knl_timer_startup            SysTick: core clock, RVR = TIMER_PERIOD * SYSCLK/1000 - 1
      tk_cre_tsk/tk_sta_tsk(init task, pri 1)  -> first dispatch
  init task: knl_start_device, banner "microT-Kernel Version 3.00", usermain()
```

* **System tick.** SysTick, clocked from the core, with `CNF_TIMER_PERIOD` = **10 ms** by default. The kernel owns SysTick, so the HAL needs a different time base (§3.1).
* **Interrupt controller.** The NVIC is used directly. `tk_def_int()` writes the RAM vector table (`knl_define_inthdr`). A `TA_HLNG` handler is called through `knl_hll_inthdr`, which looks it up in `hllint_tbl[]` and brackets it with `ENTER/LEAVE_TASK_INDEPENDENT`. `EnableInt`/`DisableInt`/`SetIntMode` live in `lib/libtk` (with EXTI support for the F7). Cube handlers that are not registered with `tk_def_int()` stay in the copied table and keep working, but they must not call `tk_*` services.
* **Console.** `tm_com.c` per board provides `tm_snd_dat`, `tm_rcv_dat` and `tm_com_init`, polled, with direct USART register access. It assumes Cube has already enabled the bus clock and pin muxing. `tm_putchar` converts LF to CR-LF and sends each character with interrupts disabled, so it is a debug channel only.

### 1.4 Compared with Zephyr and FreeRTOS

| | μT-Kernel 3.0 (BSP2) | FreeRTOS | Zephyr |
|---|---|---|---|
| **Task creation** | `tk_cre_tsk(&T_CTSK)` returns an `ID` chosen by the kernel. The task starts **DORMANT**. `tk_sta_tsk(id, stacd)` makes it READY and passes a start code. Ending with `tk_ext_tsk()` returns it to DORMANT, so it can be restarted. `tk_exd_tsk()` exits and deletes. | `xTaskCreate()` creates **and** readies in one call and returns a handle. `vTaskDelete()` | `k_thread_create()` (optional start delay) or the static `K_THREAD_DEFINE` |
| Stacks and objects | The kernel allocates the stack from system memory unless `TA_USERBUF` is set. The object count is fixed at build time (`CNF_MAX_TSKID` and so on). | Heap (`heap_x.c`) or static | Usually static (`K_THREAD_STACK_DEFINE`) |
| Priority | **1 is highest**, up to `CNF_MAX_TSKPRI` (32) | A higher number is higher priority | A lower number is higher. Negative values are cooperative. |
| **System tick** | Fixed-period tick, `CNF_TIMER_PERIOD` ms (10 ms here). The API takes **milliseconds** (`TMO`, `RELTIM`). `_u` variants take µs (`tk_dly_tsk_u`). No tickless mode in this BSP. | `configTICK_RATE_HZ`. The API takes **ticks** (`pdMS_TO_TICKS`). Optional tickless idle. | `CONFIG_SYS_CLOCK_TICKS_PER_SEC`. `k_timeout_t` (`K_MSEC`). Tickless by default on most SoCs. |
| Who owns SysTick | The kernel. It patches vector 15 in its RAM copy of the vector table. | The port's `xPortSysTickHandler` (you wire it up) | The system timer driver |
| **Service call interface** | Plain C function calls into the kernel (no SVC trap on ARMv7-M). Everything runs privileged in a single image. Every call returns `ER` (`E_OK`/negative `E_xxx`) or a positive `ID`. The kernel checks context and returns `E_CTX`, for example for a wait called from a handler. | Plain function calls. Separate `...FromISR()` variants for interrupt context. | Function calls, or syscalls through an SVC trap when `CONFIG_USERSPACE` is set. Most give/signal calls are ISR-safe. |
| ISR rules | The **same** non-waiting calls (`tk_sig_sem`, `tk_set_flg`, …) work from a handler; dispatch is delayed until the handler returns. Waiting calls return `E_CTX`. | Only `FromISR` variants, plus `portYIELD_FROM_ISR` | `K_NO_WAIT` only from an ISR |
| Naming | `tk_<op>_<obj>`: `cre`/`del`/`sta`/`ext`/`wai`/`sig`/`slp`/`wup`/`dly`/`ref`. `T_C<OBJ>` is a create packet and `T_R<OBJ>` a reference packet. `TA_` attributes, `TMO_` timeouts, `E_` errors. | `x`/`v` prefix by return type plus module | `k_<obj>_<op>` |

---

## 2. What differs between the F769I-DISCO and the F767ZI reference

| Item | NUCLEO-F767ZI (reference) | STM32F769I-DISCOVERY (this port) | Where it is handled | Source |
|---|---|---|---|---|
| Core, FPU | Cortex-M7, DP-FPU | same | unchanged (`armv7m`, `-mfpu=fpv5-d16`) | RM0410 |
| Internal memory | 2 MB flash, DTCM 128 KB + SRAM1 368 KB + SRAM2 16 KB | **same** | `cpu/stm32f7/sysdef.h` unchanged | `stm32f767xx.h` vs `stm32f769xx.h` |
| Interrupt vectors | 110 (98 reserved) | 110 (**98 = DSI**) | `N_INTVEC` unchanged | same headers |
| HSE | 8 MHz MCO from ST-LINK, bypass (CubeMX default) | **25 MHz crystal**, `RCC_HSE_ON` | `main.c` `SystemClock_Config`, `HSE_VALUE` | STM32CubeF7 F769I-Discovery Templates ⚠ |
| PLL | M=4 N=216 P=2 (from 8 MHz) | **M=25 N=432 P=2 Q=9 R=7**, OverDrive, 7 WS | `main.c` | same template |
| SYSCLK / APB1 / APB2 | 216 / 54 / 108 MHz | 216 / 54 / 108 MHz | – | – |
| Console UART | USART3 (APB1), PD8/PD9 | **USART1 (APB2), PA9 TX / PA10 RX, AF7** | **new** `lib/libtm/discovery_stm32f769/tm_com.c` (base `0x40011000`, **PCLK2**) and `stm32f7xx_hal_msp.c` | STM32CubeF7 mbedTLS example `STLINK_USARTx` ⚠ |
| User LEDs | LD1 green PB0, LD2 blue PB7, LD3 red PB14 | **LD1 red PJ13, LD2 green PJ5**, active high | `main.h`, `MX_GPIO_Init` (Cube side) | STM32CubeF7 `stm32f769i_discovery.h` ⚠ |
| User button | B1 PC13 | **B1 PA0**, high when pressed | `main.h`, `MX_GPIO_Init` | same ⚠ |
| External SDRAM | none | 16 MB, 32-bit, FMC at `0xC0000000` | **not used**. The MPU background region blocks speculative access. | `stm32f769i_discovery_sdram.h` |
| QSPI flash | none | MX25L512 (64 MB) at `0x90000000` | **not used** | `stm32f769i_discovery_qspi.h` |
| Arduino ADC pins | `hadc1`/`hadc3` mapping in `devinit.c` | unknown | `devinit.c` **unchanged**. The F769 falls into the generic `#else` branch, and `DEVCNF_USE_HAL_ADC` = 0. | – |

⚠ = from ST's STM32CubeF7 V1.17.4 package (BSP and examples), **not yet checked against UM2033 or the schematic**. See §4.

The NUCLEO-F767ZI column (pins and CubeMX default clock) is given for orientation, from UM1974 and CubeMX defaults. It was **not** re-verified here, because the port does not depend on it.

## 3. What was changed

### 3.1 Inside `mtk3_bsp2` (shows as the git diff)

| File | Change | Why |
|---|---|---|
| `include/sys/machine.h` | **modified, +3 lines**: `#elif defined(_STM32CUBE_DISCOVERY_F769_)` | Required. This is the only board selector. |
| `include/sys/sysdepend/stm32_cube/discovery_stm32f769/machine.h` | new, from F767 | Board identity (`MTKBSP_DISCOVERY_STM32F769`, `MTKBSP_CPU_STM32F769`) and `TARGET_DIR` |
| `include/sys/sysdepend/stm32_cube/discovery_stm32f769/profile.h` | new, identical content | Required to exist, through `TARGET_DIR` |
| `include/sys/sysdepend/stm32_cube/discovery_stm32f769/sysdef.h` | new, identical content | Forwards to `cpu/stm32f7/sysdef.h`, because the memory map is identical |
| `sysdepend/stm32_cube/lib/libtm/discovery_stm32f769/tm_com.c` | new, from F767 | USART1 base `0x40011000`, BRR from `halif_get_pclk2()`. Also drops the unused `get_pclk1()` prototype. |
| `doc/bsp2_stm32f769i_disco_port_en.md` | new | this document |

Kernel files (`mtkernel/`), CPU and core files, `config/*`, `devinit.c` and `hw_setting.c` are **unchanged**.

Naming follows the existing Discovery board, STM32N6570-DK: `_STM32CUBE_DISCOVERY_N657_` with directory `discovery_stm32n657`.

### 3.2 STM32Cube project (outside `mtk3_bsp2`, normally generated by CubeMX)

* `Core/Src/main.c`: CubeMX layout. The ST reference clock tree, USART1, LEDs and button, the MPU background region, I/D-cache, and `knl_start_mtkernel()` in USER CODE 2. It sends one HAL-driver line before the kernel starts, as a bring-up checkpoint.
* **HAL time base = TIM6** (`stm32f7xx_hal_timebase_tim.c`, ST template). In CubeMX this is *SYS → Timebase Source = TIM6*. Once the kernel owns SysTick, a SysTick-based `uwTick` would stop, and any HAL call with a timeout would hang after kernel start. BSP2 itself does nothing about this (§5).
* `stm32f7xx_it.c` does not define PendSV, SysTick or SVC handlers (the kernel owns them; SVC is unused).
* `STM32F769NIHX_FLASH.ld`: CubeIDE layout, with explicit `.noinit (NOLOAD)` and `.mtk_bsp2 (NOLOAD)` sections (the contents of `etc/linker/mtkernel.ld`) placed **before** `_end`, so the kernel system memory starts after them.
* `stm32f7xx_hal_conf.h`: `HSE_VALUE` = 25 MHz. Only the RCC, GPIO, DMA, Cortex, Flash, PWR, EXTI, TIM and UART modules are enabled.

### 3.3 Resulting memory use (stage 4 image)

```
FLASH 0x08000000  .isr_vector, .text, .rodata             ~20.9 KB
RAM   0x20000000  .data/.bss                               ~0.5 KB  (DTCM)
      0x20000230  .noinit  (kernel control blocks)         ~8.7 KB
      0x20002600  .mtk_bsp2 knl_exctbl (512-byte aligned)  2016 B
      0x20002DE0  _end, then the Cube min heap/stack check
      0x20020000 .. 0x20080000   kernel system memory (task stacks etc.)  384 KB
      0x20080000  _estack (boot MSP, abandoned after the first dispatch)
```

## 4. Assumptions to verify on the board or in the documentation

These values come from ST's STM32CubeF7 package, not from the board manual. Check them against **UM2033** and the MB1225 schematic, and on hardware:

1. **HSE**: a 25 MHz *crystal* (X2) driven with `RCC_HSE_ON`. If the board revision uses an oscillator, `RCC_HSE_BYPASS` would be needed. Symptom of a mismatch: `Error_Handler` (fast red LED blink), with no `[boot]` line.
2. **VCP = USART1 on PA9/PA10**. Also check whether any solder bridge must be closed on your board revision.
3. **LD1 = PJ13 (red), LD2 = PJ5 (green), active high.** UM2033 may list further LEDs (power, ST-LINK, USB) that are not user-controllable.
4. **B1 USER = PA0, high when pressed, external pull-down** (`GPIO_NOPULL` is used).
5. **VOS scale 1 and OverDrive at 216 MHz with 7 wait states** at 3.3 V (STM32F769 datasheet / RM0410 table "Number of wait states according to CPU clock (HCLK) frequency").
6. Board revision (MB1225 B-0x). Nothing here depends on it, but record it.

## 5. Open questions and known issues

1. **Boot stack against kernel memory.** With `CNF_EXC_STACK_SIZE = 0` (upstream default), the kernel memory limit is `0x20080000`. That equals `_estack`, so the end-of-heap marker overlaps `main()`'s (abandoned) frame. It is harmless as long as allocation is first-fit from the bottom, and the F767 reference behaves identically. Because it is not F769-specific, upstream was not changed. A safe local mitigation is `CNF_EXC_STACK_SIZE (1024)` in `config/config.h`.
2. **HAL tick.** Upstream BSP2 doesn't document a HAL time base. The F767 CubeMX default (SysTick) would freeze `HAL_GetTick()` after kernel start. Should this be raised with TRON Forum, or documented as a required CubeMX setting?
3. **Cube ISRs not registered with `tk_def_int()`** (TIM6 here) run outside the kernel's knowledge. That is fine while they make no `tk_*` calls. Peripheral interrupts that need to signal tasks should be registered with `tk_def_int()` with `TA_HLNG`.
4. **D-cache is on.** With future DMA drivers (SDMMC, Ethernet, LTDC/DSI), cache maintenance or MPU non-cacheable regions are needed. The BSP2 sample drivers use interrupts, not DMA.
5. **SDRAM, QSPI, LCD/DSI** are not initialised. To use SDRAM as kernel memory you would set `CNF_SYSTEMAREA_TOP`/`END` or `USE_STATIC_SYS_MEM`, after FMC initialisation in `main()` **before** `knl_start_mtkernel()`, and add an MPU region for it.
6. **ADC sample driver mapping** for the F769I-DISCO Arduino connector is not defined in `devinit.c`.
7. Upstream warning: `int_stm32f7.c:60 'pif' may be used uninitialized` (shared with the F767, not changed).
