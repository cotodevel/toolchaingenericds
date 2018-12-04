//stub pthreads since we give linux environment to TGDS/newlib for NDS, and most linux programs use threading.

#include <pthread.h>
#include <stdlib.h>
#include "config.h"
#include <pthreadtypes.h>

#ifndef HAVE_PTHREAD_SELF
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_self() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_self = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_MUTEX_INIT
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_mutex_init() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_mutex_init = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_MUTEX_DESTROY
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_mutex_destroy() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_mutex_destroy = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_MUTEX_LOCK
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_mutex_lock() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_mutex_lock = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_MUTEX_UNLOCK
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_mutex_unlock() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_mutex_unlock = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_COND_INIT
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_cond_init() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_cond_init = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_COND_DESTROY
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_cond_destroy() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_cond_destroy = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_CONDATTR_INIT
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_condattr_init() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_condattr_init = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_CONDATTR_DESTROY
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_condattr_destroy() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_condattr_destroy = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_COND_WAIT
#define NEED_ABORT_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_cond_wait() __attribute__ ((weak, alias ("__pthread_abort_stub")));
# else
#  pragma weak pthread_cond_wait = __pthread_abort_stub
# endif
#endif

#ifndef HAVE_PTHREAD_COND_TIMEDWAIT
#define NEED_ABORT_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_cond_timedwait() __attribute__ ((weak, alias ("__pthread_abort_stub")));
# else
#  pragma weak pthread_cond_timedwait = __pthread_abort_stub
# endif
#endif

#ifndef HAVE_PTHREAD_COND_SIGNAL
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_cond_signal() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_cond_signal = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_COND_BROADCAST
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_cond_broadcast() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_cond_broadcast = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_EQUAL
#define NEED_EQUAL_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_equal() __attribute__ ((weak, alias ("__pthread_equal_stub")));
# else
#  pragma weak pthread_equal = __pthread_equal_stub
# endif
#endif

#ifndef HAVE_PTHREAD_EXIT
#define NEED_EXIT_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_exit() __attribute__ ((weak, alias ("__pthread_exit_stub")));
# else
#  pragma weak pthread_exit = __pthread_exit_stub
# endif
#endif

#ifdef NEED_ZERO_STUB
static int __pthread_zero_stub()
{
    return 0;
}
#endif

#ifdef NEED_ABORT_STUB
static int __pthread_abort_stub()
{
    abort();
}
#endif

#ifdef NEED_EQUAL_STUB
static int __pthread_equal_stub(pthread_t t1, pthread_t t2)
{
    return (t1 == t2);
}
#endif

#ifdef NEED_EXIT_STUB
static void __pthread_exit_stub(void *ret)
{
    exit(EXIT_SUCCESS);
}
#endif

#ifndef HAVE_PTHREAD_CANCEL
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_cancel() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_cancel = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_JOIN
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_join() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_join = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_CREATE
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int pthread_create() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak pthread_create = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_CLEANUP_PUSH
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int _pthread_cleanup_push() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak _pthread_cleanup_push = __pthread_zero_stub
# endif
#endif

#ifndef HAVE_PTHREAD_CLEANUP_POP
#define NEED_ZERO_STUB
# ifdef SUPPORT_ATTRIBUTE_ALIAS
int _pthread_cleanup_pop() __attribute__ ((weak, alias ("__pthread_zero_stub")));
# else
#  pragma weak _pthread_cleanup_pop = __pthread_zero_stub
# endif
#endif