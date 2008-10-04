/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file:   dora_error.h
 *
 *  @brief:  Enumuration of DORA-related errors
 *
 *  @author: Ippokratis Pandis, Oct 2008
 *
 */

#ifndef __DORA_ERROR_H
#define __DORA_ERROR_H

#include "util/namespace.h"


ENTER_NAMESPACE(dora);

/* error codes returned from DORA */

/** note: All DORA-related errors start with 0x82.. */

enum {
    /** Problems generating */
    de_GEN_WORKER              = 0x820001,
    de_GEN_PRIMARY_WORKER      = 0x820002,
    de_GEN_STANDBY_POOL        = 0x820003,
    de_GEN_PARTITION           = 0x820004,
    de_GEN_TABLE               = 0x820005,

    /** Problems run-time */
    de_PROBLEM_ENQUEUE         = 0x820011,
    de_WRONG_ACTION            = 0x820011,
    de_WRONG_PARTITION         = 0x820012,
    de_WRONG_WORKER            = 0x820013,

    de_INCOMPATIBLE_LOCKS      = 0x820021
};



EXIT_NAMESPACE(dora);

#endif /* __DORA_ERROR_H */
