#include "tuple.h"
#include "thread.h"

// include me last!
#include "namespace.h"

int tuple_buffer_t::check_page_full() {
    // next page?
    if(write_page->full()) {
        // cancelled?
        if(page_buffer.write(write_page))
            return 1;
        
        // TODO: something besides malloc
        write_page = tuple_page_t::alloc(tuple_size, malloc);
        return 0;
    }

    // otherwise just test for cancellation
    return page_buffer.stopped_reading();
}

int tuple_buffer_t::wait_for_input() {
    if(read_page != NULL)
        return 0;

    // wait for the next page to arrive
    page_t *page = page_buffer.read();
    if(!page)
        return 1;
        
    read_page = tuple_page_t::mount(page);
    read_iterator = read_page->begin();
    return 0;
}

int tuple_buffer_t::get_tuple(tuple_t &rec) {
    // make sure there is a valid page
    if(!wait_for_input())
        return 1;

    rec = *read_iterator++;
    if(read_iterator == read_page->end()) {
        free(read_page);
        read_page = NULL;
    }
    
    return 0;
}

void tuple_buffer_t::send_eof() {
    if(write_page->empty()) 
        free(write_page);
    else
        page_buffer.write(write_page);

    page_buffer.stop_writing();
}

void tuple_buffer_t::close_buffer() {
    free(read_page);
    page_buffer.stop_reading();
}

void tuple_buffer_t::init(size_t _tuple_size, size_t _page_size) {
    input_arrived = false;
    initialized = false;
    tuple_size = _tuple_size;
    page_size = _page_size;
}

/**
 * Unlock the buffer and lets those that are waiting for it to start consuming
 * a thread can wait for a buffer of tuples by calling the wait_init  
 */

void tuple_buffer_t::init_buffer() {
    // unblock waiting producer
    pthread_mutex_lock_wrapper(&init_lock);
    
    initialized = true;
    pthread_cond_signal_wrapper(&init_notify);
    
    pthread_mutex_unlock_wrapper(&init_lock);
}


#include "namespace.h"
