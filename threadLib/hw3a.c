
#include "mypthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <syscall.h>
//#include<linux/futex.h>

void my_pthread_begin() {} /* no code before this */

/* Insert code here */
/*
my_pthread_mutex_t count_lock; 
my_pthread_cond_t count_nonzero; 
unsigned count=0;
void decrement_count() 
   { my_pthread_mutex_lock(&count_lock); 

     while (count == 0) 
        my_pthread_cond_wait(&count_nonzero, &count_lock); 
     count = count - 1; 
     my_pthread_mutex_unlock(&count_lock); 
   } 

void increment_count() 
   { my_pthread_mutex_lock(&count_lock); 
     if (count == 0)
       my_pthread_cond_broadcast(&count_nonzero); 
     count = count + 1;
     my_pthread_mutex_unlock(&count_lock); 
    }

//int count;
my_pthread_mutex_t mutest;
void* func1(void *ptr)
{
 int res;
 //res=my_pthread_yield();
 //mypthread_t thread1;
 //count=0;
 
//my_pthread_mutex_lock(&mutest);
 printf("in .. thread\n");
decrement_count();
my_pthread_yield();
//my_pthread_mutex_unlock(&mutest);
 //syscall(__NR_futex, &(count), FUTEX_WAIT, 0, NULL, 0, 0);
 //printf("back thread\n");
 //res=mypthread_create(&thread1,func1,NULL);
 //printf(" %d ",res);
 
 printf("t1 out..");
}

void* func3(void *ptr)
{
 //int res;
 //mypthread_t thread1;
 //syscall(__NR_futex, &(count), FUTEX_WAKE, 2, NULL, 0, 0);
 printf("thread3\n");
increment_count();
 //res=mypthread_create(&thread1,func1,NULL);
 //printf(" %d ",res);
 my_pthread_exit(NULL);
}

*/

////origi/////

int n;
int *data;
my_pthread_mutex_t *mutex;
my_pthread_mutex_t count;
my_pthread_cond_t cond;
int isSorted = 0;
int counter=0;

void* sortdata(void *threadIDptr)
{
 int i=0;
 int threadID = *((int*)threadIDptr);
 while(isSorted!=1)
 {	i++;
	 printf("\ntid: %d isSorted:%d",threadID,isSorted);
	 my_pthread_mutex_lock(&mutex[threadID]);
	 my_pthread_mutex_lock(&mutex[threadID+1]);
	 if(data[threadID] > data[threadID+1])
	 {
	  int temp=data[threadID];
	  data[threadID]=data[threadID+1];
	  data[threadID+1]=temp;
	 }
	 my_pthread_mutex_unlock(&mutex[threadID]);
	 my_pthread_mutex_unlock(&mutex[threadID+1]);
	my_pthread_mutex_lock(&count);
	counter++;
	printf("%d %d\n",threadID,counter);
	if(counter==i*(n-1) || isSorted==1)
	{
		my_pthread_cond_broadcast(&cond);
	//	counter=0;
	}
	else
	{
		while(counter<i*(n-1) && isSorted==0 )
		{
			my_pthread_cond_wait(&cond,&count);
		}	
	}
	my_pthread_mutex_unlock(&count);
	
 }
printf("out %d\n",threadID);
}

void* checkSort(void *ptr)
{
	int i=0;
	int boolsort;
	while(1)
	{
		printf("check %d\n",isSorted);
		boolsort=1;
	 	for(i=0;i<n-1;i++)
		if(data[i]>data[i+1])
			boolsort=0;
	 	isSorted=boolsort;
	 	if(isSorted==1)
		{
	  	my_pthread_exit(NULL);
	 	break;
		} 
		else
	 	{
	  	//sleep(2);
		my_pthread_yield();
	 	}
	}
}


////end origi///////
int main(int argc, char *argv[])
{
/*
	printf("HW3 CS519\n");
	int res;
	 my_pthread_t thread1,thread2,thread3;
	 //threadLib_init();
	my_pthread_mutex_init(&count_lock,NULL);
	 res=my_pthread_create(&thread1,NULL,func1,NULL);
	printf("main");
	sleep(5);
	 printf("%d \n",res);
	 //res=my_pthread_create(&thread2,NULL,func1,NULL);
	 res=my_pthread_create(&thread3,NULL,func3,NULL);
	res=my_pthread_join(thread1,NULL);
	//res=my_pthread_join(thread2,NULL);
	res=my_pthread_join(thread3,NULL);
	 printf("%d \n",res); 
	 //FIFOscheduler();
	 printf("out");
*/
///Origi program/////

 int i;
 my_pthread_t *threads,checkthread;
 int *threadids;
 if(argc < 2)
 {
  printf("\nPlease enter elements to be sorted..\n");
  exit(0);
 }
 n = argc-1;
 data = (int*)malloc(sizeof(int)*n);
 for(i=0;i<n;i++)
 {
  data[i]=atoi(argv[i+1]);
 }
 //create threads
 threads=(my_pthread_t*)malloc(sizeof(my_pthread_t)*(n-1));
 threadids=(int*)malloc(sizeof(int)*(n-1));
 mutex=(my_pthread_mutex_t*)malloc(sizeof(my_pthread_mutex_t)*n);
 for(i=0;i<n;i++)
  my_pthread_mutex_init(&mutex[i],NULL);
 //pthread_barrier_init(&phaseBarrier,0,n-1);
 my_pthread_mutex_init(&count,NULL);
 my_pthread_cond_init(&cond,NULL);
 for(i=0;i<n-1;i++)
 {
   threadids[i]=i;
   my_pthread_create(&threads[i],NULL,sortdata,(void*)&threadids[i]);
 }
 my_pthread_create(&checkthread,NULL,checkSort,(void*)NULL);
 for(i=0;i<n-1;i++)
 {
   my_pthread_join(threads[i],NULL);
 }
 my_pthread_join(checkthread,NULL);
 printf("\nIn main..\nSorted data...\n");
 for(i=0;i<n;i++)
  printf("%d ",data[i]);
 printf("\n");

///end origi program////
	return 0;
}

/* Insert code here */

void my_pthread_end() {} /* no code after this */
