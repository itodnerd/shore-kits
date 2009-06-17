/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file:   flusher.h
 *
 *  @brief:  Specialization of a worker thread that acts as the dora-flusher.
 *
 *  @author: Ippokratis Pandis, Jun 2008
 */


/**
   Log flushing is a major source of ctxs (context switches). The higher the throughput
   of the system, the larger the number of ctxs due to log flushes, and the larger the 
   size of unnecessary work done due to those ctxs. In order to reduce the high rate of ctxs, 
   in DORA we break the final step of the execution of each transactions in two different phases: 
   the work until the log-flush, and the rest. 
   The thread that it is responsible for the execution of the transaction, or in DORA's
   case the thread that executes the final-rvp, instead of having to ctx waiting for
   the log-flush to finish, it transfers the control to another specialized worker thread,
   called dora-flusher. 
   The dora-flusher, peaks all the transactions whose log-flush has finished, and they are
   runnable again, and finalizes the work, notifying the client etc... The code looks like:

   dora-worker that executes final-rvp:
   { ... commit();
   dora-flusher->enqueue_flushing(pxct); } 
   
   dora-flusher:
   while (true) {
   if (has_flushed()) {
      xct* pxct = flushed_queue->get_one();
      { ... pxct->finalize(); notify_client(); }
   else { !! move all newly ready xct from flushing_queue }

   In order to enable this mechanism Shore-kits needs to be configured with:
   --enable-dora --enable-elr --enable-dora-flusher
*/

#ifndef __DORA_FLUSHER_H
#define __DORA_FLUSHER_H


#include "dora.h"
#include "sm/shore/shore_worker.h"

using namespace shore;


ENTER_NAMESPACE(dora);


/******************************************************************** 
 *
 * @class: dora_flusher_t
 *
 * @brief: A template-based class for the worker threads
 * 
 ********************************************************************/

class dora_flusher_t : public base_worker_t
{   
public:
    typedef srmwqueue<xct_t*>          Queue;

private:

    guard<Queue> _flushing;
    guard<Queue> _flushed;

    guard<Pool> _pxct_flushing_pool;
    guard<Pool> _pxct_flushed_pool;

    const int _pre_STOP_impl() { return(0); }

    // states
    const int _work_ACTIVE_impl(); 

public:

    dora_flusher_t(ShoreEnv* env, c_str tname,
                  processorid_t aprsid = PBIND_NONE, const int use_sli = 0) 
        : base_worker_t(env, tname, aprsid, use_sli)
    { 
        int expected_sz = 50*60;

        _pxct_flushing_pool = new Pool(sizeof(xct_t*),expected_sz);
        _flushing = new Queue(_pxct_flushing_pool.get());
        assert (_flushing->get());

        _pxct_flushed_pool = new Pool(sizeof(xct_t*),expected_sz);
        _flushed = new Queue(_pxct_flushed_pool.get());        
        assert (_flushed->get());
    }

    ~dora_flusher_t() 
    { 
        _flushing.done();
        _pxct_flushing_pool.done();
        _flushed.done();
        _pxct_flushed_pool.done();
    }

    //// Access methods

    inline enqueue_flushing(xct_t* axct) { _flushing.push(axct, true); }
    inline enqueue_flushed(xct_t* axct) { _flushed.push(axct, true); }
    

}; // EOF: dora_flusher_t


EXIT_NAMESPACE(dora);

#endif /** __DORA_FLUSHER_H */

