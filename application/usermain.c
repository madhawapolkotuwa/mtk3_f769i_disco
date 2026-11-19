/*
 * usermain.c - uT-Kernel 3.0 user program for STM32F769I-DISCOVERY
 *
 * Copyright (c) 2026 Madhawa Polkotuwa
 * SPDX-License-Identifier: MIT
 *
 * usermain() is called by the kernel from the *initial task* (priority 1,
 * see mtkernel/include/sys/inittask.h) once the kernel is up. If usermain()
 * returns, the kernel shuts down, so it must not return in normal operation.
 *
 * APP_STAGE (set from the Makefile, default 4):
 *   3 : Milestone 3 - kernel boots, SysTick tick runs, console prints.
 *   4 : Milestone 4 - one task, one semaphore, one timeout-based wait,
 *       LEDs and console messages.
 *
 * Naming conventions used below (T-Kernel standard):
 *   tk_xxx_yyy  T-Kernel service call: xxx = operation, yyy = object
 *               cre=create  sta=start  del=delete  ext=exit  dly=delay
 *               slp=sleep   wai=wait   sig=signal  get=get   ref=refer
 *               tsk=task    sem=semaphore  tim=system time
 *   T_Cyyy      "packet" (struct) passed to a create call, e.g. T_CTSK, T_CSEM
 *   TA_xxx      object attribute,   TMO_xxx timeout constant,  E_xxx error code
 *   tm_xxx      T-Monitor compatible debug console API (polled, not for RT paths)
 *   ID / ER / INT / UW / UB / TMO / SYSTIM   T-Kernel portable data types
 */
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "main.h"		/* LED / button pin names (STM32Cube) */

#ifndef APP_STAGE
#define APP_STAGE	4
#endif

IMPORT UW knl_sysclk;		/* SYSCLK as read by the BSP at kernel start */

/* Print kernel system time (ms since kernel start). */
LOCAL void print_time(void)
{
	SYSTIM	tim;

	tk_get_tim(&tim);	/* 64-bit time split into hi/lo 32-bit words */
	tm_printf((UB*)"[%8d ms] ", tim.lo);
}

LOCAL void print_banner(void)
{
	tm_putstring((UB*)"uT-Kernel 3.0 BSP2 on STM32F769I-DISCOVERY\n");
	tm_printf((UB*)"SYSCLK = %d Hz, tick = %d ms\n", knl_sysclk, CNF_TIMER_PERIOD);
}

#if APP_STAGE == 3
/* ======================================================================
 * Milestone 3: boot + system tick + console
 * ====================================================================== */
EXPORT INT usermain(void)
{
	INT	n;

	print_banner();

	/* tk_dly_tsk(ms): put the calling task (here the initial task) into the
	 * WAITING state for the given relative time. This only works if the
	 * SysTick-driven system timer is running, so a 1 s cadence on the console
	 * proves the tick is configured correctly. */
	for (n = 0; ; n++) {
		print_time();
		tm_printf((UB*)"alive %d\n", n);
		HAL_GPIO_TogglePin(LD2_GREEN_GPIO_Port, LD2_GREEN_Pin);
		tk_dly_tsk(1000);
	}

	return 0;	/* not reached */
}

#else
/* ======================================================================
 * Milestone 4: one task + one semaphore + one timeout-based wait
 *
 *   usermain (initial task, pri 1) : producer. Signals the semaphore every
 *                                    1700 ms, or when B1 USER is pressed.
 *   led_task (pri 10)              : consumer. Waits on the semaphore with a
 *                                    500 ms timeout.
 *                                      E_OK    -> toggle LD1 red,  "acquired"
 *                                      E_TMOUT -> toggle LD2 green, "timeout"
 * ====================================================================== */

#define LED_TASK_PRI	10		/* 1 = highest ... CNF_MAX_TSKPRI(32) = lowest */
#define WAIT_TMO	500		/* ms - TMO is in milliseconds in uT-Kernel 3.0 */
#define SIGNAL_PERIOD	1700		/* ms */
#define POLL_PERIOD	20		/* ms - button polling */

LOCAL ID	semid;			/* semaphore ID (assigned by the kernel) */
LOCAL ID	led_tskid;		/* task ID (assigned by the kernel) */

LOCAL void led_task(INT stacd, void *exinf);

/* Task creation packet. tk_cre_tsk() reads it; the kernel allocates the
 * stack (stksz bytes) from its system memory because TA_USERBUF is not set. */
LOCAL const T_CTSK ctsk_led = {
	.exinf	= NULL,			/* extended info: free for the user */
	.tskatr	= TA_HLNG | TA_RNG3,	/* written in a high-level language;
					   protection ring 3 (as in the BSP2
					   samples - no MMU/MPU isolation here) */
	.task	= led_task,		/* entry: void task(INT stacd, void *exinf) */
	.itskpri = LED_TASK_PRI,	/* initial priority */
	.stksz	= 1024,			/* stack size in bytes */
};

/* Semaphore creation packet. */
LOCAL const T_CSEM csem = {
	.exinf	= NULL,
	.sematr	= TA_TFIFO | TA_FIRST,	/* waiting tasks queued FIFO; the first
					   task in the queue is served first */
	.isemcnt = 0,			/* initial count: empty */
	.maxsem	= 1,			/* binary semaphore */
};

LOCAL void led_task(INT stacd, void *exinf)
{
	ER	er;
	INT	got = 0, tmo = 0;

	tm_printf((UB*)"led_task: started, stacd=%d\n", stacd);

	for (;;) {
		/* tk_wai_sem(semid, cnt, tmout): take 'cnt' resources, or wait.
		 *   tmout = TMO_POL (0)  : poll, never wait
		 *   tmout = TMO_FEVR(-1) : wait forever
		 *   tmout > 0            : wait at most tmout ms -> E_TMOUT
		 * This is the timeout-based wait required for milestone 4. */
		er = tk_wai_sem(semid, 1, WAIT_TMO);

		if (er == E_OK) {
			got++;
			HAL_GPIO_TogglePin(LD1_RED_GPIO_Port, LD1_RED_Pin);
			print_time();
			tm_printf((UB*)"led_task: semaphore acquired (#%d) -> LD1 red\n", got);
		} else if (er == E_TMOUT) {
			tmo++;
			HAL_GPIO_TogglePin(LD2_GREEN_GPIO_Port, LD2_GREEN_Pin);
			print_time();
			tm_printf((UB*)"led_task: timeout (#%d) -> LD2 green\n", tmo);
		} else {
			tm_printf((UB*)"led_task: tk_wai_sem error %d\n", er);
			break;
		}
	}

	tk_ext_tsk();		/* a task must end with tk_ext_tsk(), never 'return' */
}

EXPORT INT usermain(void)
{
	ER	er;
	INT	elapsed = 0;
	INT	btn, btn_prev = 0;

	print_banner();

	/* tk_cre_sem returns the new semaphore ID (>0) or a negative error code.
	 * In T-Kernel, positive return = ID, negative = ER error code. */
	semid = tk_cre_sem(&csem);
	if (semid < E_OK) {
		tm_printf((UB*)"tk_cre_sem error %d\n", semid);
		return 0;	/* returning from usermain shuts the kernel down */
	}

	/* tk_cre_tsk creates the task in the DORMANT state; it does not run yet. */
	led_tskid = tk_cre_tsk(&ctsk_led);
	if (led_tskid < E_OK) {
		tm_printf((UB*)"tk_cre_tsk error %d\n", led_tskid);
		return 0;
	}

	/* tk_sta_tsk moves it DORMANT -> READY and passes stacd (start code).
	 * It does not run until the higher-priority initial task waits. */
	er = tk_sta_tsk(led_tskid, 0x769);
	if (er < E_OK) {
		tm_printf((UB*)"tk_sta_tsk error %d\n", er);
		return 0;
	}
	tm_printf((UB*)"usermain: semid=%d led_tskid=%d\n", semid, led_tskid);

	/* Producer loop (runs in the initial task). */
	for (;;) {
		tk_dly_tsk(POLL_PERIOD);
		elapsed += POLL_PERIOD;

		btn = (HAL_GPIO_ReadPin(B_USER_GPIO_Port, B_USER_Pin) == GPIO_PIN_SET);
		if ((btn && !btn_prev) || elapsed >= SIGNAL_PERIOD) {
			print_time();
			tm_printf((UB*)"usermain: tk_sig_sem (%s)\n",
				  (btn && !btn_prev) ? "button" : "periodic");
			/* tk_sig_sem(semid, cnt): return 'cnt' resources. If a task
			 * is waiting, it is released (made READY). maxsem is 1, so a
			 * second signal before the consumer takes it gives E_QOVR. */
			er = tk_sig_sem(semid, 1);
			if (er == E_QOVR) {
				tm_putstring((UB*)"usermain: semaphore already full (E_QOVR)\n");
			}
			elapsed = 0;
		}
		btn_prev = btn;
	}

	return 0;	/* not reached */
}
#endif /* APP_STAGE */
