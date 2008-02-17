/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file tpcc_trx_input.h
 *
 *  @brief Declaration of functions that generate the inputs for the TPCC TRXs
 *
 *  @author Ippokratis Pandis (ipandis)
 */

#ifndef __TPCC_TRX_INPUT_H
#define __TPCC_TRX_INPUT_H

#include "stages/tpcc/common/tpcc_scaling_factor.h"
#include "stages/tpcc/common/tpcc_input.h" 


ENTER_NAMESPACE(tpcc);


/** Exported functionality */

payment_input_t      create_payment_input(int sf = QUERIED_TPCC_SCALING_FACTOR);

new_order_input_t    create_no_input(int sf = QUERIED_TPCC_SCALING_FACTOR);

order_status_input_t create_order_status_input(int sf = QUERIED_TPCC_SCALING_FACTOR);

delivery_input_t     create_delivery_input(int sf = QUERIED_TPCC_SCALING_FACTOR);

stock_level_input_t  create_stock_level_input(int sf = QUERIED_TPCC_SCALING_FACTOR);


EXIT_NAMESPACE(tpcc);

#endif
