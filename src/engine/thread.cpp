/* -*- mode:C++; c-basic-offset:4 -*- */

#include "engine/thread.h"
#include "trace.h"
#include "qpipe_panic.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdio>
#include <cstring>



/* internal constants */

#define MAX_STRERROR_STRING_SIZE 256



/* internal data structures */

static pthread_key_t THREAD_KEY_SELF;



/* internal helper functions */

static void thread_error(const char* function_name, int err);
static void thread_fatal_error(const char* function_name, int err);
static void thread_destroy(void* thread_object);
extern "C" void* start_thread(void *);



/* method definitions */


/**
 *  @brief thread_t base class constructor. Does nothing useful. Every
 *  subclass should invoke the static initialization functions to set
 *  up a new thread object.
 */
thread_t::thread_t(void)
{
    thread_name = NULL;
}



/**
 *  @brief This constructor should ONLY be used in thread_init() by
 *  the root thread. All other subclasses of thread_t() should invoke
 *  the thread_t(void) constructor and invoke init_thread_name().
 */
thread_t::thread_t(const char* format, ...)
{
    int ret;
    va_list ap;
    va_start(ap, format);
    ret = init_thread_name_v(format, ap);
    va_end(ap);
    assert( ret == 0 );
}



/**
 *  @brief thread_t destructor. Will 
 */
thread_t::~thread_t(void)
{
    // error checking... thread name should NEVER be NULL
    assert(thread_name != NULL);
    
    // thread_name was created with asprintf()/malloc()
    free(thread_name);
}



const char* thread_t::get_thread_name(void)
{
    return thread_name;
}



int thread_t::init_thread_name(const char* format, ...)
{
    int ret;
    va_list ap;
    va_start(ap, format);
    ret = init_thread_name_v(format, ap);
    va_end(ap);
    return ret;
}



int thread_t::init_thread_name_v(const char* format, va_list ap)
{
    // construct thread name
    if ( vasprintf(&thread_name, format, ap) == -1 )
    {
        TRACE(TRACE_ALWAYS, "vasprintf() failed to initialize thread_name\n");
        return -1;    
    }
  
    return 0;
}



/**
 *  @brief Initialize thread module.
 *
 *  @return void
 */

void thread_init(void)
{
    if ( pthread_key_create( &THREAD_KEY_SELF, thread_destroy ) )
    {
        TRACE(TRACE_ALWAYS, "pthread_key_create() failed on THREAD_KEY_SELF\n");
        QPIPE_PANIC();
    }
    thread_t* root_thread = new thread_t("root-thread");

    int err;
    err = pthread_setspecific(THREAD_KEY_SELF, root_thread);
    if (err)
        thread_fatal_error("pthread_setspecific()", err);
}
 


thread_t* thread_get_self(void)
{
    // It would be nice to verify that the name returned is not
    // NULL. However, the name of the root thread can be NULL if we have
    // not yet completely initialized it.
    return (thread_t*)pthread_getspecific(THREAD_KEY_SELF);
}



/**
 *  @brief Creates a new thread and starts it.
 *
 *  @param thread A pointer to a pthread_t that will store the ID of
 *  the newly created thread.
 *
 *  @param t - A thead that contains the function to run.
 *
 *  @return 0 on success. Non-zero on error.
 */

int thread_create(pthread_t* thread, thread_t* t)
{
    int err;
    pthread_t tid;
    pthread_attr_t pattr;

  
    // create a new kernel schedulable thread
    err = pthread_attr_init( &pattr );
    if (err) {
        thread_error("pthread_attr_init()", err);
        return -1;
    }
  
    err = pthread_attr_setscope( &pattr, PTHREAD_SCOPE_SYSTEM );
    if (err) {
        thread_error("pthread_attr_setscope()", err);
        return -1;
    }
  
    err = pthread_create(&tid, &pattr, start_thread, t);
    if (err) {
        thread_error("pthread_create()", err);
        return -1;
    }
  
    // thread created ... wait for it to initialize?
    // for now, assume it succeeds
  
    if ( thread != NULL )
        *thread = tid;
    return 0;
}



void pthread_mutex_init_wrapper(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr)
{

  pthread_mutexattr_t        mutex_attr;
  const pthread_mutexattr_t* ptr_mutex_attr;
  int err;

  
  if (attr == NULL)
  {
    err = pthread_mutexattr_init(&mutex_attr);
    if (err)
      thread_fatal_error("pthread_mutexattr_init()", err);

    err = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    if (err)
      thread_fatal_error("pthread_mutexattr_settype()", err);
    
    err = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_PRIVATE);
    if (err)
      thread_fatal_error("pthread_mutexattr_setpshared()", err);

    ptr_mutex_attr = &mutex_attr;
  }
  else
  {
    ptr_mutex_attr = attr;
  }


  err = pthread_mutex_init(mutex, ptr_mutex_attr);
  if (err)
    thread_fatal_error("pthread_mutex_init()", err);
}



void pthread_mutex_lock_wrapper(pthread_mutex_t* mutex)
{
  int err = pthread_mutex_lock(mutex);
  if (err)
    thread_fatal_error("pthread_mutex_lock()", err);
}



void pthread_mutex_unlock_wrapper(pthread_mutex_t* mutex)
{
  int err = pthread_mutex_unlock(mutex);
  if (err)
    thread_fatal_error("pthread_mutex_unlock()", err);
}



void pthread_mutex_destroy_wrapper(pthread_mutex_t* mutex)
{
  int err = pthread_mutex_destroy(mutex);
  if (err)
    thread_fatal_error("pthread_mutex_destroy()", err);
}



void pthread_cond_init_wrapper(pthread_cond_t* cond, const pthread_condattr_t* attr)
{
  int err = pthread_cond_init(cond, attr);
  if (err)
    thread_fatal_error("pthread_cond_init()", err);
}



void pthread_cond_destroy_wrapper(pthread_cond_t* cond)
{
  int err = pthread_cond_destroy(cond);
  if (err)
    thread_fatal_error("pthread_cond_destroy()", err);
}



void pthread_cond_signal_wrapper(pthread_cond_t* cond)
{
  int err = pthread_cond_signal(cond);
  if (err)
    thread_fatal_error("pthread_cond_signal()", err);
}



void pthread_cond_wait_wrapper(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
  int err = pthread_cond_wait(cond, mutex);
  if (err)
    thread_fatal_error("pthread_cond_wait()", err);
}



/**
 *  @brief thread_main function for newly created threads. Receives a
 *  thread_t object as its argument and it calls its run() function.
 *
 *  @param thread_object A thread_t*.
 *
 *  @return The value returned by thread_object->run().
 */

void* start_thread(void* thread_object)
{
    thread_t* thread = (thread_t*)thread_object;

    int err;

    // Register local data. Should not fail since we only need two
    // pieces of thread-specific storage.
    err = pthread_setspecific(THREAD_KEY_SELF, thread);
    if (err)
        thread_fatal_error("pthread_setspecific() on THREAD_KEY_SELF", err);
  
    return thread->run();
}



/* definitions of internal helper functions */


static void thread_error(const char* function_name, int err)
{
    char error_buf[MAX_STRERROR_STRING_SIZE];
    if ( strerror_r(err, error_buf, MAX_STRERROR_STRING_SIZE) ) {
        TRACE(TRACE_ALWAYS, "%s failed\n", function_name);
        TRACE(TRACE_ALWAYS, "strerror_r() failed to parse error code\n");
    }
    else
        TRACE(TRACE_ALWAYS, "%s failed: %s\n", function_name, error_buf);
}



static void thread_fatal_error(const char* function_name, int err)
{
    thread_error(function_name, err);
    QPIPE_PANIC();
}



static void thread_destroy(void* thread_object)
{
    thread_t* thread = (thread_t*)thread_object;
    delete thread;
}
