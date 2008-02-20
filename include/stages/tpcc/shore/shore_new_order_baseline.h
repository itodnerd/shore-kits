/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file shore_new_order_baseline.h
 *
 *  @brief Interface for the Shore Baseline TPC-C NewOrder transaction.
 *  The Baseline implementation uses a single thread for the entire
 *  transaction. We wrap the code to a stage in order to use the
 *  same subsystem
 *
 *  @author Ippokratis Pandis (ipandis)
 */


#ifndef __SHORE_TPCC_NEW_ORDER_BASELINE_H
#define __SHORE_TPCC_NEW_ORDER_BASELINE_H

#include <cstdio>

#include "core.h"
#include "util.h"
#include "scheduler.h"

#include "stages/tpcc/common/trx_packet.h"
#include "stages/tpcc/common/tpcc_input.h"
#include "stages/tpcc/shore/shore_tpcc_env.h"


using namespace qpipe;
using namespace tpcc;


/* exported datatypes */

class shore_new_order_baseline_packet_t : public trx_packet_t 
{
public:

    static const c_str PACKET_TYPE;

    // structure that contains the required input
    new_order_input_t _no_in;


    /**
     *  @brief shore_new_order_baseline_packet_t constructor.
     *
     *  @param packet_id The ID of this packet. This should point to a
     *  block of bytes allocated with malloc(). This packet will take
     *  ownership of this block and invoke free() when it is
     *  destroyed.
     *
     *  @param output_buffer The buffer where this packet should send
     *  its data. A packet DOES NOT own its output buffer (we will not
     *  invoke delete or free() on this field in our packet
     *  destructor).
     *
     *  @param output_filter The filter that will be applied to any
     *  tuple sent to output_buffer. The packet OWNS this filter. It
     *  will be deleted in the packet destructor.
     *
     *  @param All the NEW_ORDER transaction input variables
     */

    shore_new_order_baseline_packet_t(const c_str    &packet_id,
                                      tuple_fifo*     output_buffer,
                                      tuple_filter_t* output_filter,
                                      const new_order_input_t& a_no_input)
        : trx_packet_t(packet_id, PACKET_TYPE, output_buffer, output_filter,
                       create_plan(a_no_input._wh_id, a_no_input._d_id, 
                                   a_no_input._c_id, a_no_input._ol_cnt),
                       false, /* merging allowed */
                       true  /* unreserve worker on completion */
                       ),
          _no_in(a_no_input) /* copy input */          
    {
        _trx_state = UNDEF;
    }


    virtual ~shore_new_order_baseline_packet_t() { }

    
    // FIXME: (ip) Correct the plan creation
    static query_plan* create_plan(const short a_wh_id,
                                   const short a_d_id,
                                   const short a_c_id,
                                   const short a_ol_cnt) 
      {
        c_str action("%s:%d:%d:%d:%d", 
                     PACKET_TYPE.data(), 
		     a_wh_id, a_d_id, 
                     a_c_id, a_ol_cnt);
        
        return new query_plan(action, "none", NULL, 0);
      }

    
    virtual void declare_worker_needs(resource_declare_t* declare) {
      declare->declare(_packet_type, 1);
      /* no inputs */
    }


    /** Helper Functions */

    void describe_trx() {        
        _no_in.describe(_trx_id);
    }

}; // EOF shore_new_order_baseline_packet_t



/** 
 *  @brief SHORE_NEW_ORDER_BASELINE stage. 
 *
 *  @desc Executes the entire tpcc new-order transaction in a conventional,
 *  single-threaded fashion, using Shore.
 */

class shore_new_order_baseline_stage_t : public stage_t {

protected:

    virtual void process_packet();

public:

    static const c_str DEFAULT_STAGE_NAME;
    typedef shore_new_order_baseline_packet_t stage_packet_t;

    shore_new_order_baseline_stage_t();
 
    ~shore_new_order_baseline_stage_t() { 
	TRACE(TRACE_DEBUG, "SHORE_NEW_ORDER_BASELINE destructor\n");	
    }        
    
}; // EOF: shore_new_order_baseline_stage_t


#endif
