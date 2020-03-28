/// @file
/// @brief QF/C++ port to PIC32, preemptive QK kernel, XC32 toolchain
/// @ingroup qep
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

// The maximum number of system clock tick rates
#define QF_MAX_TICK_RATE        2U

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE           32U

// QF interrupt disable/enable, see NOTE2
#define QF_INT_DISABLE()        __asm__ volatile("DI")
#define QF_INT_ENABLE()         __asm__ volatile("EI")

// QF critical section entry/exit, see NOTE2
// QF_CRIT_STAT_TYPE not defined: "unconditional interrupt disabling" policy
#define QF_CRIT_ENTRY(dummy)    QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)     QF_INT_ENABLE()

// fast log-base-2 with CLZ instruction
#define QF_LOG2(n_) (static_cast<std::uint8_t>(32U - _clz(n_)))

#include <xc.h>       // for _clz()

#include "qep_port.hpp" // QEP port
#include "qk_port.hpp"  // QK preemptive kernel port
#include "qf.hpp"       // QF platform-independent public interface

//////////////////////////////////////////////////////////////////////////////
// NOTE1:
// The maximum number of active objects QF_MAX_ACTIVE can be increased
// up to 64, if necessary. Here it is set to a lower level to save some RAM.
//
// NOTE2:
// The DI/EI instructions are used for fast, unconditional disabling and
// enabling of interrupts.
//
// CAUTION: This QP port assumes that interrupt nesting is _enabled_,
// which is the default in the PIC32 processors. Interrupt nesting should
// never be disabled by setting the NSTDIS control bit (INTCON1<15>). If you
// don't want interrupts to nest, you can always prioritize them at the same
// level. For example, the default priority level for all interrupts is 4 out
// of reset. If you don't change this level for any interrupt the nesting of
// interrupt will not occur.
//

#endif // QF_PORT_HPP

