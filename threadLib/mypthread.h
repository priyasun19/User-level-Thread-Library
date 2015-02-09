#ifndef MY_PTHREAD_H
#define MY_PTHREAD_H

#if MYPTHREAD > 0

#define STACK_SIZE 8192
#define MAX_THREADS 8192
#define MY_PTHREAD_MUTEX_INITIALIZER {.spin=0,.lock=0,.kind=0}
#include <ucontext.h>

typedef enum {READY,RUNNING,BLOCKED,SIGWAIT,WAITING,KILLED} ThreadState;


typedef unsigned int my_pthread_t; /* change this if necessary */
typedef struct TCB_struct
{
int tid;
struct TCB_struct *parent;
ucontext_t context;
ThreadState state;
int joinable; //0-detached; 1-joinable
struct threadQueue_t *waitThreads;
int join_count;
} my_pthread;

struct threadLib_struct
{
	my_pthread **threads;
}threadLib;

struct threadQueue_t
{
	my_pthread *tcb;
	struct threadQueue_t *next;
};

struct threadQueue
{
	struct threadQueue_t *head;
	struct threadQueue_t *tail;
};

typedef struct {
	int lock; //0-no one is holding; 1-holding
	int spin;
	my_pthread_t owner;
	int kind;
	struct threadQueue mutexWaitQueue;
} my_pthread_mutex_t;

typedef struct
{
}my_pthread_mutexattr_t;

typedef struct {
	my_pthread_mutex_t *mutex;
	struct threadQueue condWaitQueue;
} my_pthread_cond_t;

typedef void my_pthread_attr_t;
//typedef void my_pthread_mutexattr_t;
typedef void my_pthread_condattr_t;

struct threadQueue readyQueue;
struct threadQueue waitQueue;
struct threadQueue terminationQueue;
my_pthread *current;
static int threadCount=-1;

#if PREEMPTIVE > 0
struct sigaction preempt_sa;
static char stack[SIGSTKSZ];
static void preempt(int sig,siginfo_t* siginfo,void *context);
#endif

#else /* MY_THREAD */

#include <pthread.h>

typedef pthread_t my_pthread_t;
typedef pthread_mutex_t my_pthread_mutex_t;
typedef pthread_cond_t my_pthread_cond_t;
typedef pthread_attr_t my_pthread_attr_t;
typedef pthread_mutexattr_t my_pthread_mutexattr_t;
typedef pthread_condattr_t my_pthread_condattr_t;

#endif
void FIFOscheduler();
//int threadLib_init();
void threadExecute(void* (*funct)(void*),void *args);
/* thread creation */
int my_pthread_create(my_pthread_t *thread, const my_pthread_attr_t *attr,
		      void *(*start_routine) (void *), void *arg);
void my_pthread_exit(void *retval);
int my_pthread_yield(void);
//int my_pthread_join(my_pthread_t *thread, void **retval);
int my_pthread_join(my_pthread_t thread, void **retval);
/* mutex */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex,
			  const my_pthread_mutexattr_t *mutexattr);
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

/* conditional variables */
int my_pthread_cond_init(my_pthread_cond_t *cond,
			 my_pthread_condattr_t *cond_attr);
int my_pthread_cond_signal(my_pthread_cond_t *cond);
int my_pthread_cond_broadcast(my_pthread_cond_t *cond);
int my_pthread_cond_wait(my_pthread_cond_t *cond, my_pthread_mutex_t *mutex);
int my_pthread_cond_destroy(my_pthread_cond_t *cond);

#endif
