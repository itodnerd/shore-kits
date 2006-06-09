/* -*- mode:C++; c-basic-offset:4 -*- */

#ifndef __AGGREGATE_H
#define __AGGREGATE_H

#include <cstdio>

#include "tuple.h"
#include "packet.h"
#include "stage.h"
#include "trace/trace.h"
#include "qpipe_panic.h"



using namespace qpipe;

/**
 *  @brief Packet definition for the aggregation stage.
 */

struct aggregate_packet_t : public packet_t {

    static const char* PACKET_TYPE;
  
    tuple_buffer_t *input_buffer;
    tuple_aggregate_t *aggregate;
    aggregate_packet_t(char *packet_id,
		       tuple_buffer_t *out_buffer,
		       tuple_filter_t *filt,
		       tuple_buffer_t *in_buffer,
		       tuple_aggregate_t *agg)
	: packet_t(packet_id, PACKET_TYPE, out_buffer, filt, in_buffer, false),
	 input_buffer(in_buffer),
	 aggregate(agg)
    {
    }

    virtual ~aggregate_packet_t() {
    }

    virtual void terminate_inputs() {
	// TODO detect close() error and delete input_buffer
	input_buffer->close();
    }

};



/**
 *  @brief Aggregation stage that aggregates over grouped inputs. It
 *  produces one output tuple for each input set of tuples.
 */

class aggregate_stage_t : public stage_t {

public:

    static const char* DEFAULT_STAGE_NAME;

    aggregate_stage_t() { }

    ~aggregate_stage_t() { }


 protected:

    virtual int process_packet();
};



#endif
