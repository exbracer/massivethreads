/* 
 * myth.h --- toplevel include file of MassiveThreads
 */
#pragma once
#ifndef MYTH_H_
#define MYTH_H_

#include <stdlib.h>
#include "myth/myth_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

  /*
    Function: myth_init

    Initialize MassiveThreads library with
    default parameters (see <myth_init_ex>).
    You normally do not have to call it by
    youself, as it is automatically called
    when you first call any MassiveThreads
    function.  Internally, it invokes underlying
    workers.  You may want to call it to make
    absolutely sure that an initilization has
    taken place before a certain point.

    Returns:

    The number of workers MassiveThreads that ends
    up launching.

    See Also: 
    <myth_init_ex>
  */
  int myth_init(void);

  /*
    Function: myth_init_ex

    Initialize MassiveThreads library with the
    specified number of workers and the
    default stack size (in bytes).  You
    normally do not have to call it by
    youself, as <myth_init> is automatically
    called when you first call any
    MassiveThreads function.  You may want to
    call it to explicitly to set the number of
    workers or the default stack size within
    your program.  Note that myth_init() can
    still specify those parameters via
    environment variables (see below).  Also
    note that myth_init() is equivalent to 
    myth_init_ex(0,0).

    Parameters:

    worker_num - the number of workers, or
    zero to mean the default (see below)

    def_stack_size - the default stack size of
    each user-level thread, or zero to mean
    the default (see below).

    When worker_num=0, it checks if the environment
    variable MYTH_WORKER_NUM is set.  If so,
    it parses it as an integer and uses the value.

    When def_stack_size=0, it checks if the
    environment variable MYTH_DEF_STKSIZE is
    set.  If so, it parses it as an integer
    and uses the value.  Otherwise it uses the
    global default.  The global default can be
    specified at compile time by ./configure
    --with-default-stack-size=S, whose default
    128KB (131072).  No matter what is the default,
    you can specify the size of individual
    user-level threads when creating them via
    <myth_create_ex>.

    Returns:

    The number of workers MassiveThreads that ends
    up launching.

    See Also: 
    <myth_init>, <myth_create>, <myth_create_ex>
  */
  int myth_init_ex(int worker_num, size_t def_stack_size);

  //void myth_init_withparam(int worker_num,size_t def_stack_size);

  /*
    Function: myth_fini

    Finalize MassiveThreads.

    See Also: <myth_init>, <myth_init_ex>
  */
  void myth_fini(void);

  /*
    Function: myth_fini_ex

    Finalize MassiveThreads.

    See Also: <myth_init>, <myth_init_ex>
  */
  void myth_fini_ex(void);

  void myth_exit_workers_ex(void);
  void myth_ext_exit_workers_ex(void);
  void myth_worker_start_ex(int rank);
  void myth_startpoint_init_ex(int rank);
  void myth_startpoint_exit_ex(int rank);

  /* 
     Type: myth_thread_attr_t

     This is given to myth_thread

     Fields:
     size_t stack_size - stack size in bytes (default: what you specified in <myth_init_ex>)
     int switch_immediately - 1 if myth_create should immediately switch to the child (default: 1)
     size_t custom_data_size - 
     void *custom_data - 
  */

  typedef struct myth_thread_attr { 
    size_t stack_size;
    int switch_immediately;
    size_t custom_data_size;
    void *custom_data;
  } myth_thread_attr_t;
  
  /* Type: myth_func_t 

     a type of function taking an argument of
     type (void *) and returning a value of
     type (void *);
  */
  typedef void*(*myth_func_t)(void*);
  
  /* 
     at this point we define myth_thread_t
     to be an opaque pointer. struct myth_thread
     is defined in myth_desc.h
  */

  typedef struct myth_thread * myth_thread_t;

  /*
    Function: myth_create

    Create a new user-level thread executing func(arg)
    with default options.  Note that it is equivalent
    to myth_create_ex(func,arg,0);

    Parameters:

    func - a pointer to a function.
    arg - a pointer given to func.

    Returns:

    The identifier of the newly created user-level thread.

    Bug:

    Should any error occur, it terminates the
    program rather than returning an error
    code.

    See Also:
    <myth_create_ex>, <myth_join>
  */
  myth_thread_t myth_create(myth_func_t func,void *arg);

  /*
    Function: myth_create_ex

    Create a new user-level thread executing func(arg)
    with specified options.

    Parameters:

    id - a pointer, if not NULL, to which id of the created thread will be stored.
    func - a pointer to a function.
    arg - a pointer given to func.
    attr - a pointer to a data structure
    of type <myth_thread_attr_t>
    specifying thread attributes, or NULL
    to mean the deafult.

    Returns:

    0 if succeed.

    Bug:

    Should any error occur, it terminates the
    program rather than returning an error
    code.

    See Also:
    <myth_create>, <myth_join>, <myth_thread_option>
  */
  int myth_create_ex(myth_thread_t * id, myth_thread_attr_t * attr,
		     myth_func_t func, void * arg);
  myth_thread_t myth_create_nosched(myth_func_t func, void * arg,
				    myth_thread_attr_t * attr);

  /*
    Function: myth_create_join_many_ex

    Create many user-level threads executing the same function
    with various arguments and attributes and wait for them
    to complete.

    Parameters:

    ids -           base pointer to a (strided) array, to which thread ids of
                    the created threads wll be stored (may be NULL)
    attrs -         base pointer to a (strided) array specifying attributes of
                    threads to create (may be NULL)
    func -          a function to execute by each thread
    args -          base pointer to a (strided) array specifying arguments to func
    results -       base pointer to a (strided) array to which results of the
                    function call will be stored (may be NULL)
    id_stride -     the stride of the ids array, in bytes
    attr_stride -   the stride of the attrs array, in bytes
    arg_stride -    the stride of the args array, in bytes
    result_stride - the stride of the results array, in bytes
    long nthreads - number of threads to execute f

    in its simplest form,

        myth_create_join_many_ex(0, 0, f, X, 0, 
                                 0, 0,    s, 0, n);

    will execute f(args), f(args+s), f(args+2*s), ..., and f(args+(n-1)*s),
    each by a separate thread and discard their return values. 
    if you want to get return values, give results and result_stride. e.g.,

        myth_create_join_many_ex(0, 0, f, X,  Y, 
                                 0, 0,    xs, t, n);

    is equivalent to:			     

    for all i = 0, ..., n - 1
        ((void **)(Y + i * t))[0] = f(args + i * s);

    note that all stride arguments must be given in bytes.
    this is to allow you to receive results in a field of 
    an enclosing structure. e.g.,

     struct { char stuff[100]; void * result } args[nthreads];

    in this case you want to call this function with 
    results = &args[0].result and result_stride = sizeof(args[0]);

    consistent with this policy, results is a void pointer,
    although it is internally used as (void **).

    You can similarly specify addresses of attributes and thread ids,
    using the base pointer and the stride.

    Returns:

    0 if succeed.

    Bug:

    Should any error occur, it terminates the
    program rather than returning an error
    code.

    See Also:
    <myth_create>, <myth_join>, <myth_thread_attr>
  */

  int myth_create_join_many_ex(myth_thread_t * ids,
			       myth_thread_attr_t * attrs,
			       myth_func_t func,
			       void * args,
			       void * results,
			       size_t id_stride,
			       size_t attr_stride,
			       size_t arg_stride,
			       size_t result_stride,
			       long nthreads);
  
  /*
    Function: myth_create_join_various_ex

    Create many user-level threads executing
    various functions with various arguments
    and attributes and wait for them to
    complete.  This is almost the same with
    myth_create_join_many_ex, except that you
    can have threads execute different
    functions.

    Parameters:

    ids -           base pointer to a (strided) array, to which thread ids of
                    the created threads wll be stored (may be NULL)
    attrs -         base pointer to a (strided) array specifying attributes of
                    threads to create (may be NULL)
    funcs -         base pointer to a (strided) array specifying functions
                    to execute
    args -          base pointer to a (strided) array specifying arguments to func
    results -       base pointer to a (strided) array to which results of the
                    function call will be stored (may be NULL)
    id_stride -     the stride of the ids array, in bytes
    attr_stride -   the stride of the attrs array, in bytes
    func_stride -   the stride of the funcs array, in bytes
    arg_stride -    the stride of the args array, in bytes
    result_stride - the stride of the results array, in bytes
    long nthreads - number of threads to execute f

    in its simplest form,

        myth_create_join_many_ex(0, 0, F,  X,  0, 
                                 0, 0, fs, xs, 0, n);

    will execute f_0(args), f_1(args+xs), f_2(args+2*xs), ..., 
    where f_i = *((myth_func_t *)(F + fs * i)),
    each by a separate thread and discard their return values.
    if you want to get return values, give results and result_stride. e.g.,

        myth_create_join_many_ex(0, 0, f, X, Y, 
                                 0, 0,    s, t, n);

    is equivalent to:			     

    for all i = 0, ..., n - 1
        ((void **)(Y + i * t))[0] = f(args + i * s);

    note that all stride arguments must be given in bytes.
    this is to allow you to receive results in a field of 
    an enclosing structure. e.g.,

     struct { char stuff[100]; void * result } args[nthreads];

    in this case you want to call this function with 
    results = &args[0].result and result_stride = sizeof(args[0]);

    consistent with this policy, results is a void pointer,
    although it is internally used as (void **).

    You can similarly specify addresses of attributes and thread ids,
    using the base pointer and the stride.

    Returns:

    0 if succeed.

    Bug:

    Should any error occur, it terminates the
    program rather than returning an error
    code.

    See Also:
    <myth_create>, <myth_join>, <myth_thread_attr>
  */

  int myth_create_join_various_ex(myth_thread_t * ids,
				  myth_thread_attr_t * attrs,
				  myth_func_t * funcs,
				  void * args,
				  void * results,
				  size_t id_stride,
				  size_t attr_stride,
				  size_t func_stride,
				  size_t arg_stride,
				  size_t result_stride,
				  long nthreads);

  /*
    Function: myth_join

    Wait for the specified thread th to finish.

    Parameters:

    th - the identifier of the thread to wait for 
    result - a pointer to a data structure receiving
    the exit value of the thread, as determined by
    <myth_exit> or the return value of the thread's
    main function.
   
    See Also:
    <myth_create>, <myth_create_ex>
   
  */
  void myth_join(myth_thread_t th, void **result);

  /*
    Function: myth_exit

    Terminate the calling user-level thread.

    Parameters:

    ret - exit value of the thread, which can
    be retrieved by calling <myth_join> on
    this thread.

    See Also:
    <myth_join>
  */
  void myth_exit(void *ret);

  /*
    Function: myth_get_worker_num

    Returns:
    The index of the calling worker, an 
    integer x satisfying
    0 <= x < myth_get_num_workers().

    See Also:
    <myth_get_num_workers>
  */
  int myth_get_worker_num(void);

  /*
    Function: myth_get_num_workers

    Returns:
    The number of underlying workers.

    See Also:
    <myth_get_worker_num>
  */
  int myth_get_num_workers(void);

  /*
    Function: myth_self

    Returns:
    The identifier of the calling thread.

    See Also:
    <myth_get_worker_num>, <myth_get_num_workers>
  */
  myth_thread_t myth_self(void);

  typedef unsigned int myth_key_t;

  /*
    Function: myth_key_create

    Create a key for user-level thread-specific data.

    Parameters:

    key - a pointer to which the created key will be stored.
    destr_function - a pointer to a destructor function.


    Returns:

    Zero if succeed, or an errno when an error occurred.

    Bug:

    destr_function is ignored in the current implementation.

    See Also:

    <myth_key_delete>, <myth_setspecific>, <myth_getspecific>
  */
  int myth_key_create(myth_key_t *key, void (*destr_function)(void *));

  /*
    Function: myth_key_delete

    Delete a key for user-level thread-specific data.

    Parameters:

    key - key to delete

    Returns:

    Zero if succeed, or an errno when an error occurred.

    See Also:

    <myth_key_create>, <myth_setspecific>, <myth_getspecific>
  */
  int myth_key_delete(myth_key_t key);

  /*
    Function: myth_setspecific

    Associate a thread-specific data with a key.

    Parameters:

    key - a key created by myth_key_create
    data - a data to be associated with key

    Returns:

    Zero if succeed, or an errno when an error occurred.

    See Also:

    <myth_key_create>, <myth_key_delete>, <myth_getspecific>
  */
  int myth_setspecific(myth_key_t key, void *data);

  /*
    Function: myth_getspecific

    Obtain a user-level thread-specific data
    associated with a key.

    Parameters:

    key - a key to retrieve data.

    Returns:

    a data previously associated with key via
    myth_setspecific, or NULL if no data has
    been associated with it.

    See Also:

    <myth_key_create>, <myth_key_delete>, <myth_setspecific>
  */
  void *myth_getspecific(myth_key_t key);

  int myth_detach(myth_thread_t th);
  int myth_setcancelstate(int state, int *oldstate);
  int myth_setcanceltype(int type, int *oldtype);
  int myth_cancel(myth_thread_t th);
  void myth_testcancel(void);

  /*
    Function: myth_yield

    Yield execution to another user-level thread.

    Parameters:
    force_worksteal - if 1, the underlying worker
    tries to steal a user-level thread from another worker,
    rather than trying to execute another user-level thread in
    its local scheduling queue.

    See Also:
    <myth_yield2>

    Note:
    The above describes the current implementation,
    which may change in future.  You should not rely
    on its exact behavior (other than it switches
    to another user-level thread).
   
  */
  void myth_yield(int force_worksteal);

  /*
    Function: myth_yield2

    Yield execution to another user-level
    thread in the worker's local scheduling
    queue.

    See Also:
    <myth_yield>

    Note:
    The above describes the current implementation,
    which may change in future.  You should not rely
    on its exact behavior (other than it switches
    to another user-level thread).
   
  */
  void myth_yield2(void);

  /* new api for mutex; made more similar to pthreads */

  /*
    Function: myth_mutex_init

    Initialize a mutex.

    Parameters:

    mutex - a pointer to a mutex data structure to initialize.
    attr - a pointer to mutex attributes.

    Returns:

    zero if suceeds or an errno.

    See Also:

    <myth_mutex_destroy>, <myth_mutex_lock>, <myth_mutex_trylock>, <myth_mutex_unlock>
  */
  int myth_mutex_init(myth_mutex_t * mutex,
		      const myth_mutexattr_t * attr);

  /*
    Function: myth_mutex_destroy

    Destroy a mutex.

    Parameters:

    mutex - a pointer to a mutex data structure to initialize.

    Returns:

    zero if suceeds or an errno.

    See Also:

    <myth_mutex_init>, <myth_mutex_lock>, <myth_mutex_trylock>, <myth_mutex_unlock>
  */
  int myth_mutex_destroy(myth_mutex_t * mutex);

  /*
    Function: myth_mutex_lock

    Lock a mutex.

    Parameters:

    mutex - a mutex to lock.

    Returns:

    zero if suceeds or an errno when an error occurred.

    See Also:
   
    <myth_mutex_init>, <myth_mutex_destroy>, <myth_mutex_trylock>, <myth_mutex_unlock>

  */
  int myth_mutex_lock(myth_mutex_t * mtx);

  /*
    Function: myth_mutex_trylock

    Try to lock a mutex.

    Parameters:

    mutex - a mutex to try to lock.

    Returns:

    zero if it successfully acquired a lock.
    an errno otherwise.

    See Also:
   
    <myth_mutex_init>, <myth_mutex_destroy>, <myth_mutex_lock>, <myth_mutex_unlock>
   
  */
  int myth_mutex_trylock(myth_mutex_t * mtx);


  /*
    Function: myth_mutex_unlock

    Unlock a mutex.

    Parameters:

    mutex - a mutex to unlock.

    Returns:

    zero if suceeds or an errno when an error occurred.

    See Also:

    <myth_mutex_init>, <myth_mutex_destroy>, <myth_mutex_lock>, <myth_mutex_trylock>
  */
  int myth_mutex_unlock(myth_mutex_t * mtx);

  /*
    Function: myth_cond_init

    Initialize a condition variable.

    Parameters:

    cond - a pointer to a condition variable to initialize
    attr - a pointer to condition variable attributes, or NULL

    Returns:

    Zero if succeed, or an errno when an error occurred.

    See Also:

    <myth_cond_destroy>, <myth_cond_wait>, <myth_cond_signal>, <myth_cond_broadcast>
  */
  int myth_cond_init(myth_cond_t * cond,
		     const myth_condattr_t * attr);

  /*
    Function: myth_cond_destroy

    Destroy a condition variable.

    Parameters:

    cond - a pointer to a condition variable to destroy.

    Returns:

    Zero if succeed, or an errno when an error occurred.

    See Also:

    <myth_cond_init>, <myth_cond_wait>, <myth_cond_signal>, <myth_cond_broadcast>
   
  */
  int myth_cond_destroy(myth_cond_t * cond);

  /*
    Function: myth_cond_wait

    Atomically unlock a mutex and block on a condition variable.

    Parameters:

    cond - a pointer to a condition variable to block on.
    mutex - a pointer to a mutex to unlock

    Returns:

    Zero if succeed, or an errno when an error occurred.

    See Also:

    <myth_cond_init>, <myth_cond_destroy>, <myth_cond_signal>, <myth_cond_broadcast>
  */
  int myth_cond_wait(myth_cond_t * cond, myth_mutex_t * mutex);

  /*
    Function: myth_cond_signal

    Wake up at least one thread blocking on a condition variable.

    Parameters:

    cond - a pointer to a condition variable to signal.

    Returns:

    Zero if succeed, or an errno when an error occurred.

    See Also:

    <myth_cond_init>, <myth_cond_destroy>, <myth_cond_wait>, <myth_cond_broadcast>
  */
  int myth_cond_signal(myth_cond_t * c);

  /*
    Function: myth_cond_broadcast

    Wake up all threads blocking on a condition variable.

    Parameters:

    cond - a pointer to a condition variable from which threads
    are to wake up.

    Returns:

    Zero if succeed, or an errno when an error occurred.

    See Also:

    <myth_cond_init>, <myth_cond_destroy>, <myth_cond_wait>, <myth_cond_signal>
  */
  int myth_cond_broadcast(myth_cond_t * cond);


  /* 
     Function: myth_barrier_init

     Initialize a barrier.

     Parameters:
     barrier - a pointer to a barrier data structure to initialize.
     attr - a pointer to barrier attributes
     count - the number of threads going to synchronize with this barrier

     Returns:
     Zero if succeeded.  An errno if failed.
  */
  int myth_barrier_init(myth_barrier_t * barrier,
			const myth_barrierattr_t * attr, 
			unsigned int count);
  /* 
     Function: myth_barrier_wait
   
     Wait on a barrier.

     Parameters:
     barrier - a pointer to a barrier data structure on which
     the calling thread synchronizes

     Returns:

     When a barrier succeeds,
     MYTH_BARRIER_SERIAL_THREAD is returned to
     a single thread whereas zeros to other
     threads.  When a barrier fails,
     an errno.
   
  */
  int myth_barrier_wait(myth_barrier_t *barrier);

  /* 
     Function: myth_barrier_destroy
   
     Destroy a barrier.

     Parameters:
     barrier - a pointer to a barrier data structure to destroy.
   
  */
  int myth_barrier_destroy(myth_barrier_t *barrier);

  /* join counter */
  int myth_join_counter_init(myth_join_counter_t * jc, 
			     const myth_join_counterattr_t * attr, int val);
  int myth_join_counter_wait(myth_join_counter_t * jc);
  int myth_join_counter_dec(myth_join_counter_t * jc);

  /* felock */

#if 1
  int myth_felock_init(myth_felock_t * fe, const myth_felockattr_t * attr);
  int myth_felock_destroy(myth_felock_t * fe);
  int myth_felock_lock(myth_felock_t * fe);
  int myth_felock_unlock(myth_felock_t * fe);
  int myth_felock_wait_and_lock(myth_felock_t * fe, int status_to_wait);
  int myth_felock_mark_and_signal(myth_felock_t * fe,int status_to_signal);
  int myth_felock_status(myth_felock_t * fe);

#else
  int myth_felock_init(myth_felock_t * felock, const myth_felockattr_t * attr);
  int myth_felock_destroy(myth_felock_t * felock);
  int myth_felock_lock(myth_felock_t * felock);
  int myth_felock_wait_lock(myth_felock_t * felock, int val);
  int myth_felock_unlock(myth_felock_t * felock);
  int myth_felock_status(myth_felock_t * felock);
  int myth_felock_set_unlock(myth_felock_t * felock, int val);
#endif


  /*
    Function: myth_set_def_stack_size

    Set the default stack size of subsequently
    created user-level threads.

    Parameters:

    newsize - the stack size of user-level threads

  */
  void myth_set_def_stack_size(size_t newsize);

  /* 
     declare myth_pickle_t to be an opaque structure
   */
  typedef struct myth_pickle myth_pickle_t;
  void myth_serialize(myth_thread_t th,myth_pickle_t * p);
#define myth_ext_serialize(th,p) myth_serialize(th,p)
  myth_thread_t myth_deserialize(myth_pickle_t * p);
  myth_thread_t myth_ext_deserialize(myth_pickle_t * p);

  myth_thread_t myth_steal(void);
#define myth_ext_steal() myth_steal()
  void myth_import(myth_thread_t th);
  void myth_ext_import(myth_thread_t th);

  void myth_release_stack(myth_thread_t th);
  void myth_release_desc(myth_thread_t th);

  void myth_log_start(void);
  void myth_log_pause(void);
  void myth_log_flush(void);
  void myth_log_reset(void);
  void myth_log_annotate_thread(myth_thread_t th,char *name);
  //void myth_log_get_thread_annotation(myth_thread_t th,char *name);
  void myth_sched_prof_start(void);
  void myth_sched_prof_pause(void);

  typedef int (*myth_wsapi_decidefn_t)(myth_thread_t th,void *udata);

  myth_thread_t myth_wsapi_runqueue_peek(int victim,void *ptr,size_t *psize);
  size_t myth_wsapi_get_hint_size(myth_thread_t th);
  void *myth_wsapi_get_hint_ptr(myth_thread_t th);
  void myth_wsapi_set_hint(myth_thread_t th,void **data,size_t *size);
  int myth_wsapi_rand(void);
  void myth_wsapi_randarr(int *ret,int n);
  myth_thread_t myth_wsapi_runqueue_take(int victim,myth_wsapi_decidefn_t decidefn,void *udata);
  int myth_wsapi_runqueue_pass(int target,myth_thread_t th);
  void myth_wsapi_runqueue_push(myth_thread_t th);
  myth_thread_t myth_wsapi_runqueue_pop(void);
  int myth_wsapi_rand(void);
  int myth_wsapi_rand2(int min,int max);

  /* TODO: a duplicated definition is in myth_worker.h */
  typedef struct myth_thread* (*myth_steal_func_t)(int);
  myth_steal_func_t myth_wsapi_set_stealfunc(myth_steal_func_t fn);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* MYTH_H_ */
