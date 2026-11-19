# μT-Kernel 3.0 BSP2 for STM32F769I-DISCOVERY

[![build](https://github.com/madhawapolkotuwa/mtk3_f769i_disco/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/madhawapolkotuwa/mtk3_f769i_disco/actions/workflows/build.yml?query=branch%3Amain)

A board support package (BSP) that runs the real-time OS **μT-Kernel 3.0** on the
**STM32F769I-DISCOVERY** kit (STM32F769NI, Cortex-M7, 216 MHz). It extends TRON Forum's
[mtk3_bsp2](https://github.com/tron-forum/mtk3_bsp2), which does not support this board,
and it is derived from the official NUCLEO-F767ZI support in that repository.

The port keeps the `mtk3_bsp2` structure, so it will look familiar to μT-Kernel developers.
It adds one target, `_STM32CUBE_DISCOVERY_F769_`, plus 4 board files. The only change to
an existing upstream file is 3 lines in `include/sys/machine.h`. The kernel itself is unmodified.

> **Status:** builds with GNU Arm 13.3. Hardware verification is in progress. Pin and clock
> settings come from ST's STM32CubeF7 package and are still being checked against UM2033
> (see [open items](mtk3_bsp2/doc/bsp2_stm32f769i_disco_port_en.md#4-assumptions-to-verify-on-the-board-or-in-the-documentation)).
>
> This is a community port. It is not affiliated with or endorsed by TRON Forum or STMicroelectronics.

## Features

- μT-Kernel 3.0 (v3.00.07) through mtk3_bsp2 v1.00.04 on STM32CubeF7 HAL V1.17.4
- 216 MHz from the 25 MHz HSE, with I/D-cache, an MPU background region and the FPU
- T-Monitor console (`tm_printf`) on the ST-LINK virtual COM port (USART1, 115200 8N1)
- SysTick kernel tick (10 ms) and a TIM6 HAL time base, so HAL timeouts keep working under the kernel
- Plain `make` build (Windows, Linux, macOS) that is equivalent to the STM32CubeIDE setup in the BSP2 manual
- Step-by-step demo programs:
  - **Milestone 3:** boot, system tick, console
  - **Milestone 4:** a task, a semaphore and a wait with a timeout, shown with the LEDs and the console
- GitHub Actions CI that builds both milestones

## Repository layout

```
.
├── Core/{Inc,Src,Startup}    STM32Cube part: main.c (clock, GPIO, USART1, MPU), MSP, IRQs, startup, TIM6 timebase
├── Drivers/                  STM32CubeF7 V1.17.4 CMSIS + HAL (subset)
├── application/usermain.c    user program (APP_STAGE 3 / 4)
├── mtk3_bsp2/                TRON Forum mtk3_bsp2 v1.00.04 + the F769I-DISCO board files
│   ├── include/sys/sysdepend/stm32_cube/discovery_stm32f769/   board headers   (new)
│   ├── sysdepend/stm32_cube/lib/libtm/discovery_stm32f769/     console UART    (new)
│   ├── doc/bsp2_stm32f769i_disco_port_en.md                    port notes      (new)
│   └── mtkernel/             submodule: official tron-forum/mtkernel_3 v3.00.07 (unmodified)
├── STM32F769NIHX_FLASH.ld    linker script (CubeIDE layout + BSP2 sections)
├── .github/workflows/build.yml  CI: builds milestones 3 and 4 on every push / pull request
└── Makefile
```

## Requirements

- STM32F769I-DISCOVERY board and a USB cable for the ST-LINK connector (CN16)
- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) `arm-none-eabi` 13.x (13.3.Rel1 tested). STM32CubeCLT includes it.
- GNU make (on Windows, the copy bundled with STM32CubeIDE works) and git
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) (`STM32_Programmer_CLI`) or [OpenOCD](https://openocd.org) to flash
- A serial terminal set to 115200 baud, 8N1, no flow control

| Host | Status |
|---|---|
| Windows 11 (Git Bash / PowerShell, STM32CubeCLT 1.18) | build tested |
| Linux (Ubuntu 24.04, Arm GNU Toolchain 13.3.Rel1) | same setup as the CI workflow |
| macOS | not yet tested. The commands below are the standard setup. |

## Getting the source

The μT-Kernel source (`mtk3_bsp2/mtkernel`) is a submodule of the official TRON Forum repository, so `--recursive` is required:

```bash
git clone --recursive https://github.com/madhawapolkotuwa/mtk3_f769i_disco.git
cd mtk3_f769i_disco
```

If you already cloned without `--recursive`, run `git submodule update --init --recursive`.

## Tool setup

Do this once per machine. The `PATH` lines apply to the current shell only; add them to your shell profile to make them permanent.

### Windows

Install [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html) 1.18, which provides the toolchain and STM32_Programmer_CLI, and [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) 1.18, which provides `make`. Then:

**Git Bash**
```bash
export PATH="/c/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin:/c/ST/STM32CubeCLT_1.18.0/STM32CubeProgrammer/bin:/c/ST/STM32CubeIDE_1.18.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845/tools/bin:$PATH"
```

**PowerShell**
```powershell
$env:PATH = "C:\ST\STM32CubeCLT_1.18.0\GNU-tools-for-STM32\bin;C:\ST\STM32CubeCLT_1.18.0\STM32CubeProgrammer\bin;C:\ST\STM32CubeIDE_1.18.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin;" + $env:PATH
```

### Linux (Ubuntu / Debian, x86_64)

```bash
sudo apt update
sudo apt install -y make git curl xz-utils

# Arm GNU Toolchain 13.3.Rel1 (official Arm build, as used by the CI)
curl -LO https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
sudo tar -xJf arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz -C /opt
export PATH="/opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin:$PATH"
arm-none-eabi-gcc --version
```

For an aarch64 Linux host, use `arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz`.
The distribution packages (`sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi`) should also work, but they are not tested here.

For flashing, install either:
- **STM32CubeProgrammer for Linux** from ST, then add it to `PATH`. The default install location is
  `~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin`; with STM32CubeCLT it is
  `/opt/st/stm32cubeclt_1.18.0/STM32CubeProgrammer/bin`. Adjust to your install.
  ```bash
  export PATH="$HOME/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin:$PATH"
  ```
- **OpenOCD**: `sudo apt install -y openocd`

To use the ST-LINK and its serial port without root, install the ST-LINK udev rules (the STM32CubeProgrammer installer does this; with OpenOCD only, `sudo apt install stlink-tools` provides them) and add yourself to the `dialout` group:
```bash
sudo usermod -aG dialout "$USER"     # log out and back in afterwards
```

### macOS (Apple Silicon or Intel)

```bash
xcode-select --install               # provides make and git
brew install --cask gcc-arm-embedded # Arm GNU Toolchain (official Arm package)
arm-none-eabi-gcc --version
```

Without Homebrew, download `arm-gnu-toolchain-13.3.rel1-darwin-arm64-arm-none-eabi.pkg` (Apple Silicon) or
`...-darwin-x86_64-arm-none-eabi.pkg` (Intel) from the [Arm download page](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and add its `bin` directory to `PATH`.

For flashing, install either:
- **STM32CubeProgrammer for macOS** from ST. Its CLI is inside the app bundle, at a path like
  `/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin`. Adjust to your install.
  ```bash
  export PATH="/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin:$PATH"
  ```
- **OpenOCD**: `brew install open-ocd`

macOS needs no USB driver for the ST-LINK.

## Building

The commands are the same on all three systems:

```bash
make STAGE=3 -j8      # milestone 3: boot + tick + console
make -j8              # milestone 4 (default): task + semaphore + timeout
make clean
```

To use every CPU core: `make -j"$(nproc)"` on Linux, `make -j"$(sysctl -n hw.ncpu)"` on macOS.
Output goes to `build/stage<N>/mtk3_f769i_disco.{elf,hex,bin,map}`.

Expected end of the build (milestone 3):
```
LD build/stage3/mtk3_f769i_disco.elf
Memory region         Used Size  Region Size  %age Used
             RAM:       13280 B       512 KB      2.53%
           FLASH:       19276 B         2 MB      0.92%
```
You will also see two kinds of harmless warning. One is newlib's "`_close`/`_read`/`_write`/`_lseek` is not implemented" (from `nosys.specs`, nothing uses them). The other is an upstream `maybe-uninitialized` warning in `int_stm32f7.c`.

## Flashing and running

### With STM32CubeProgrammer (all systems)

```bash
make STAGE=3 flash    # or: make flash   for milestone 4
```

This runs `STM32_Programmer_CLI -c port=SWD mode=UR -w <elf> -v -rst`. If the CLI is not on `PATH`, pass its location:
```bash
make flash PROGRAMMER=/path/to/STM32_Programmer_CLI
```

### With OpenOCD (Linux / macOS / Windows)

```bash
make STAGE=3
openocd -f board/stm32f769i-disco.cfg \
        -c "program build/stage3/mtk3_f769i_disco.elf verify reset exit"
```
(`board/stm32f769i-disco.cfg` ships with OpenOCD. If your version lacks it, use
`-f interface/stlink.cfg -f target/stm32f7x.cfg`.)

### Opening the console

Connect to the ST-LINK virtual COM port at 115200 8N1, then press the black RESET button.

| Host | Port name | Example |
|---|---|---|
| Windows | `COMx` (*STMicroelectronics STLink Virtual COM Port* in Device Manager) | PuTTY or Tera Term, Serial, `COMx`, 115200 |
| Linux | `/dev/ttyACM0` (check with `ls /dev/ttyACM*`) | `picocom -b 115200 /dev/ttyACM0` (exit: Ctrl-A Ctrl-X)<br>`screen /dev/ttyACM0 115200` (exit: Ctrl-A K) |
| macOS | `/dev/cu.usbmodem*` (check with `ls /dev/cu.usbmodem*`) | `screen /dev/cu.usbmodemXXXX 115200` (exit: Ctrl-A K)<br>`brew install picocom`, then `picocom -b 115200 /dev/cu.usbmodemXXXX` |

### Milestone 3: boot, system tick, console

```
[boot] STM32F769I-DISCO: HAL init done, starting uT-Kernel


microT-Kernel Version 3.00

uT-Kernel 3.0 BSP2 on STM32F769I-DISCOVERY
SYSCLK = 216000000 Hz, tick = 10 ms
[       0 ms] alive 0
[    1000 ms] alive 1
[    2000 ms] alive 2
```
LD2 (green) toggles once a second.

### Milestone 4: task, semaphore, timeout wait

```
uT-Kernel 3.0 BSP2 on STM32F769I-DISCOVERY
SYSCLK = 216000000 Hz, tick = 10 ms
usermain: semid=1 led_tskid=2
led_task: started, stacd=1897
[     530 ms] led_task: timeout (#1) -> LD2 green
[    1030 ms] led_task: timeout (#2) -> LD2 green
[    1530 ms] led_task: timeout (#3) -> LD2 green
[    1720 ms] usermain: tk_sig_sem (periodic)
[    1720 ms] led_task: semaphore acquired (#1) -> LD1 red
[    2230 ms] led_task: timeout (#4) -> LD2 green
```
`led_task` waits with `tk_wai_sem(semid, 1, 500)`. The green LED toggles on each 500 ms timeout.
The red LED toggles when `usermain` signals the semaphore, which happens every 1.7 s or when you press the blue USER button.
The millisecond values can differ by a few ticks.

### Troubleshooting

| Symptom | Likely cause |
|---|---|
| No output, LD1 red blinks fast | `Error_Handler`: the clock setup (HSE/PLL) failed |
| No output, no blinking | Wrong COM port or baud rate, or a fault before the UART was ready |
| Only the `[boot]` line | The HAL side works. Kernel start or the T-Monitor UART (USART1/PCLK2) failed. |
| Garbage characters | Baud mismatch: `HSE_VALUE` does not match the crystal |
| `alive` stops or runs at the wrong rate | SysTick / `knl_sysclk` problem |

## How the port works

The board-specific code is small because the STM32F769 and STM32F767 share the core, the memory map and the vector table:

| | NUCLEO-F767ZI (upstream) | STM32F769I-DISCOVERY (this port) |
|---|---|---|
| Target symbol | `_STM32CUBE_NUCLEO_F767_` | `_STM32CUBE_DISCOVERY_F769_` |
| Console UART | USART3 (APB1) | USART1 (APB2), PA9/PA10 |
| HSE | 8 MHz from ST-LINK, bypass | 25 MHz crystal |
| LEDs / button | PB0, PB7, PB14 / PC13 | PJ13 red, PJ5 green / PA0 |
| Internal memory, vectors | 2 MB / 512 KB, 110 | same |

The full write-up is in
**[mtk3_bsp2/doc/bsp2_stm32f769i_disco_port_en.md](mtk3_bsp2/doc/bsp2_stm32f769i_disco_port_en.md)**. It covers:
- the BSP2 architecture: boot sequence, tick, interrupts and console
- a comparison with Zephyr and FreeRTOS
- every difference from the F767 and the file that handles it
- assumptions still to verify, and open questions

### Using the port in your own STM32CubeIDE project

Follow the BSP2 manual (`mtk3_bsp2/doc/bsp2_stm32_cube_jp.md` §4). Wherever it gives a board-specific value, use the F769I-DISCO value:
- define `_STM32CUBE_DISCOVERY_F769_`
- in CubeMX, enable USART1 on PA9/PA10 with clock source PCLK2
- set *SYS → Timebase Source* to **TIM6**
- call `knl_start_mtkernel()` in `USER CODE BEGIN 2`

## Relationship to upstream

`mtk3_bsp2/` is a copy of [tron-forum/mtk3_bsp2](https://github.com/tron-forum/mtk3_bsp2)
release **v1.00.04** (commit `1ab52cc`). Its files are stored byte-for-byte, CRLF line endings
included, so they can be compared directly with upstream. The port changes on top of that release are:

| File in `mtk3_bsp2/` | Change |
|---|---|
| `include/sys/machine.h` | +3 lines: `_STM32CUBE_DISCOVERY_F769_` board selector |
| `include/sys/sysdepend/stm32_cube/discovery_stm32f769/{machine,profile,sysdef}.h` | new (derived from `nucleo_stm32f767`) |
| `sysdepend/stm32_cube/lib/libtm/discovery_stm32f769/tm_com.c` | new: console on USART1 / PCLK2 |
| `doc/bsp2_stm32f769i_disco_port_en.md` | new: port notes |
| `README.md` | board added to the supported-boards table |
| `.gitmodules` | removed. The `mtkernel` submodule is registered in this repository's `.gitmodules` instead. |

The kernel (`mtk3_bsp2/mtkernel`) is not copied. It is a submodule that points at the official
[tron-forum/mtkernel_3](https://github.com/tron-forum/mtkernel_3) at the commit BSP2 v1.00.04 uses
(`435096c`, v3.00.07). Nothing is committed to the TRON Forum repositories.

To see the changes against upstream:
```bash
git clone https://github.com/tron-forum/mtk3_bsp2.git /tmp/upstream && git -C /tmp/upstream checkout 1ab52cc
git diff --no-index --stat /tmp/upstream mtk3_bsp2   # ignore the .git and mtkernel entries
```

## License

The project code (`application/`, `Makefile`, CI and documentation) is released under the [MIT License](LICENSE).
Bundled and submodule components keep their own licenses:

- **μT-Kernel 3.0 and mtk3_bsp2:** T-License 2.2
- **STM32Cube HAL and the Cube project files:** BSD-3-Clause
- **CMSIS:** Apache-2.0

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for which license applies to which file.

## Acknowledgements

- [TRON Forum](https://www.tron.org) for μT-Kernel 3.0 and mtk3_bsp2
- STMicroelectronics for STM32CubeF7
