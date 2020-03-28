//.$file${.::qhsmtst.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qhsmtst.qm
// File:  ${.::qhsmtst.hpp}
//
// This code has been generated by QM 4.6.0 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::qhsmtst.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef QHSMTST_HPP
#define QHSMTST_HPP

enum QHsmTstSignals {
    A_SIG = QP::Q_USER_SIG,
    B_SIG,
    C_SIG,
    D_SIG,
    E_SIG,
    F_SIG,
    G_SIG,
    H_SIG,
    I_SIG,
    TERMINATE_SIG,
    IGNORE_SIG,
    MAX_SIG
};

extern QP::QHsm * const the_hsm; // opaque pointer to the test HSM

// BSP functions to dispaly a message and exit
void BSP_display(char const *msg);
void BSP_terminate(int16_t const result);

#endif // QHSMTST_HPP
