/// @file
/// @brief QF/C++ port to ThreadX kernel, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 6.6.0
/// Last updated on  2019-07-30
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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

// ThreadX event queue and thread types
#define QF_EQUEUE_TYPE       TX_QUEUE
#define QF_THREAD_TYPE       TX_THREAD
#define QF_OS_OBJECT_TYPE    bool

// QF priority offset within ThreadX priority numbering scheme, see NOTE1
#define QF_TX_PRIO_OFFSET    8U

// The maximum number of active objects in the application, see NOTE2
#define QF_MAX_ACTIVE        (31U - QF_TX_PRIO_OFFSET)

// QF critical section for ThreadX, see NOTE3
#define QF_CRIT_STAT_TYPE    UINT
#define QF_CRIT_ENTRY(stat_) ((stat_) = tx_interrupt_control(TX_INT_DISABLE))
#define QF_CRIT_EXIT(stat_)  ((void)tx_interrupt_control(stat_))

#include "tx_api.h"    // ThreadX API

#include "qep_port.hpp"  // QEP port
#include "qequeue.hpp"   // used for event deferral
#include "qpset.hpp"     // used for publish/subscribe
#include "qf.hpp"        // QF platform-independent public interface


//////////////////////////////////////////////////////////////////////////////
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL

    // ThreadX-specific scheduler locking (implemented in qf_port.cpp)
    #define QF_SCHED_STAT_ QFSchedLock lockStat_;
    #define QF_SCHED_LOCK_(prio_) do {       \
        if (_tx_thread_system_state != 0U) { \
            lockStat_.m_lockPrio = 0U;       \
        } else {                             \
            lockStat_.lock((prio_));         \
        } \
    } while (false)
    #define QF_SCHED_UNLOCK_() do {       \
        if (lockStat_.m_lockPrio != 0U) { \
            lockStat_.unlock();           \
        }                                 \
    } while (false)

    namespace QP {
        struct QFSchedLock {
            uint_fast8_t m_lockPrio; //!< lock prio [QF numbering scheme]
            UINT m_prevThre;         //!< previoius preemption threshold
            TX_THREAD *m_lockHolder; //!< the thread holding the lock

            void lock(uint_fast8_t prio);
            void unlock(void) const;
        };
    } // namespace QP
    extern "C" {
        extern UINT _tx_thread_system_state; // internal TX interrupt counter
    }

    // TreadX block pool operations...
    #define QF_EPOOL_TYPE_   TX_BLOCK_POOL
    #define QF_EPOOL_INIT_(pool_, poolSto_, poolSize_, evtSize_) \
        Q_ALLEGE(tx_block_pool_create(&(pool_), \
                 const_cast<CHAR *>("QP"), (evtSize_), \
                 (poolSto_), (poolSize_)) == TX_SUCCESS)

    #define QF_EPOOL_EVENT_SIZE_(pool_) \
        (static_cast<std::uint_fast16_t>((pool_).tx_block_pool_block_size))

    #define QF_EPOOL_GET_(pool_, e_, margin_) do { \
        QF_CRIT_STAT_ \
        QF_CRIT_ENTRY_(); \
        if ((pool_).tx_block_pool_available > (margin_)) { \
            Q_ALLEGE(tx_block_allocate(&(pool_), \
                     reinterpret_cast<VOID **>(&(e_)), TX_NO_WAIT) \
                     == TX_SUCCESS); \
        } \
        else { \
            (e_) = nullptr; \
        } \
        QF_CRIT_EXIT_(); \
    } while (false)

    #define QF_EPOOL_PUT_(dummy, e_) \
        Q_ALLEGE(tx_block_release(static_cast<VOID *>(e_)) == TX_SUCCESS)

#endif // ifdef QP_IMPL


//////////////////////////////////////////////////////////////////////////////
// NOTE1:
// QF_TX_PRIO_OFFSET specifies the number of highest-urgency ThreadX
// priorities not available to QP active objects. These highest-urgency
// priorities might be used by ThreadX threads that run "above" QP active
// objects.
//
// Because the ThreadX priority numbering is "upside down" compared
// to the QP priority numbering, the ThreadX priority for an active object
// thread is calculated as follows:
//     tx_prio = QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - qp_prio
//
// NOTE2:
// The maximum number of active objects in QP can be increased to 63,
// inclusive, but it can be reduced to save some memory. Also, the number of
// active objects cannot exceed the number of TreadX thread priorities
// TX_MAX_PRIORITIES, because each QP active object requires a unique
// priority level. Due to the artificial limitations of the ThreadX demo
// for Windows, QF_MAX_ACTIVE is set here lower to not exceed the total
// available priority levels.
//
// NOTE3:
// The ThreadX critical section must be able to nest, which is the case with
// the tx_interrupt_control() API.

#endif // QF_PORT_HPP
