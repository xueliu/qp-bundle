/// @file
/// @brief QK/C++ preemptive kernel core functions
/// @ingroup qk
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-03-23
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#define QP_IMPL             // this is QF/QK implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope internal interface
#include "qassert.h"        // QP assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QK_HPP
    #error "Source file included in a project NOT based on the QK kernel"
#endif // QK_HPP

// Public-scope objects ******************************************************
extern "C" {

Q_DEFINE_THIS_MODULE("qk")

QK_Attr QK_attr_; // global attributes of the QK kernel

} // extern "C"


namespace QP {

//****************************************************************************
/// @description
/// Initializes QF and must be called exactly once before any other QF
/// function. Typcially, QP::QF::init() is called from main() even before
/// initializing the Board Support Package (BSP).
///
/// @note
/// QP::QF::init() clears the internal QF variables, so that the framework
/// can start correctly even if the startup code fails to clear the
/// uninitialized data (as is required by the C Standard).
///
void QF::init(void) {
    QF_maxPool_      = 0U;
    QF_subscrList_   = nullptr;
    QF_maxPubSignal_ = 0;

    bzero(&QF::timeEvtHead_[0], sizeof(QF::timeEvtHead_));
    bzero(&active_[0], sizeof(active_));
    bzero(&QK_attr_,   sizeof(QK_attr_));

    QK_attr_.actPrio  = 0U; // prio of QK idle loop
    QK_attr_.lockPrio = QF_MAX_ACTIVE; // locked

#ifdef QK_INIT
    QK_INIT(); // port-specific initialization of the QK kernel
#endif
}

//****************************************************************************
/// @description
/// This function stops the QF application. After calling this function,
/// QF attempts to gracefully stop the application. This graceful shutdown
/// might take some time to complete. The typical use of this function is
/// for terminating the QF application to return back to the operating
/// system or for handling fatal errors that require shutting down
/// (and possibly re-setting) the system.
///
/// @attention
/// After calling QF::stop() the application must terminate and cannot
/// continue. In particular, QF::stop() is **not** intended to be followed
/// by a call to QF::init() to "resurrect" the application.
///
/// @sa QP::QF::onCleanup()
///
void QF::stop(void) {
    QF::onCleanup();  // cleanup callback
    // nothing else to do for the QK preemptive kernel
}

//****************************************************************************
//! process all events posted during initialization */
static void initial_events(void); // prototype
static void initial_events(void) {
    QK_attr_.lockPrio = 0U; // scheduler unlocked

    // any active objects need to be scheduled before starting event loop?
    if (QK_sched_() != 0U) {
        QK_activate_(); // activate AOs to process all events posted so far
    }
}

//****************************************************************************
/// @description
///
/// QP::QF::run() is typically called from your startup code after you
/// initialize the QF and start at least one active object with
/// QP::QActive::start().
///
/// @returns In QK, the QP::QF::run() function does not return.
///
int_t QF::run(void) {
    QF_INT_DISABLE();
    initial_events(); // process all events posted during initialization
    onStartup();      // application-specific startup callback
    QF_INT_ENABLE();

    // the QK idle loop...
    for (;;) {
        QK::onIdle(); // application-specific QK on-idle callback
    }
#ifdef __GNUC__  // GNU compiler?
    return 0;
#endif
}

//****************************************************************************
// @description
// Starts execution of the AO and registers the AO with the framework.
//
// @param[in] prio    priority at which to start the active object
// @param[in] qSto    pointer to the storage for the ring buffer of the
//                    event queue (used only with the built-in QP::QEQueue)
// @param[in] qLen    length of the event queue [events]
// @param[in] stkSto  pointer to the stack storage (must be NULL in QK)
// @param[in] stkSize stack size [bytes]
// @param[in] par     pointer to an extra parameter (might be NULL)
//
// @usage
// The following example shows starting an AO when a per-task stack is needed:
// @include
// qf_start.cpp
//
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    (void)stkSize; // unused paramteter in the QK port

    /// @pre AO cannot be started from an ISR, the priority must be in range
    /// and the stack storage must not be provided, because the QK kernel does
    /// not need per-AO stacks.
    Q_REQUIRE_ID(300, (!QK_ISR_CONTEXT_())
        && (0U < prio) && (prio <= QF_MAX_ACTIVE)
        && (stkSto == nullptr));

    m_eQueue.init(qSto, qLen); // initialize the built-in queue

    m_prio = static_cast<std::uint8_t>(prio);  // set the QF prio of this AO
    QF::add_(this); // make QF aware of this AO

    this->init(par); // take the top-most initial tran. (virtual)
    QS_FLUSH(); // flush the trace buffer to the host

    // See if this AO needs to be scheduled in case QK is already running
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    if (QK_sched_() != 0U) { // activation needed?
        QK_activate_();
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
///
/// @description
/// This function locks the QK scheduler to the specified ceiling.
///
/// @param[in]   ceiling    priority ceiling to which the QK scheduler
///                         needs to be locked
///
/// @returns
/// The previous QK Scheduler lock status, which is to be used to unlock
/// the scheduler by restoring its previous lock status in
/// QP::QK::schedUnlock().
///
/// @note
/// QP::QK::schedLock() must be always followed by the corresponding
/// QP::QK::schedUnlock().
///
/// @sa QK_schedUnlock()
///
/// @usage
/// The following example shows how to lock and unlock the QK scheduler:
/// @include qk_lock.cpp
///
QSchedStatus QK::schedLock(std::uint_fast8_t const ceiling) noexcept {
    QSchedStatus stat;
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    /// @pre The QK scheduler lock:
    /// - cannot be called from an ISR;
    Q_REQUIRE_ID(600, !QK_ISR_CONTEXT_());

    // first store the previous lock prio if it is below the ceiling
    if (static_cast<std::uint_fast8_t>(QK_attr_.lockPrio) < ceiling) {
        stat = (static_cast<QSchedStatus>(QK_attr_.lockPrio) << 8);
        QK_attr_.lockPrio = static_cast<std::uint8_t>(ceiling);

        QS_BEGIN_NOCRIT_PRE_(QS_SCHED_LOCK, nullptr, nullptr)
            QS_TIME_PRE_(); // timestamp
            // prev and new lock prio...
            QS_2U8_PRE_(stat, QK_attr_.lockPrio);
        QS_END_NOCRIT_PRE_()

        // add the previous lock holder priority
        stat |= static_cast<QSchedStatus>(QK_attr_.lockHolder);

        QK_attr_.lockHolder = QK_attr_.actPrio;
    }
    else {
       stat = 0xFFU;
    }
    QF_CRIT_EXIT_();

    return stat; // return the status to be saved in a stack variable
}

//****************************************************************************
///
/// @description
/// This function unlocks the QK scheduler to the previous status.
///
/// @param[in]   stat       previous QK Scheduler lock status returned from
///                         QP::QK::schedLock()
/// @note
/// QP::QK::schedUnlock() must always follow the corresponding
/// QP::QK::schedLock().
///
/// @sa QP::QK::schedLock()
///
/// @usage
/// The following example shows how to lock and unlock the QK scheduler:
/// @include qk_lock.cpp
///
void QK::schedUnlock(QSchedStatus const stat) noexcept {
    // has the scheduler been actually locked by the last QK_schedLock()?
    if (stat != 0xFFU) {
        std::uint_fast8_t const lockPrio =
            static_cast<std::uint_fast8_t>(QK_attr_.lockPrio);
        std::uint_fast8_t const prevPrio =
            static_cast<std::uint_fast8_t>(stat >> 8);
        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();

        /// @pre The scheduler cannot be unlocked:
        /// - from the ISR context; and
        /// - the current lock priority must be greater than the previous
        Q_REQUIRE_ID(700, (!QK_ISR_CONTEXT_())
                          && (lockPrio > prevPrio));

        QS_BEGIN_NOCRIT_PRE_(QS_SCHED_UNLOCK, nullptr, nullptr)
            QS_TIME_PRE_(); // timestamp
            QS_2U8_PRE_(lockPrio, prevPrio); //before & after
        QS_END_NOCRIT_PRE_()

        // restore the previous lock priority and lock holder
        QK_attr_.lockPrio   = static_cast<std::uint8_t>(prevPrio);
        QK_attr_.lockHolder = static_cast<std::uint8_t>(stat & 0xFFU);

        // find the highest-prio thread ready to run
        if (QK_sched_() != 0U) { // priority found?
            QK_activate_(); // activate any unlocked basic threads
        }

        QF_CRIT_EXIT_();
    }
}

} // namespace QP

//============================================================================
extern "C" {

//****************************************************************************
/// @description
/// The QK scheduler finds out the priority of the highest-priority AO
/// that (1) has events to process and (2) has priority that is above the
/// current priority.
///
/// @returns the 1-based priority of the the active object, or zero if
/// no eligible active object is ready to run.
///
/// @attention
/// QK_sched_() must be always called with interrupts **disabled** and
/// returns with interrupts **disabled**.
///
std::uint_fast8_t QK_sched_(void) noexcept {
    // find the highest-prio AO with non-empty event queue
    std::uint_fast8_t p = QK_attr_.readySet.findMax();

    // is the highest-prio below the active prio?
    if (p <= static_cast<std::uint_fast8_t>(QK_attr_.actPrio)) {
        p = 0U; // active object not eligible
    }
    else if (p <= static_cast<std::uint_fast8_t>(QK_attr_.lockPrio)) {
        p = 0U; // active object not eligible
    }
    else {
        Q_ASSERT_ID(410, p <= QF_MAX_ACTIVE);
        QK_attr_.nextPrio = static_cast<std::uint8_t>(p); // next AO to run
    }
    return p;
}

//****************************************************************************
/// @description
/// QK_activate_() activates ready-to run AOs that are above the initial
/// active priority (QK_attr_.actPrio).
///
/// @note
/// The activator might enable interrupts internally, but always returns with
/// interrupts **disabled**.
///
void QK_activate_(void) noexcept {
    std::uint_fast8_t const pin =
        static_cast<std::uint_fast8_t>(QK_attr_.actPrio);
    std::uint_fast8_t p = static_cast<std::uint_fast8_t>(QK_attr_.nextPrio);
    QP::QActive *a;

    // QK_attr_.actPrio and QK_attr_.nextPrio must be in ragne
    Q_REQUIRE_ID(500,
        (pin <= QF_MAX_ACTIVE)
        && (0U < p) && (p <= QF_MAX_ACTIVE));

    // QK Context switch callback defined or QS tracing enabled?
#if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)
    std::uint_fast8_t pprev = pin;
#endif // QK_ON_CONTEXT_SW || Q_SPY

    QK_attr_.nextPrio = 0U; // clear for the next time

    // loop until no more ready-to-run AOs of higher prio than the initial
    do {
        a = QP::QF::active_[p]; // obtain the pointer to the AO
        QK_attr_.actPrio = static_cast<std::uint8_t>(p); // new active prio

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_NEXT,
                             QP::QS::priv_.locFilter[QP::QS::AO_OBJ], a)
            QS_TIME_PRE_();        // timestamp
            QS_2U8_PRE_(p, pprev); // sechduled prio & previous prio...
        QS_END_NOCRIT_PRE_()

#if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)
        if (p != pprev) {  // changing threads?

#ifdef QK_ON_CONTEXT_SW
            // context-switch callback
            QK_onContextSw(((pprev != 0U)
                           ? QP::QF::active_[pprev]
                           : nullptr), a);
#endif // QK_ON_CONTEXT_SW

            pprev = p; // update previous priority
        }
#endif // QK_ON_CONTEXT_SW || Q_SPY

        QF_INT_ENABLE();  // unconditionally enable interrupts

        // perform the run-to-completion (RTS) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and QActive_get_() asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QP::QEvt const * const e = a->get_();
        a->dispatch(e);
        QP::QF::gc(e);

        // determine the next highest-priority AO ready to run...
        QF_INT_DISABLE();

        if (a->m_eQueue.isEmpty()) { // empty queue?
            QK_attr_.readySet.rmove(p);
        }

        // find new highest-prio AO ready to run...
        p = QK_attr_.readySet.findMax();

        // is the new priority below the initial preemption threshold?
        if (p <= pin) {
            p = 0U; // active object not eligible
        }
        else if (p <= static_cast<std::uint_fast8_t>(QK_attr_.lockPrio)) {
            p = 0U; // active object not eligible
        }
        else {
            Q_ASSERT_ID(510, p <= QF_MAX_ACTIVE);
        }
    } while (p != 0U);

    QK_attr_.actPrio = static_cast<std::uint8_t>(pin); // restore the prio

#if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)

    if (pin != 0U) { // resuming an active object?
        a = QP::QF::active_[pin]; // the pointer to the preempted AO

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_RESUME,
                             QP::QS::priv_.locFilter[QP::QS::AO_OBJ], a)
            QS_TIME_PRE_(); // timestamp
            QS_2U8_PRE_(pin, pprev); // resumed prio & previous prio...
        QS_END_NOCRIT_PRE_()
    }
    else {  // resuming priority==0 --> idle
        a = nullptr; // QK idle loop

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_IDLE, nullptr, nullptr)
            QS_TIME_PRE_();    // timestamp
            QS_U8_PRE_(pprev); // previous prio
        QS_END_NOCRIT_PRE_()
    }

#ifdef QK_ON_CONTEXT_SW
    QK_onContextSw(QP::QF::active_[pprev], a); // context-switch callback
#endif // QK_ON_CONTEXT_SW

#endif // QK_ON_CONTEXT_SW || Q_SPY
}

} // extern "C"

