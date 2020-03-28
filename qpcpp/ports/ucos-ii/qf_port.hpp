/// @file
/// @brief QF/C++ port to uC/OS-II (V2.92) kernel, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-01-20
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

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// uC/OS-II event queue and thread types
#define QF_EQUEUE_TYPE       OS_EVENT *
#define QF_THREAD_TYPE       uint32_t

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE ((OS_LOWEST_PRIO - 1 < 64) ? (OS_LOWEST_PRIO - 1) : 64U)

// uC/OS-II critical section operations (critical section type 3), NOTE1
#define QF_CRIT_STAT_TYPE    OS_CPU_SR
#define QF_CRIT_ENTRY(stat_) ((stat_) = OS_CPU_SR_Save())
#define QF_CRIT_EXIT(stat_)  OS_CPU_SR_Restore(stat_)

#include "ucos_ii.h"   // uC/OS-II API

#include "qep_port.hpp"  // QEP port, includes the master uC/OS-II include
#include "qequeue.hpp"   // used for event deferral
#include "qpset.hpp"     // this QP port uses the native QP priority set
#include "qf.hpp"        // QF platform-independent public interface


//****************************************************************************
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL

    // uC/OS-II-specific scheduler locking, see NOTE2
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) do { \
        if (OSIntNesting == 0U) {      \
            OSSchedLock();             \
        } \
    } while (false)
    #define QF_SCHED_UNLOCK_() do { \
        if (OSIntNesting == 0U) {   \
            OSSchedUnlock();        \
        } \
    } while (false)


    // uC/OS-II event pool operations...
    #define QF_EPOOL_TYPE_ OS_MEM*
    #define QF_EPOOL_INIT_(pool_, poolSto_, poolSize_, evtSize_) do {       \
        INT8U err;                                                          \
        (pool_) = OSMemCreate((poolSto_), (INT32U)((poolSize_)/(evtSize_)), \
                              (INT32U)(evtSize_), &err);                    \
        Q_ASSERT_ID(105, err == OS_ERR_NONE);                               \
    } while (false)

    #define QF_EPOOL_EVENT_SIZE_(pool_) ((pool_)->OSMemBlkSize)
    #define QF_EPOOL_GET_(pool_, e_, m_) do {       \
        QF_CRIT_STAT_                               \
        QF_CRIT_ENTRY_();                           \
        if ((pool_)->OSMemNFree > (m_)) {           \
            INT8U err;                              \
            (e_) = (QEvt *)OSMemGet((pool_), &err); \
            Q_ASSERT_ID(205, err == OS_ERR_NONE);   \
        }                                           \
        else {                                      \
            (e_) = nullptr;          \
        }                                           \
        QF_CRIT_EXIT_();                            \
    } while (false)

    #define QF_EPOOL_PUT_(pool_, e_) \
        Q_ALLEGE_ID(305, OSMemPut((pool_), (e_)) == OS_ERR_NONE)

#endif // ifdef QP_IMPL
//****************************************************************************
// NOTE1:
// The uC/OS-II critical section must be able to nest.
//
// NOTE2:
// uC/OS-II provides only global scheduler locking for all thread priorities
// by means of OSSchedLock() and OSSchedUnlock(). Therefore, locking the
// scheduler only up to the specified lock priority is not supported.
//

#endif // QF_PORT_HPP
