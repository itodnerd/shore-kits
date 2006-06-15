// -*- mode:C++; c-basic-offset:4 -*-

#ifndef __SORT_H
#define __SORT_H

#include "stage.h"
#include "stages/merge.h"
#include "dispatcher.h"
#include "trace/trace.h"
#include "qpipe_panic.h"
#include <list>
#include <map>
#include <deque>
#include <string>



using namespace qpipe;
using namespace std;



/* exported functions */



/**
 *@brief Packet definition for the sort stage
 */
struct sort_packet_t : public packet_t {

public:

    static const char* PACKET_TYPE;


    tuple_comparator_t* _comparator;
    packet_t*           _input;
    tuple_buffer_t*     _input_buffer;


    /**
     *  @brief sort_packet_t constructor.
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
     *  @param output_filter The filter that will be applied to any
     *  tuple sent to output_buffer. The packet OWNS this filter. It
     *  will be deleted in the packet destructor.
     *
     *  @param aggregator The aggregator we will be using for this
     *  packet. The packet OWNS this aggregator. It will be deleted in
     *  the packet destructor.
     *
     *  @param input The input packet for this aggregator. The packet
     *  takes ownership of this packet, but it will hand off ownership
     *  to a container as soon as this packet is dispatched.
     */
    sort_packet_t(char*               packet_id,
                  tuple_buffer_t*     output_buffer,
                  tuple_filter_t*     output_filter,
                  tuple_comparator_t* comparator,
                  packet_t*           input)
	: packet_t(packet_id, PACKET_TYPE, output_buffer, output_filter, false),
          _comparator(comparator),
          _input(input),
          _input_buffer(input->_output_buffer)
    {
        assert(_input != NULL);
        assert(_input_buffer != NULL);
    }


    virtual ~sort_packet_t() {
        assert(_input == NULL);
        assert(_input_buffer == NULL);
        delete _comparator;
    }


    virtual void destroy_subpackets() {

        delete _input_buffer;
        _input_buffer = NULL;
        
        _input->destroy_subpackets();
        delete _input;
        _input = NULL;
    }
    
    
    virtual void terminate_inputs() {

        // input buffer
        if ( !_input_buffer->terminate() ) {
            // Producer has already terminated this buffer! We are now
            // responsible for deleting it.
            delete _input_buffer;
        }
        _input_buffer = NULL;


        // TODO Ask the dispatcher to clear our input packet (_input)
        // from system, if it still exists.
        
        // Now that we know _input is not in the system, remove our
        // reference to it.
        _input = NULL;


        // TODO destroy any other packets we may have dispatched
        // during our execution
    }
    
};



/**
 * @brief Sort stage that partitions the input into sorted runs and
 * merges them into a single output run.
 */
class sort_stage_t : public stage_t {

private:


    static const unsigned int MERGE_FACTOR;
    static const unsigned int PAGES_PER_INITIAL_SORTED_RUN;

    
    // state provided by the packet
    tuple_buffer_t*     _input_buffer;
    tuple_comparator_t* _comparator;
    size_t              _tuple_size;
    

    typedef list<string> run_list_t;


    // all information we need for an active merge
    struct merge_t {
	string _output; // name of output file
	run_list_t _inputs;
        tuple_buffer_t* _signal_buffer;
	
	merge_t () { }
        merge_t(const string &output, const run_list_t &inputs, tuple_buffer_t * signal_buffer)
            : _output(output),
	      _inputs(inputs),
	      _signal_buffer(signal_buffer)
        {
        }
    };

    
    typedef map<int, run_list_t> run_map_t;
    typedef list<merge_t> merge_list_t;
    typedef map<int, merge_list_t> merge_map_t;
    typedef merge_packet_t::buffer_list_t buffer_list_t;
    typedef vector<key_tuple_pair_t> key_vector_t;


    // used to communicate with the monitor thread
    pthread_t _monitor_thread;
    notify_t  _monitor;


    volatile bool _sorting_finished;
    

    // run/merge management
    run_map_t   _run_map;
    merge_map_t _merge_map;
    
public:

    static const char* DEFAULT_STAGE_NAME;


    sort_stage_t() { }

    
    ~sort_stage_t() {

        // make sure the monitor thread exits before we do...
        if(_monitor_thread && pthread_join(_monitor_thread, NULL)) {
            TRACE(TRACE_ALWAYS, "sort stage unable to join on monitor thread");
            QPIPE_PANIC();
        }
        
        // also, remove any remaining temp files
        merge_map_t::iterator level_it=_merge_map.begin();
        while(level_it != _merge_map.end()) {
            merge_list_t::iterator it = level_it->second.begin();
            while(it != level_it->second.end()) {
                remove_input_files(it->_inputs);
                ++it;
            }
            ++level_it;
        }
    }

protected:

    virtual result_t process_packet();
    
private:

    bool final_merge_ready();
    int create_sorted_run(int page_count);

    tuple_buffer_t* monitor_merge_packets();

    void check_finished_merges();
    void start_new_merges();
    void start_merge(int new_level, run_list_t& runs, int merge_factor);
    void remove_input_files(run_list_t& files);

    // debug
    int print_runs();
    int print_merges();
};



#endif
