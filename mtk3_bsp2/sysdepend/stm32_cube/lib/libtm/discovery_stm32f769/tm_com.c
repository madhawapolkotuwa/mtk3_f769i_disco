/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.0 BSP 2.0
 *
 *    Copyright (C) 2023-2024 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2024/02.
 *
 *    STM32F769I-DISCOVERY port: derived from libtm/nucleo_stm32f767/tm_com.c
 *----------------------------------------------------------------------
 */

/*
 *    tm_com.c
 *    T-Monitor Communication low-level device driver (STM32F769I-DISCOVERY)
 *
 *    The ST-LINK/V2-1 Virtual COM port of the STM32F769I-DISCOVERY is
 *    connected to USART1 (PA9: USART1_TX, PA10: USART1_RX, AF7).
 *    (On NUCLEO-F767ZI it is USART3 on APB1; USART1 is on APB2.)
 *
 *    As on the other STM32Cube boards, the USART1 bus clock and the GPIO
 *    alternate function must be configured by the STM32Cube generated code
 *    (MX_USART1_UART_Init / HAL_UART_MspInit) before the kernel starts.
 *    This driver only re-programs the USART registers for polled I/O.
 *    The USART1 kernel clock source must be PCLK2 (RCC_DCKCFGR2.USART1SEL
 *    = 00, reset value), since the baud rate is computed from PCLK2.
 */

#include <tk/tkernel.h>

#if USE_TMONITOR
#include <mtkernel/lib/libtm/libtm.h>
#include <sysdepend/stm32_cube/halif.h>

#ifdef MTKBSP_DISCOVERY_STM32F769
#if TM_COM_SERIAL_DEV

/* UART register definition (Use USART1) */
#define	UART_BASE	(0x40011000UL)		/* USART1 register base address */

#define UART_CR1	(*(_UW*)(UART_BASE+0x0000))	/* Control register.1 */
#define UART_CR2	(*(_UW*)(UART_BASE+0x0004))	/* Control register.2 */
#define UART_CR3	(*(_UW*)(UART_BASE+0x0008))	/* Control register.3 */
#define UART_BRR	(*(_UW*)(UART_BASE+0x000C))	/* Baud Rate register */
#define UART_GTPR	(*(_UW*)(UART_BASE+0x0010))	/* GUARD TIME and PRESCALER register */
#define UART_RTOR	(*(_UW*)(UART_BASE+0x0014))	/* RECEIVER TIMEOUT register */
#define UART_RQR	(*(_UW*)(UART_BASE+0x0018))	/* REQUEST register */
#define UART_ISR	(*(_UW*)(UART_BASE+0x001C))	/* INTERRUPT and STATUS register */
#define UART_ICR	(*(_UW*)(UART_BASE+0x0020))	/* INTERRUPT FLAG CLEAR register */
#define UART_RDR	(*(_UW*)(UART_BASE+0x0024))	/* RECEIVE DATA register */
#define UART_TDR	(*(_UW*)(UART_BASE+0x0028))	/* TRANSMIT DATA register */

#define CR1_UE		(0x00000001)			/* Enable UART */
#define CR1_RE		(0x00000004)			/* Enable reception */
#define CR1_TE		(0x00000008)			/* Enable sending */

#define ISR_TXE		(0x00000080)			/* Transmit data register empty */
#define ISR_TC		(0x00000040)			/* Transmission complete */
#define ISR_RXNE	(0x00000020)			/* Read data register not empty */

/* Communication speed */
#define UART_BAUD	(115200)			/* 115200 bps */

EXPORT	void	tm_snd_dat( const UB* buf, INT size )
{
	UB	*b;

	for( b = (UB *)buf; size > 0; size--, b++ ){
		while ((UART_ISR & ISR_TXE) == 0 );
		UART_TDR = *b;
		while ((UART_ISR & ISR_TC) == 0 );
	}
}


EXPORT	void	tm_rcv_dat( UB* buf, INT size )
{
	for( ; size > 0; size--, buf++ ){
		while ( (UART_ISR & ISR_RXNE) == 0 );
		*buf = UART_RDR & 0xff;
	}
}

EXPORT	void	tm_com_init(void)
{
	UW	pclk2;

	/* Initialize serial communication. Disable all interrupt. */
	UART_CR1 = 0;		/* 8bit, Non parity (Reset value) */
	UART_CR2 = 0;		/* Stop bit 1 (Reset value) */
	UART_CR3 = 0;		/* No hard flow control (Reset value) */

	/* Set baud rate (USART1 is clocked from APB2 = PCLK2, oversampling by 16) */
	pclk2 = halif_get_pclk2();
	UART_BRR = (pclk2 + UART_BAUD/2)/UART_BAUD;

	UART_CR1 = CR1_UE | CR1_RE |CR1_TE;	/* Start UART */
}

#endif /* TM_COM_SERIAL_DEV */
#endif /* MTKBSP_DISCOVERY_STM32F769 */
#endif /* USE_TMONITOR */
