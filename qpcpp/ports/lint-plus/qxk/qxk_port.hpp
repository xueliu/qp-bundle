/// @file
/// @brief QXK/C++ port example, Generic C++ compiler
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-01-16
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

#ifndef QXK_PORT_HPP
#define QXK_PORT_HPP

//lint -save -e1960    MISRA-C++:2008 Rule 7-3-1, Global declaration

//****************************************************************************
//! determination if the code executes in the ISR context
//! (used internally in QXK only)
#define QXK_ISR_CONTEXT_() (getSR() != 0U)

//! trigger context switch (used internally in QXK only)
#define QXK_CONTEXT_SWITCH_() (trigSWI())

//****************************************************************************
//! activate the context-switch callback
#define QXK_ON_CONTEXT_SW 1

//****************************************************************************
// QXK interrupt entry and exit

//! Define the ISR entry sequence, if the compiler supports writing
//! interrupts in C++.
/// @note This is just an example of #QK_ISR_ENTRY. You need to define
/// the macro appropriately for the CPU/compiler you're using. Also, some
/// QK ports will not define this macro, but instead will provide ISR
/// skeleton code in assembly.
#define QXK_ISR_ENTRY() do { \
    ++QXK_attr_.intNest;     \
} while (false)


//! Define the ISR exit sequence, if the compiler supports writing
//! interrupts in C++.
/// @note This is just an example of #QK_ISR_EXIT. You need to define
/// the macro appropriately for the CPU/compiler you're using. Also, some
/// QK ports will not define this macro, but instead will provide ISR
/// skeleton code in assembly.
#define QXK_ISR_EXIT() do {        \
    --QXK_attr_.intNest;           \
    if (QXK_attr_.intNest == 0U) { \
        if (QXK_sched_() != 0U) {  \
            QXK_activate_();       \
        }                          \
    }                              \
    else {                         \
        Q_ERROR();                 \
    }                              \
} while (false)

extern "C" {

std::uint32_t getSR(void);
void trigSWI(void);

} // extern "C"

//lint -restore

#include "qxk.hpp" // QXK platform-independent public interface

#endif // QXK_PORT_HPP
