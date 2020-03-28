//****************************************************************************
// DPP example
// Last updated for version 6.3.3
// Last updated on  2018-06-23
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//****************************************************************************
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

static QP::QTicker l_ticker0(0); // ticker for tick rate 0
QP::QActive *DPP::the_Ticker0 = &l_ticker0;

//............................................................................
int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO];

    QP::QF::init();  // initialize the framework and the underlying RT kernel

    DPP::BSP::init(); // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(DPP::AO_Table);
    QS_OBJ_DICTIONARY(DPP::AO_Philo[0]);
    QS_OBJ_DICTIONARY(DPP::AO_Philo[1]);
    QS_OBJ_DICTIONARY(DPP::AO_Philo[2]);
    QS_OBJ_DICTIONARY(DPP::AO_Philo[3]);
    QS_OBJ_DICTIONARY(DPP::AO_Philo[4]);

    QP::QF::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start((uint_fast8_t)(n + 1U), // priority
                           philoQueueSto[n], Q_DIM(philoQueueSto[n]),
                           (void *)0, 0U);
    }

    // example of prioritizing the Ticker0 active object
    DPP::the_Ticker0->start((uint_fast8_t)(N_PHILO + 1U), // priority
                            0, 0,
                            0, 0);

    DPP::AO_Table->start((uint_fast8_t)(N_PHILO + 2U), // priority
                    tableQueueSto, Q_DIM(tableQueueSto),
                    (void *)0, 0U);

    return QP::QF::run(); // run the QF application
}
