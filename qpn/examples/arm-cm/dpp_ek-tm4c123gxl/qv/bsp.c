/*****************************************************************************
* Product: DPP on EK-TM4C123GXL board, cooperative QV kernel
* Last Updated for Version: 5.8.0
* Date of the Last Update:  2016-11-06
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses/>.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
*****************************************************************************/
#include "qpn.h"
#include "dpp.h"
#include "bsp.h"

#include "TM4C123GH6PM.h"        /* the device specific header (TI) */
#include "rom.h"                 /* the built-in ROM functions (TI) */
#include "sysctl.h"              /* system control driver (TI) */
#include "gpio.h"                /* GPIO driver (TI) */
/* add other drivers if necessary... */

//Q_DEFINE_THIS_FILE

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
* DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
*/
enum KernelUnawareISRs { /* see NOTE00 */
    /* ... */
    MAX_KERNEL_UNAWARE_CMSIS_PRI  /* keep always last */
};
/* "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts */
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI <= QF_AWARE_ISR_CMSIS_PRI);

enum KernelAwareISRs {
    GPIOA_PRIO = QF_AWARE_ISR_CMSIS_PRI, /* see NOTE00 */
    SYSTICK_PRIO,
    /* ... */
    MAX_KERNEL_AWARE_CMSIS_PRI /* keep always last */
};
/* "kernel-aware" interrupts should not overlap the PendSV priority */
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));

/* ISRs defined in this BSP ------------------------------------------------*/
void SysTick_Handler(void);
void GPIOPortA_IRQHandler(void);

/* Local-scope objects -----------------------------------------------------*/
#define LED_RED     (1U << 1)
#define LED_GREEN   (1U << 3)
#define LED_BLUE    (1U << 2)

#define BTN_SW1     (1U << 4)
#define BTN_SW2     (1U << 0)

/* random seed */
static uint32_t l_rnd;


/* ISRs used in this project ===============================================*/
void SysTick_Handler(void) {
    /* state of the button debouncing, see below */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { ~0U, ~0U };
    uint32_t current;
    uint32_t tmp;

    QF_tickXISR(0U); /* process time events for rate 0 */


    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    current = ~GPIOF->DATA_Bits[BTN_SW1 | BTN_SW2]; /* read SW1 and SW2 */
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */
    if ((tmp & BTN_SW1) != 0U) {  /* debounced SW1 state changed? */
        if ((buttons.depressed & BTN_SW1) != 0U) { /* is SW1 depressed? */
            QACTIVE_POST_ISR((QActive *)&AO_Table, PAUSE_SIG, 0U);
        }
        else {            /* the button is released */
            QACTIVE_POST_ISR((QActive *)&AO_Table, SERVE_SIG, 0U);
        }
    }
}
/*..........................................................................*/
void GPIOPortA_IRQHandler(void) {
    QACTIVE_POST_ISR((QActive *)&AO_Table, MAX_SIG, 0U); /* for testing */
}


/* BSP functions ===========================================================*/
void BSP_init(void) {
    /* NOTE: SystemInit() already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();

    /*
    * Do NOT to use the automatic FPU state preservation and
    * do NOT to use the FPU lazy stacking.
    *
    * NOTE:
    * Use the following setting when FPU is used at the task-level only and
    * NOT in any ISR. This setting is very efficient, but if ISRs start using
    * the FPU, this can lead to corruption of the FPU registers.
    */
    FPU->FPCCR &= ~((1U << FPU_FPCCR_ASPEN_Pos)
                   | (1U << FPU_FPCCR_LSPEN_Pos));

    /* enable clock for to the peripherals used by this application... */
    SYSCTL->RCGCGPIO |= (1U << 5); /* enable Run mode for GPIOF */

    /* configure the LEDs and push buttons */
    GPIOF->DIR |= (LED_RED | LED_GREEN | LED_BLUE);/* set direction: output */
    GPIOF->DEN |= (LED_RED | LED_GREEN | LED_BLUE); /* digital enable */
    GPIOF->DATA_Bits[LED_RED]   = 0U; /* turn the LED off */
    GPIOF->DATA_Bits[LED_GREEN] = 0U; /* turn the LED off */
    GPIOF->DATA_Bits[LED_BLUE]  = 0U; /* turn the LED off */

    /* configure the Buttons */
    GPIOF->DIR &= ~(BTN_SW1 | BTN_SW2); /*  set direction: input */
    ROM_GPIOPadConfigSet(GPIOF_BASE, (BTN_SW1 | BTN_SW2),
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    BSP_randomSeed(1234U);
}
/*..........................................................................*/
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    GPIOF->DATA_Bits[LED_RED]   = ((stat[0] == 'h') ? LED_RED   : 0U);
    GPIOF->DATA_Bits[LED_GREEN] = ((stat[0] == 'e') ? LED_GREEN : 0U);
}
/*..........................................................................*/
void BSP_displayPaused(uint8_t paused) {
    GPIOF->DATA_Bits[LED_RED] = ((paused != 0U) ? LED_RED : 0U);
}
/*..........................................................................*/
uint32_t BSP_random(void) { /* a very cheap pseudo-random-number generator */
    /* The flating point code is to exercise the FPU... */
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;

    /* "Super-Duper" Linear Congruential Generator (LCG)
    * LCG(2^32, 3*7*11*13*23, 0, seed)
    */
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);

    return l_rnd >> 8;
}
/*..........................................................................*/
void BSP_randomSeed(uint32_t seed) {
    l_rnd = seed;
}
/*..........................................................................*/
void BSP_terminate(int16_t result) {
    (void)result;
}


/* QF callbacks ============================================================*/
void QF_onStartup(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* assing all priority bits for preemption-prio. and none to sub-prio. */
    NVIC_SetPriorityGrouping(0U);

    /* set priorities of ALL ISRs used in the system, see NOTE00
    *
    * !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    * Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    * DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    */
    NVIC_SetPriority(SysTick_IRQn,   SYSTICK_PRIO);
    NVIC_SetPriority(GPIOA_IRQn,     GPIOA_PRIO);
    /* ... */

    /* enable IRQs... */
    NVIC_EnableIRQ(GPIOA_IRQn);
}
/*..........................................................................*/
void QV_onIdle(void) {  /* called with interrupts disabled, see NOTE01 */
    /* toggle the User LED on and then off, see NOTE02 */
    GPIOF->DATA_Bits[LED_BLUE] = LED_BLUE;  /* turn the Blue LED on  */
    GPIOF->DATA_Bits[LED_BLUE] = 0U;        /* turn the Blue LED off */

#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M MCU.
    */
    QV_CPU_SLEEP();  /* atomically go to sleep and enable interrupts */
#else
    QF_INT_ENABLE(); /* just enable interrupts */
#endif
}

/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const module, int loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    (void)module;
    (void)loc;

    NVIC_SystemReset();
}

/*****************************************************************************
* NOTE00:
* The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
* ISR priority that is disabled by the QF framework. The value is suitable
* for the NVIC_SetPriority() CMSIS function.
*
* Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
* with the numerical values of priorities equal or higher than
* QF_AWARE_ISR_CMSIS_PRI) are allowed to call any QF services. These ISRs
* are "QF-aware".
*
* Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
* level (i.e., with the numerical values of priorities less than
* QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
* Such "QF-unaware" ISRs cannot call any QF services. The only mechanism
* by which a "QF-unaware" ISR can communicate with the QF framework is by
* triggering a "QF-aware" ISR, which can post/publish events.
*
* NOTE01:
* The QV_onIdle() callback is called with interrupts disabled, because the
* determination of the idle condition might change by any interrupt posting
* an event. QV_onIdle() must internally enable interrupts, ideally
* atomically with putting the CPU to the power-saving mode.
*
* NOTE02:
* The User LED is used to visualize the idle loop activity. The brightness
* of the LED is proportional to the frequency of invcations of the idle loop.
* Please note that the LED is toggled with interrupts locked, so no interrupt
* execution time contributes to the brightness of the User LED.
*/
