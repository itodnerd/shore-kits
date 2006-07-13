/* -*- mode:C++; c-basic-offset:4 -*- */

#ifndef _STAGE_CONTAINTER_H
#define _STAGE_CONTAINTER_H

#include "engine/thread.h"
#include "engine/core/packet.h"
#include "engine/core/stage_factory.h"
#include "engine/core/stage.h"
#include "engine/util/guard.h"



using std::list;



// include me last!!!
#include "engine/namespace.h"



/* exported datatypes */


class stage_container_t {

public:
   
    static const unsigned int NEXT_TUPLE_UNINITIALIZED;
    static const unsigned int NEXT_TUPLE_INITIAL_VALUE;

	
protected:

    class stage_adaptor_t : public stage_t::adaptor_t {

    protected:
        
        // adaptor synch vars
	pthread_mutex_t _stage_adaptor_lock;

	stage_container_t* _container;

        packet_t*      _packet;
        packet_list_t* _packet_list;

	unsigned int _next_tuple;
        bool _still_accepting_packets;

        // Group many output() tuples into a page before "sending"
        // entire page to packet list
        page_guard_t out_page;
	
	// Checked independently of other variables. Don't need to
	// protect this with _stage_adaptor_mutex.
	volatile bool _cancelled;

	
    public:

        stage_adaptor_t(stage_container_t* container,
                        packet_list_t* packet_list,
                        size_t tuple_size);
        

        ~stage_adaptor_t() {
            // we should have deleted the primary packet
            assert( _packet == NULL );
            // we should have either deleted or handed off ownership
            // of the packet list
            assert( _packet_list == NULL );
        }


	virtual const char* get_container_name() {
	    return _container->get_name();
	}


        virtual packet_t* get_packet() {
            return _packet;
        }

        
        /**
         *  @brief Thin wrapper that just invokes output_page.
         *  Hopefully, this function will be inline and
         *  the compiler can optimize across the call to
         *  output_page, which is not virtual.
         */
        virtual void output(tuple_page_t *page) {
            output_page(page);
        }
	

	virtual void stop_accepting_packets() {
	    critical_section_t cs(&_stage_adaptor_lock);
	    _still_accepting_packets = false;
	}
	

        virtual bool check_for_cancellation() {
            return _cancelled;
        }


	bool try_merge(packet_t* packet);
	void run_stage(stage_t* stage);

    protected:

	void finish_packet(packet_t* packet);
        void cleanup();
        void abort_queries();

    private:

        void output_page(tuple_page_t *page);
    };
    
    
    // container synch vars
    pthread_mutex_t _container_lock;
    pthread_cond_t  _container_queue_nonempty;


    char*                   _container_name;
    list <packet_list_t*>   _container_queue;
    list <stage_adaptor_t*> _container_current_stages;

    stage_factory_t* _stage_maker;


    // container queue manipulation
    void container_queue_enqueue_no_merge(packet_list_t* packets);
    void container_queue_enqueue_no_merge(packet_t* packet);
    packet_list_t* container_queue_dequeue();
    
    
public:

    stage_container_t(const char* container_name, stage_factory_t* stage_maker);
    ~stage_container_t();
  
    const char* get_name(){ return _container_name; }

    void enqueue(packet_t* packet);

    void run();
};



#include "engine/namespace.h"



#endif
