#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "mypthread.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <ucontext.h>
#include<sys/mman.h>
#include <unistd.h>
#include<stdint.h>
#include<signal.h>
#define __USE_GNU

#if MYPTHREAD > 0

extern void my_pthread_begin();
extern void my_pthread_end();
extern int main(int argc, char *argv[]);
	/* Insert code here */

//call before main
void threadLib_init (void) __attribute__ ((constructor));

//Queue Functions
int addToQueue(my_pthread* tcbobj,struct threadQueue *queue)
{
 struct threadQueue_t *threadobj;
 threadobj=(struct threadQueue_t *)malloc(sizeof(struct threadQueue_t));
 threadobj->tcb=tcbobj;
 threadobj->next=NULL;
 if(queue->head==NULL && queue->tail==NULL)
 {
  queue->head = threadobj;
  queue->tail = threadobj;
 }
 else
 {
  (queue->tail)->next = threadobj;
  queue->tail=threadobj;
 }
 return 0;
}

my_pthread* dequeue(struct threadQueue *queue)
{
 if(queue->head==NULL && queue->tail==NULL)
  return NULL;
 else
 {
  my_pthread *oldnode=(queue->head)->tcb;
  if(queue->head==queue->tail)
  {
    queue->head=NULL;
    queue->tail=NULL;
  }
  else
   queue->head=(queue->head)->next;
   return oldnode;
 }
 
}

//delete given node and queue
my_pthread* deleteNode(my_pthread *tcb,struct threadQueue *queue)
{
	struct threadQueue_t *temp,*prev;
	if(queue->head==NULL && queue->tail==NULL)
		return NULL;
	else
	{
		temp=queue->head;
		prev=temp;
		while(temp!=NULL)
		{
			if(temp->tcb==tcb)
			{
				//if only one node
				if(queue->head==queue->tail)
				{
					queue->head=NULL;
					queue->tail=NULL;
				}
				//if head, change head. If tail, change tail
				else if(temp==queue->head)
					queue->head=(queue->head)->next;
				else if(temp==queue->tail)
				{
					queue->tail=prev;
				}
				prev->next=temp->next;
				free(temp);
				break;
			}
			prev=temp;
			temp=temp->next;
		}
		return tcb;
	}
}
/*
void printQueue(struct threadQueue queue)
{
 struct threadQueue_t *temp=queue.head;
 while(temp!=NULL)
 {
  temp=temp->next;
  printf("%d ",(temp->tcb)->tid);
 }
}
*/

void threadLib_init(void)
{
	printf("init..");
	int i=0;
	threadLib.threads=(my_pthread**)malloc(sizeof(my_pthread*)*MAX_THREADS);
 	for(i=0;i<MAX_THREADS;i++)
		threadLib.threads[i]=(my_pthread*)malloc(sizeof(my_pthread));
	if(threadLib.threads!=NULL)
	{
		ucontext_t context;
		my_pthread *mainthread; //main thread 
		mainthread=(my_pthread *)malloc(sizeof(my_pthread));
		if(getcontext(&context)!=0 || (threadCount>MAX_THREADS))
		{
			free(mainthread);
			//return -1;
		}
		mainthread->tid=++threadCount;
		mainthread->context=context; //doesn't matter now. While switching let us save latest context
		mainthread->state=RUNNING;
		mainthread->parent=NULL;
		mainthread->joinable=0;
		mainthread->join_count=0;
		mainthread->waitThreads=NULL;
		readyQueue.head=NULL;
		readyQueue.tail=NULL;
		current=mainthread;
		threadLib.threads[mainthread->tid]=mainthread;
		//return 0;
	}
	//If preemptive scheduler set signal
	#if PREEMPTIVE > 0
	memset(&preempt_sa, 0, sizeof(preempt_sa));
    	preempt_sa.sa_sigaction = preempt;
    	preempt_sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
	stack_t ss={.ss_size=SIGSTKSZ,.ss_sp=stack};
	sigaltstack(&ss,0);
    	if (sigaction(SIGALRM, &preempt_sa, NULL) < 0) 
	{
        	printf("Preemptive scheduler activation failed.");
    	}
	alarm(1);
	#endif	
}
int my_pthread_create(my_pthread_t *threadid, const my_pthread_attr_t *attr,void *(*start_routine) (void *), void *args)
{
	my_pthread *thread=(my_pthread*)malloc(sizeof(my_pthread));
	printf("create\n");
	ucontext_t childcontext;
        
	my_pthread *parent=current;//active();
 	if(parent==NULL)
  		return -1; 

	getcontext(&childcontext);

	childcontext.uc_link=0;
 	childcontext.uc_stack.ss_sp=mmap(0, 0x10000,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_ANON, -1, 0);
 	childcontext.uc_stack.ss_size=0x10000;
 	childcontext.uc_stack.ss_flags=0;
 	makecontext(&childcontext,(void(*)(void))threadExecute,2,start_routine,args);
 	thread->parent=parent;
 	thread->tid=++threadCount;
 	thread->context=childcontext;
 	thread->state=READY;
	thread->joinable=0;
	thread->join_count=0;
	thread->waitThreads=NULL;
	threadLib.threads[thread->tid]=thread;
	addToQueue(thread,&readyQueue);
	*threadid=thread->tid;
	return 0;
}
void wakeFromWaitQueue(my_pthread *tcb)
{
	my_pthread *temp;
	if(tcb->state==WAITING)
	{	
		//check if not waiting for io or lock--
		//if not move to ready queue
		temp=deleteNode(tcb,&waitQueue);
		if(temp==NULL)
			return;
		temp->state=READY;
		addToQueue(tcb,&readyQueue);
	}
}
void my_pthread_exit(void *retval)
{
	printf("in exit..\n");
	printf("%d %d\n",current->state,current->joinable);
	//check if already exited..If so just return
	if(current->state==KILLED)
		return;
	//If joinable wake waiting thread	
	//If detached add to terminate queue
	if(current->joinable==1)
	{
		struct threadQueue_t *temp=current->waitThreads;
		struct threadQueue_t *prev;
		while(temp!=NULL)
		{printf("in1..");
			if((temp->tcb)->state==WAITING)
				(temp->tcb->join_count)--;
			if(temp->tcb->join_count==0)
			{
				wakeFromWaitQueue(temp->tcb);printf("in2..");
			}
			prev=temp;
			temp=temp->next;
			free(prev); //deallocate waitThreads
		}
		current->joinable=0;
	}
	else
	{
	}
	current->state=KILLED;
	//Deallocate context stack 
	//free((current->context).uc_stack.ss_sp);
	retval=NULL;
}

int my_pthread_yield(void)
{
	current->state=READY;
	addToQueue(current,&readyQueue);
	FIFOscheduler();
	return 0;
}

int my_pthread_join(my_pthread_t threadid, void **retval)
{
	my_pthread *thread=threadLib.threads[threadid];
	printf("join %d\n",thread->joinable);
	if(current==NULL)
	{
		retval=NULL;
		return 1;
	}
	if(thread->joinable==0)
	{
		printf("in join1..");
		thread->waitThreads=(struct threadQueue_t*)malloc(sizeof(struct threadQueue_t));
		thread->waitThreads->tcb=current;
		thread->waitThreads->next=NULL;
		thread->joinable=1;
	}
	else
	{
		struct threadQueue_t *temp,*newnode;
		temp=thread->waitThreads;
		while(temp->next!=NULL)
		{
			temp=temp->next;
		}
		newnode=(struct threadQueue_t*)malloc(sizeof(struct threadQueue_t));
		newnode->tcb=current;
		newnode->next=NULL;
		temp->next=newnode;
	}
	
	(current->join_count)++;
	current->state=WAITING;
	addToQueue(current,&waitQueue);
	FIFOscheduler();
	return 0;
}

void threadExecute(void* (*funct)(void*),void *args)
{
 int x;
 funct(args);
 //at this point thread has completed its execution. If not explicitly killed KILL IT NOW
 if(current->state!=KILLED)
 	my_pthread_exit((void*)&x);
 FIFOscheduler();
}

void FIFOscheduler()
{
	printf("fifo..\n");
	//printQueue(readyQueue);
	struct TCB_struct *execnode,*oldnode;
  	oldnode=current;
  	execnode=dequeue(&readyQueue);
  	if(execnode==NULL)
		return;
   		//exit(0);
  	execnode->state=RUNNING;
  	current=execnode;
	//printf("old:%d %d\n",oldnode->tid,oldnode->state);
	//printf("Active:%d %d\n",current->tid,current->state);
  	int x=swapcontext(&(oldnode->context),&(current->context));
	//printf("after swap ",x);
}

void inline RRScheduler()
{
	//getcontext(&(current->context));
	//test
	//mcontext_t *mcontext=&((ucontext_t*)&(current->context))->uc_mcontext;
	//printf("----%x----\n",mcontext->gregs[REG_RIP]);
	//end test
	printf("RR.. \n");
	//printQueue(readyQueue);
	struct TCB_struct *execnode,*oldnode;
  	oldnode=current;
	oldnode->state=READY;
	addToQueue(oldnode,&readyQueue);
  	execnode=dequeue(&readyQueue);
  	if(execnode==NULL)
   		return;
  	execnode->state=RUNNING;
  	current=execnode;
	//printf("old:%d %d\n",oldnode->tid,oldnode->state);
	//printf("Active:%d %d\n",current->tid,current->state);
  	int x=swapcontext(&(oldnode->context),&(current->context));
	//setcontext(&(current->context));
}

static void preempt(int sig,siginfo_t* siginfo,void *context)
{
	//printf("%x %x %x\n",&my_pthread_begin,&my_pthread_end,&main);
	mcontext_t *mcontext=&((ucontext_t*)context)->uc_mcontext;
	#if defined(__x86_64)
	//printf("--%x\n",mcontext->gregs[REG_RIP]);
	if( ((mcontext->gregs[REG_RIP] > &my_pthread_begin) && (mcontext->gregs[REG_RIP] < &my_pthread_end)) ||
	((mcontext->gregs[REG_RIP] > &my_pthread_end) && (mcontext->gregs[REG_RIP] < &my_pthread_begin)) )
	{
		
		ucontext_t* context1=( ucontext_t *)context;	
		(current->context).uc_link=context1->uc_link;
		(current->context).uc_stack.ss_sp=context1->uc_stack.ss_sp;
		(current->context).uc_stack.ss_size=context1->uc_stack.ss_size;
		(current->context).uc_stack.ss_flags=context1->uc_stack.ss_flags;
		mcontext->gregs[REG_RIP]=RRScheduler;
	}
	#elif defined(__i386)
	//uint32_t *code=(uint32_t*)mcontext->gregs[REG_EIP];
	if( ((mcontext->gregs[REG_EIP] > &my_pthread_begin) && (mcontext->gregs[REG_EIP] < &my_pthread_end)) ||
	((mcontext->gregs[REG_EIP] > &my_pthread_end) && (mcontext->gregs[REG_EIP] < &my_pthread_begin)) )
	{
		
		ucontext_t* context1=( ucontext_t *)context;	
		(current->context).uc_link=context1->uc_link;
		(current->context).uc_stack.ss_sp=context1->uc_stack.ss_sp;
		(current->context).uc_stack.ss_size=context1->uc_stack.ss_size;
		(current->context).uc_stack.ss_flags=context1->uc_stack.ss_flags;
		mcontext->gregs[REG_EIP]=RRScheduler;
	}
	
	#endif
	printf("end...ne");
}

int my_pthread_mutex_init(my_pthread_mutex_t *mutex,
			  const my_pthread_mutexattr_t *mutexattr)
{
	mutex->spin=0;
	mutex->lock=0;
	mutex->kind=1;	
	return 0;
}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
 if(mutex->kind==-1)
	return 1;

 if(!__sync_lock_test_and_set(&(mutex->lock), 1))
	return 0;
 //atomic incr spin 
 __sync_add_and_fetch(&(mutex->spin),1);
 while(1)
 {
  if(!__sync_lock_test_and_set(&(mutex->lock), 1))
  {
	//atomic decr 
	__sync_sub_and_fetch(&(mutex->spin),1);
	return 0;
  }
  //put in wait queue	--
  current->state=WAITING;
  addToQueue(current,&(mutex->mutexWaitQueue));
  FIFOscheduler();
  //syscall(__NR_futex, &(mutex->lock), FUTEX_WAIT, 1, NULL, 0, 0);
  //futex(&(mutex->lock),FUTEX_WAIT,1);
 }
 return 1;
}


int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
	if(mutex->kind==-1)
		return 1;
	__sync_val_compare_and_swap(&(mutex->lock),1,0);
	if(__sync_xor_and_fetch(&(mutex->spin),0)==0) //need to test if spin==0
		return 0;
	//wake all---
	while(1)
	{
	 my_pthread *temp=dequeue(&(mutex->mutexWaitQueue));
	 if(temp==NULL)
		break;
	 temp->state=READY;
	 addToQueue(temp,&readyQueue);
	}	

	//syscall(__NR_futex, &(mutex->lock), FUTEX_WAKE, 0, NULL, 0, 0);
	//futex(&(mutex->lock),FUTEX_WAKE,0);
	return 0;
}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex)
{
	mutex->spin=0;
	mutex->lock=0;
	mutex->kind=-1;
	return 0;
}

int my_pthread_cond_init(my_pthread_cond_t *cond,
			 my_pthread_condattr_t *cond_attr)
{
	cond->mutex=NULL;
}
int my_pthread_cond_signal(my_pthread_cond_t *cond)
{
	my_pthread *temp=dequeue(&(cond->condWaitQueue));
	 if(temp==NULL)
		return 1;
	 temp->state=READY;
	if(cond->mutex->kind==-1)
		return 1;

	if(!__sync_lock_test_and_set(&(cond->mutex->lock), 1))
	{
	}
	printf("lock val: %d\n",cond->mutex->lock);
	 addToQueue(temp,&readyQueue);
	return 0;
}

int my_pthread_cond_broadcast(my_pthread_cond_t *cond)
{
	int givelock=0,res=1;
	while(1)
	{
	 my_pthread *temp=dequeue(&(cond->condWaitQueue));
	 if(temp==NULL)
		break;
	 temp->state=READY;
	 if(givelock==0)
	 {
		//res=my_pthread_mutex_lock(cond->mutex);
		printf("lock val: %d\n",cond->mutex->lock);
		//inline///
		if(cond->mutex->kind==-1)
			return 1;

		 if(!__sync_lock_test_and_set(&(cond->mutex->lock), 1))
		{
		}
		else
		{
		/*
		 //atomic incr spin 
		 __sync_add_and_fetch(&(cond->mutex->spin),1);
		 while(1)
		 {
		  if(!__sync_lock_test_and_set(&(cond->mutex->lock), 1))
		  {
			//atomic decr 
			__sync_sub_and_fetch(&(cond->mutex->spin),1);
			return 0;
		  }
		  //put in wait queue	--
		  current->state=WAITING;
		  addToQueue(current,&(cond->mutex->mutexWaitQueue));
		  //FIFOscheduler();
		  //syscall(__NR_futex, &(mutex->lock), FUTEX_WAIT, 1, NULL, 0, 0);
		  //futex(&(mutex->lock),FUTEX_WAIT,1);
		 }
 		*/
		}
		//end inline//
		givelock=1;
	 }
	 addToQueue(temp,&readyQueue);
	}
	return res;
}

int my_pthread_cond_wait(my_pthread_cond_t *cond, my_pthread_mutex_t *mutex)
{
	int res=1;
	cond->mutex=mutex;
	res=my_pthread_mutex_unlock(mutex);
	if(res==0)
	{
		//go to wait queue
		current->state=WAITING;
  		addToQueue(current,&(cond->condWaitQueue));
		printf("in wait...lock val: %d\n",cond->mutex->lock);
  		FIFOscheduler();
	}
	return res;
}
int my_pthread_cond_destroy(my_pthread_cond_t *cond)
{
 cond->mutex=NULL;
}
#else

/* thread creation */
int my_pthread_create(my_pthread_t *thread, const my_pthread_attr_t *attr,
		      void *(*start_routine) (void *), void *arg)
{
	return pthread_create(thread, attr, start_routine, arg);
}

void my_pthread_exit(void *retval)
{
	pthread_exit(retval);
}

int my_pthread_yield(void)
{
	return pthread_yield();
}

int my_pthread_join(my_pthread_t thread, void **retval)
{
	return pthread_join(thread, retval);
}

/* mutex */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex,
			  const my_pthread_mutexattr_t *mutexattr)
{
	return pthread_mutex_init(mutex, mutexattr);
}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
	return pthread_mutex_lock(mutex);
}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
	return pthread_mutex_unlock(mutex);
}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex)
{
	return pthread_mutex_destroy(mutex);
}

/* conditional variables */
int my_pthread_cond_init(my_pthread_cond_t *cond,
			 my_pthread_condattr_t *cond_attr)
{
	return pthread_cond_init(cond, cond_attr);
}

int my_pthread_cond_signal(my_pthread_cond_t *cond)
{
	return pthread_cond_signal(cond);
}

int my_pthread_cond_broadcast(my_pthread_cond_t *cond)
{
	return pthread_cond_broadcast(cond);
}

int my_pthread_cond_wait(my_pthread_cond_t *cond, my_pthread_mutex_t *mutex)
{
	return pthread_cond_wait(cond, mutex);
}

int my_pthread_cond_destroy(my_pthread_cond_t *cond)
{
	return pthread_cond_destroy(cond);
}

#endif
