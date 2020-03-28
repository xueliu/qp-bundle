//////////////////////////////////////////////////////////////////////////////
// Product: DPP example for ThreadX
// Last updated for version 6.3.7
// Last updated on  2018-12-17
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//////////////////////////////////////////////////////////////////////////////
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

// Local-scope objects -------------------------------------------------------
static QP::QEvt const *l_tableQueueSto[N_PHILO];
static QP::QEvt const *l_philoQueueSto[N_PHILO][N_PHILO];
static QP::QSubscrList l_subscrSto[DPP::MAX_PUB_SIG];

static union SmallEvents {
    void   *e0;  // minimum event size
    uint8_t e1[sizeof(DPP::TableEvt)];
    // ... other event types to go into this pool
} l_smlPoolSto[2*N_PHILO + 10]; // storage for the small event pool

static ULONG l_philoStk[N_PHILO][256]; // stacks for the Philosophers
static ULONG l_tableStk[256];          // stack for the Table

//............................................................................
int main() {
    DPP::BSP::init();   // initialize the Board Support Package
    tx_kernel_enter();  // transfet control to the ThreadX RTOS
    return 0; // tx_kernel_enter() does not return
}
//............................................................................
void tx_application_define(void * /*first_unused_memory*/) {
    // initialize the framework and the underlying RT kernel...
    QP::QF::init();

    // init publish-subscribe
    QP::QF::psInit(l_subscrSto, Q_DIM(l_subscrSto));

    // initialize event pools...
    QP::QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto),
                     sizeof(l_smlPoolSto[0]));

    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_tableQueueSto);
    QS_OBJ_DICTIONARY(l_philoQueueSto[0]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[1]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[2]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[3]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[4]);

    // start the active objects...
    for (uint8_t n = 0; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start(
            static_cast<uint_fast8_t>(n + 1),
            l_philoQueueSto[n], Q_DIM(l_philoQueueSto[n]),
            l_philoStk[n], sizeof(l_philoStk[n]));
    }
    DPP::AO_Table->start(
        static_cast<uint_fast8_t>(N_PHILO + 1),
        l_tableQueueSto, Q_DIM(l_tableQueueSto),
        l_tableStk, sizeof(l_tableStk));

    (void)QP::QF::run(); // run the QF application
}
