/* -*- mode:C++; c-basic-offset:4 -*- */

#ifndef _FDUMP_H
#define _FDUMP_H

#include <cstdio>

#include "db_cxx.h"
#include "tuple.h"
#include "packet.h"
#include "stage.h"
#include "trace/trace.h"
#include "qpipe_panic.h"



using namespace qpipe;



/* exported functions */


/**
 *  @brief
 */
struct fdump_packet_t : public packet_t {

public:
    
    static const char* PACKET_TYPE;

    tuple_buffer_t* _input_buffer;
    
    char* _filename;

    notify_t *_notifier;


    /**
     *  @brief FDUMP packet constructor.
     *
     *  @param packet_id The packet identifier.
     *
     *  @param out_buffer No real data will be transmitted through
     *  this buffer. The worker thread processing this packet will
     *  simply invoke send_eof() on this buffer when the file has been
     *  completely sent to disk.
     *
     *  @param in_buffer The input buffer.
     *
     *  FDUMP should not really need a tuple filter. All filtering for
     *  the query can be moved to the stage feeding the FDUMP. For
     *  this reason, we simple pass the null-filtering capabilities of
     *  the tuple_filter_t base class.
     */    
    fdump_packet_t(char* packet_id,
		   tuple_buffer_t* output_buffer,
		   tuple_buffer_t* input_buffer,
                   tuple_buffer_t* client_buffer,
		   const char*     filename,
                   notify_t *notifier=NULL)
	: packet_t(packet_id, PACKET_TYPE, output_buffer,
                   new tuple_filter_t(input_buffer->tuple_size), client_buffer),
	  _input_buffer(input_buffer), _notifier(notifier)
    {
	if ( asprintf(&_filename, "%s", filename) == -1 ) {
	    TRACE(TRACE_ALWAYS, "asprintf() failed on filename %s\n",
		  filename);
	    QPIPE_PANIC();
	}
    }


    virtual ~fdump_packet_t() {
	// may have deleted _filename earlier in terminate_inputs()
	if (_filename != NULL)
	    free(_filename);

        // let it be known that the file dump is finished
        if(_notifier)
            _notifier->notify();
    }


    virtual void terminate_inputs() {	
	
	// TODO detect close() error and delete input_buffer
	_input_buffer->close();

	// No longer any need to store filename separately. Set it to
	// NULL so the destructor can know and not do a double-free().
	free(_filename);
	_filename = NULL;

	// As for the output file, the meta-stage is responsible for
	// deleting the file when it knows we are all done.
    }
};



/**
 *  @brief FDUMP stage. Creates a (hopefully) temporary file on the
 *  local file system.
 */
class fdump_stage_t : public stage_t {

public:

    static const char* DEFAULT_STAGE_NAME;

    fdump_stage_t() { }

    virtual ~fdump_stage_t() { }

protected:

    virtual result_t process_packet();
};



#endif
