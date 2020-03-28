//****************************************************************************
// Product: DPP example, STM32F4-Discovery board, embOS kernel
// Last updated for version 5.9.5
// Last updated on  2017-07-20
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// https://state-machine.com
// <info@state-machine.com>
//****************************************************************************
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

#include "stm32f4xx.h"  // CMSIS-compliant header file for the MCU used
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

// namespace DPP *************************************************************
namespace DPP {

// Local-scope objects -------------------------------------------------------
#define LED_GPIO_PORT     GPIOD
#define LED_GPIO_CLK      RCC_AHB1Periph_GPIOD

#define LED4_PIN          GPIO_Pin_12
#define LED3_PIN          GPIO_Pin_13
#define LED5_PIN          GPIO_Pin_14
#define LED6_PIN          GPIO_Pin_15

#define BTN_GPIO_PORT     GPIOA
#define BTN_GPIO_CLK      RCC_AHB1Periph_GPIOA
#define BTN_B1            GPIO_Pin_0

static uint32_t l_rnd;    // random seed

#ifdef Q_SPY
    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    // event-source identifiers used for tracing
    static uint8_t const l_embos_ticker = 0U;

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        COMMAND_STAT
    };
#endif

extern "C" {

// ISRs used in this project =================================================
#ifdef Q_SPY
/*
* ISR for receiving bytes from the QSPY Back-End
* NOTE: This ISR is "kernel-unaware" meaning that it does not interact with
* the QF/embOS and is not disabled.
*/
void USART2_IRQHandler(void);
void USART2_IRQHandler(void) {
    if ((USART2->SR & USART_SR_RXNE) != 0) {
        uint32_t b = USART2->DR;
        QP::QS::rxPut(b);
    }
}
#else
void USART2_IRQHandler(void);
void USART2_IRQHandler(void) {}
#endif

// embOS application hooks ===================================================
static void tick_handler(void) {  // signature of embOS tick hook routine
    uint32_t tmp;

#ifdef Q_SPY
    tmp = SysTick->CTRL; // clear SysTick_CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif

    // scale down the 1000Hz embOS tick to the desired BSP::TICKS_PER_SEC
    static uint_fast8_t ctr = 1U;
    if (--ctr == 0U) {
        ctr = 1000U / BSP::TICKS_PER_SEC;
        QP::QF::TICK_X(0U, &l_embos_ticker);

        // Perform the debouncing of buttons. The algorithm for debouncing
        // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
        // and Michael Barr, page 71.
        //
        static struct ButtonsDebouncing {
            uint32_t depressed;
            uint32_t previous;
        } buttons = { 0U, 0U }; // state of the button debouncing
        uint32_t current;
        current = BTN_GPIO_PORT->IDR; // read BTN GPIO
        tmp = buttons.depressed; // save the debounced depressed buttons
        buttons.depressed |= (buttons.previous & current); // set depressed
        buttons.depressed &= (buttons.previous | current); // clear released
        buttons.previous   = current; // update the history
        tmp ^= buttons.depressed;     // changed debounced depressed
        if ((tmp & BTN_B1) != 0U) {  // debounced B1 state changed?
            if ((buttons.depressed & BTN_B1) != 0U) { // is B1 depressed?
                static QP::QEvt const pauseEvt = { PAUSE_SIG, 0U, 0U};
                QP::QF::PUBLISH(&pauseEvt, &l_embos_ticker);
            }
            else {            // the button is released
                static QP::QEvt const serveEvt = { SERVE_SIG, 0U, 0U};
                QP::QF::PUBLISH(&serveEvt, &l_embos_ticker);
            }
        }
    }
}
/*..........................................................................*/
/*
*  OS_Idle() function overridden from RTOSInit_STM32F4x_CMSIS.c
*
*  Function description
*    This is basically the "core" of the embOS idle loop.
*    This core loop can be changed, but:
*    The idle loop does not have a stack of its own, therefore no
*    functionality should be implemented that relies on the stack
*    to be preserved. However, a simple program loop can be programmed
*    (like toggling an output or incrementing a counter)
*/
void OS_Idle(void) {
    while (1) {

       /* toggle LED6 on and then off, see NOTE01 */
        QF_INT_DISABLE();
        LED_GPIO_PORT->BSRRL = LED6_PIN; /* turn LED on  */
        __NOP(); /* wait a little to actually see the LED glow */
        __NOP();
        __NOP();
        __NOP();
        LED_GPIO_PORT->BSRRH = LED6_PIN; /* turn LED off */
        QF_INT_ENABLE();

#ifdef Q_SPY
        QP::QS::rxParse();  /* parse all the received bytes */

        if ((USART2->SR & USART_FLAG_TXE) != 0) {  /* is TXE empty? */
            uint16_t b;

            QF_INT_DISABLE();
            b = QP::QS::getByte();
            QF_INT_ENABLE();

            if (b != QP::QS_EOD) {  /* not End-Of-Data? */
                USART2->DR  = (b & 0xFFU);  /* put into the DR register */
            }
        }
#elif defined NDEBUG
        /* Put the CPU and peripherals to the low-power mode.
        * NOTE: You might need to customize the clock management for your
        * application, see the datasheet for your particular Cortex-M3 MCU.
        */
        /* !!!CAUTION!!!
        * The WFI instruction stops the CPU clock, which unfortunately
        * disables the STM32 JTAG port, so the ST-Link debugger can no longer
        * connect to the board. For that reason, the call to __WFI() has to
        * be used with CAUTION. See also NOTE02
        */
#if ((OS_VIEW_IFSELECT != OS_VIEW_IF_JLINK) && (OS_DEBUG == 0))
        //__WFI(); /* Wait-For-Interrupt */
#endif
#endif
    }
}

} // extern "C"

// BSP functions =============================================================
void BSP::init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // Explictily Disable the automatic FPU state preservation as well as
    // the FPU lazy stacking
    //
    FPU->FPCCR &= ~((1U << FPU_FPCCR_ASPEN_Pos) | (1U << FPU_FPCCR_LSPEN_Pos));

    // Initialize thr port for the LEDs
    RCC_AHB1PeriphClockCmd(LED_GPIO_CLK , ENABLE);

    // GPIO Configuration for the LEDs...
    GPIO_InitTypeDef GPIO_struct;
    GPIO_struct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_struct.GPIO_OType = GPIO_OType_PP;
    GPIO_struct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_struct.GPIO_Pin = LED3_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED3_PIN; // turn LED off

    GPIO_struct.GPIO_Pin = LED4_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED4_PIN; // turn LED off

    GPIO_struct.GPIO_Pin = LED5_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED5_PIN; // turn LED off

    GPIO_struct.GPIO_Pin = LED6_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_struct);
    LED_GPIO_PORT->BSRRH = LED6_PIN; // turn LED off

    // Initialize thr port for Button
    RCC_AHB1PeriphClockCmd(BTN_GPIO_CLK , ENABLE);

    // GPIO Configuration for the Button...
    GPIO_struct.GPIO_Pin   = BTN_B1;
    GPIO_struct.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_struct.GPIO_OType = GPIO_OType_PP;
    GPIO_struct.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BTN_GPIO_PORT, &GPIO_struct);

    BSP::randomSeed(1234U);

    if (!QS_INIT((void *)0)) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_embos_ticker);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(COMMAND_STAT);
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    // exercise the FPU with some floating point computations
    float volatile x;
    x = 3.1415926F;
    x = x + 2.7182818F;

    if (stat[0] == 'h') {
        LED_GPIO_PORT->BSRRL = LED3_PIN; // turn LED on
    }
    else {
        LED_GPIO_PORT->BSRRH = LED3_PIN; // turn LED off
    }
    if (stat[0] == 'e') {
        LED_GPIO_PORT->BSRRL = LED5_PIN; // turn LED on
    }
    else {
        LED_GPIO_PORT->BSRRH = LED5_PIN; // turn LED on
    }
    (void)n; // unused parameter (in all but Spy build configuration)

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    if (paused) {
        LED_GPIO_PORT->BSRRL = LED4_PIN; // turn LED on
    }
    else {
        LED_GPIO_PORT->BSRRH = LED4_PIN; // turn LED on
    }
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);

    return l_rnd >> 8;
}
//............................................................................
void BSP::randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
void BSP::terminate(int16_t result) {
    (void)result;
}

} // namespace DPP


// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    static OS_TICK_HOOK tick_hook;
    OS_TICK_AddHook(&tick_hook, &DPP::tick_handler);

#ifdef Q_SPY
    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn); // USART2 interrupt used for QS-RX
#endif
}
//............................................................................
void QF::onCleanup(void) {
}

//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
    NVIC_SystemReset();
}

// QS callbacks ==============================================================
#ifdef Q_SPY
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[2*1024]; // buffer for QS-TX channel
    static uint8_t qsRxBuf[100];  // buffer for QS-RX channel

    initBuf(qsBuf, sizeof(qsBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    GPIO_InitTypeDef GPIO_struct;
    USART_InitTypeDef USART_struct;

    // enable peripheral clock for USART2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // GPIOA clock enable
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // GPIOA Configuration:  USART2 TX on PA2
    GPIO_struct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_struct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_struct.GPIO_OType = GPIO_OType_PP;
    GPIO_struct.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_struct);

    // Connect USART2 pins to AF2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // TX = PA2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // RX = PA3

    USART_struct.USART_BaudRate = 115200;
    USART_struct.USART_WordLength = USART_WordLength_8b;
    USART_struct.USART_StopBits = USART_StopBits_1;
    USART_struct.USART_Parity = USART_Parity_No;
    USART_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_struct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_struct);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // enable RX interrupt
    USART_Cmd(USART2, ENABLE); // enable USART2

    DPP::QS_tickPeriod_ = SystemCoreClock / DPP::BSP::TICKS_PER_SEC;
    DPP::QS_tickTime_ = DPP::QS_tickPeriod_; // to start the timestamp at zero

    // setup the QS filters...
    QS_FILTER_ON(QS_SM_RECORDS); // state machine records */
    QS_FILTER_ON(QS_AO_RECORDS); // active object records */
    QS_FILTER_ON(QS_UA_RECORDS); // all user records */

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) { // not set?
        return DPP::QS_tickTime_ - static_cast<QSTimeCtr>(SysTick->VAL);
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return DPP::QS_tickTime_ + DPP::QS_tickPeriod_
               - static_cast<QSTimeCtr>(SysTick->VAL);
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;

    QF_INT_DISABLE();
    while ((b = getByte()) != QS_EOD) {  // while not End-Of-Data...
        QF_INT_ENABLE();
        while ((USART2->SR & USART_FLAG_TXE) == 0U) { // while TXE not empty
        }
        USART2->DR = (b & 0xFFU); // put into the DR register
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}
//............................................................................
void QS::onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
void QS::onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
    QS_BEGIN(DPP::COMMAND_STAT, (void *)1) //application-specific record begin
        QS_U8(2, cmdId);
        QS_U32(8, param1);
        QS_U32(8, param2);
        QS_U32(8, param3);
    QS_END()

    if (cmdId == 10U) {
        //Q_ERROR();
    }
    else if (cmdId == 11U) {
        //Q_ERROR();
    }
}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//****************************************************************************
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
