// List all group member's name:Abhishek Jain Swathi gopal
// username of iLab:ilab1.cs
// iLab Server:

#define _XOPEN_SOURCE 600

#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H
#ifndef Queue_H
#define Queue_H


#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_MYTHREAD macro */
//#define USE_MYTHREAD 1
#include <ucontext.h>

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>


typedef unsigned int mypthread_t;

typedef enum threadstate {
	Ready,
	Running,
	Blocked,
	Terminated

 } threadstate;

typedef struct threadControlBlock {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
	mypthread_t threadId;
	ucontext_t context;
	threadstate state;
	int queueNum;
	int counter;


} tcb;

/* mutex struct definition */
typedef struct mypthread_mutex_t {
	int mutexid;
	mypthread_t threadId;
	
	/* add something here */

	// YOUR CODE HERE
} mypthread_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE

typedef void* queue_item;

struct node { 
    queue_item item; 
    struct node* next; 
}; 
  
struct queue { 
    struct node *front, *rear; 
};  


struct queue* createQueue();

int steque_size(struct queue* que);


void enQueue(struct queue* que, queue_item item);


queue_item deQueue(struct queue* que);

struct node* front(struct queue* q);



#endif
/* Function Declarations: */

/* create a new thread */
// int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void
    // *(*function)(void*), void * arg);

int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);


/* give CPU pocession to other user level threads voluntarily */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *value_ptr);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initial the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex);

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex);

/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex);

#ifdef USE_MYTHREAD
#define pthread_t mypthread_t
#define pthread_mutex_t mypthread_mutex_t
#define pthread_create mypthread_create
#define pthread_exit mypthread_exit
#define pthread_join mypthread_join
#define pthread_mutex_init mypthread_mutex_init
#define pthread_mutex_lock mypthread_mutex_lock
#define pthread_mutex_unlock mypthread_mutex_unlock
#define pthread_mutex_destroy mypthread_mutex_destroy
#endif

#endif
