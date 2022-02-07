// List all group member's name:Abhishek Jain Swathi gopal
// username of iLab:ilab1.cs
// iLab Server:


#define _XOPEN_SOURCE 600
#include<string.h>
#include "mypthread.h"
#include <sys/time.h>
#include "stdbool.h"
#include <signal.h>
#include <ucontext.h>
#include <stdio.h> 
#include <stdlib.h> 


//##################################################################################

#define STACKCAPACITY 8192
// INITAILIZE ALL YOUR VARIABLES HERE




/* create a new thread */

static struct queue* runQueue = NULL;
static struct queue* runQueue2 = NULL;
static struct queue* runQueue3 = NULL;
static struct queue* runQueue4 = NULL;
struct queue* mutexQueue = NULL;

static bool initialize = false;


struct itimerval *timer;
struct sigaction timerhandler;

static int timeQuantam = 5000;
static int threadCount = 0;
static int mutexCount = 0;
static ucontext_t scheduler_context;
mypthread_mutex_t* mutexes[1000];
static int terminated[1000];
tcb* runningThread;

//static void schedule();

struct queue* getCurrentQueue(){
    struct node *iter;
    tcb* iterThread;
    struct queue* dummy = NULL;
    iter = front(runQueue);
    
    while(iter!=NULL){

      iterThread = (tcb *)iter->item;
//      printf("current thread %d ",iterThread->threadId);
      if (iterThread->state == Ready) {
             return runQueue;
      }
      iter = iter->next;
  }

  iter = front(runQueue2);
  while(iter!=NULL){

      iterThread = (tcb *)iter->item;
//    printf("current thread %d ",iterThread->threadId);
      if (iterThread->state == Ready) {
             return runQueue2;
      }
      iter = iter->next;
  }

  iter = front(runQueue3);
  while(iter!=NULL){

      iterThread = (tcb *)iter->item;
//     printf("current thread %d %d",iterThread->threadId,iterThread->state);
      if (iterThread->state == Ready) {
             return runQueue3;
      }
      iter = iter->next;
  }

  iter = front(runQueue4);
  while(iter!=NULL){

      iterThread = (tcb *)iter->item;
//      printf("current thread %d %d",iterThread->threadId,iterThread->state);
      if (iterThread->state == Ready) {
             return runQueue4;
      }
      iter = iter->next;
  }
  return dummy;

}

static struct queue* getPreviousQueue() {
    if (runningThread->queueNum == 1){
      return runQueue;
    }
    if (runningThread->queueNum == 2){
      return runQueue2;
    }
    if (runningThread->queueNum == 3){
      return runQueue3;
    }
    if (runningThread->queueNum == 4){
      return runQueue4;
    }
    return NULL;
}

static void sched_mlfq() {
 // printf("started MLFQ scheduling ");
  struct queue* currentQueue = getCurrentQueue();
  struct queue* previousQueue = getPreviousQueue();
  timer->it_interval.tv_sec = 0;
  timer->it_interval.tv_usec = 0;
  timer->it_value.tv_sec = 0;
  timer->it_value.tv_usec = 0;
  setitimer(ITIMER_PROF, timer, NULL);

  tcb* currentThread;
  int flag = 0;
  int newTimeQuantam = timeQuantam;
  if (runningThread->state == Terminated){
    deQueue(previousQueue);
  }
  else if(runningThread->state == Ready) {
    enQueue(previousQueue, deQueue(previousQueue));
  }
  else if(runningThread->state == Running) {
    flag = 1;
    runningThread->state = Ready;
    if (runningThread->queueNum == 1){
        runningThread->queueNum = 2;
        enQueue(runQueue2, deQueue(runQueue));   
        }
    else if (runningThread->queueNum == 2){
        runningThread->queueNum = 3;
        enQueue(runQueue3, deQueue(runQueue2));   
        }
    else if (runningThread->queueNum == 3){
        runningThread->queueNum = 4;
        enQueue(runQueue4, deQueue(runQueue3));   
        }
    else if (runningThread->queueNum == 4){
        enQueue(runQueue4, deQueue(runQueue4));   
        }
  }
  tcb* oldThread = runningThread;
    if (currentQueue == NULL){
          return;
        if(runningThread->queueNum == 2)
            currentQueue = runQueue2;
        else if(runningThread->queueNum == 3)
                currentQueue = runQueue3;
        else if(runningThread->queueNum == 4)
                currentQueue = runQueue4;
  }
  struct node* frontNode = front(currentQueue);
  currentThread = (tcb* )(frontNode->item);
  currentThread->state = Running;

  runningThread = currentThread;
  
  if(currentThread->queueNum == 2)
    newTimeQuantam = newTimeQuantam * 2;
  else if(currentThread->queueNum == 3)
    newTimeQuantam = newTimeQuantam * 3;
  else if(currentThread->queueNum == 4)
    newTimeQuantam = newTimeQuantam * 4;
  timer->it_interval.tv_sec = 0;
  timer->it_interval.tv_usec = newTimeQuantam;
  timer->it_value.tv_sec = 0;
  timer->it_value.tv_usec = newTimeQuantam;
  timerhandler.sa_handler = &sched_mlfq;
  setitimer(ITIMER_PROF, timer, NULL);
  sigaction(SIGPROF, &timerhandler, NULL);
  
  if (flag == 0){
   setcontext(&(currentThread->context));
  }
  else{
    if(oldThread->threadId != currentThread->threadId){
     swapcontext(&(oldThread->context), &(currentThread->context));  
    }
    
  }
 // printf("end of MLFQ scheduling ");
}

static void sched_stcf() {
  // Your own implementation of STCF
  // (feel free to modify arguments and return types)

  //printf("started scheduling ");
  tcb* oldThread = NULL;
  tcb* currentThread = NULL;
  int minVal = 0;
  tcb* iterThread;
  struct node *iter;
  timer->it_interval.tv_sec = 0;
  timer->it_interval.tv_usec = 0;
  timer->it_value.tv_sec = 0;
  timer->it_value.tv_usec = 0;
  setitimer(ITIMER_PROF, timer, NULL);

    iter = front(runQueue);
    minVal = ((tcb *)(iter->item))->counter;
    while(iter!=NULL){

      iterThread = (tcb *)iter->item;

      if (iterThread->state == Running){
        oldThread = iterThread;
      }
      if (iterThread->state == Ready && iterThread->counter <= minVal){
        minVal = iterThread->counter;
        currentThread = iterThread;
      }

      iter = iter->next;
    }
// printf("current thread details are %lu ", currentThread->threadId);
  if (oldThread!=NULL){
    oldThread->state = Ready;
  }
  if (currentThread!=NULL){
    currentThread->state = Running;
    currentThread->counter = currentThread->counter + 1;
    runningThread = currentThread;  
  }
  
    timer->it_interval.tv_sec = 0;
    timer->it_interval.tv_usec = timeQuantam;
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = timeQuantam;
    timerhandler.sa_handler = &sched_stcf;
    setitimer(ITIMER_PROF, timer, NULL);
    sigaction(SIGPROF, &timerhandler, NULL);
    if (oldThread == NULL){
//      printf(" SETTING" );
      setcontext(&(currentThread->context));
    }
    else{
      if (currentThread != NULL){
	      //    printf(" SWAPPING ");
      if(swapcontext(&(oldThread->context), &(currentThread->context)) == -1){
        printf("Error  while swapping context\n");
 //       return -1;
        }
      }
    else{
  //    printf(" NIETHER ");
      oldThread->state = Running;
      runningThread = oldThread;
      oldThread->counter = oldThread->counter + 1;
     }
    
    }
  }

/* scheduler */
static void schedule() {
  // Every time when timer interrup happens, your thread library
  // should be contexted switched from thread context to this
  // schedule function

  // Invoke different actual scheduling algorithms
  // according to policy (STCF or MLFQ)

  // if (sched == STCF)
  //     sched_stcf();
  // else if (sched == MLFQ)
  //     sched_mlfq();

 //printf("Inside schedule");
#ifndef MLFQ
//    printf("Inside STCF");
    sched_stcf();
  // Choose STCF
#else
   // printf("Inside MLFQ");
    sched_mlfq();    
  // Choose MLFQ
#endif

}


int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
       // create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
  if (initialize == false) {
    initialize = true;
  //  printf("Hello\n");
    runQueue = createQueue();
    runQueue2 = createQueue();
    runQueue3 = createQueue();
    runQueue4 = createQueue(); 
    if ( getcontext(&scheduler_context) == -1 ) {
      perror("getcontext");
      exit(EXIT_FAILURE);
    }
    
    scheduler_context.uc_stack.ss_sp = malloc(STACKCAPACITY);
    scheduler_context.uc_stack.ss_size = STACKCAPACITY;
    scheduler_context.uc_stack.ss_flags = 0;
    scheduler_context.uc_link = NULL;
    makecontext(&scheduler_context,schedule,0);
    
    tcb *mainThread = (tcb*) malloc (sizeof(tcb));
    mainThread->threadId =  threadCount;
    mainThread->state = Running;
    mainThread->counter = 0;
    mainThread->queueNum = 1;
    if ( getcontext(&(mainThread->context)) == -1 ) {
      perror("getcontext");
      exit(EXIT_FAILURE);
    }
    *thread = threadCount++;
    enQueue(runQueue, (queue_item) mainThread); 
    runningThread = mainThread;
    timer = (struct itimerval*) malloc(sizeof(struct itimerval));
//timerhandler = malloc
    memset(&timerhandler, 0, sizeof(timerhandler));
//    timerhandler.sa_handler = &sched_stcf;  
//  sigaction(SIGPROF, &timerhandler, NULL);
  }
    timer->it_interval.tv_sec = 0;
    timer->it_interval.tv_usec = 0;
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = 0;
    setitimer(ITIMER_PROF, timer, NULL);
 
    tcb *newThread = (tcb*) malloc (sizeof(tcb));
 
    (newThread)->threadId =  threadCount;
    (newThread)->state = Ready;
    (newThread)->counter = 0;
    (newThread)->queueNum = 1;
 
    if (getcontext(&(newThread->context))==-1) {
      perror("getcontext");
      exit(EXIT_FAILURE);
    }
 
    (newThread->context).uc_stack.ss_sp = malloc(STACKCAPACITY);
    (newThread->context).uc_stack.ss_size = STACKCAPACITY;
    (newThread->context).uc_stack.ss_flags = 0;
    (newThread->context).uc_link = &scheduler_context;
 
    if ((newThread->context).uc_stack.ss_sp == NULL) {
      return -1;
    }
   // printf(" new pointer is %p ",(newThread->context).uc_stack.ss_sp);
   // printf("Before make context\n");
   // printf("arg value is %d ",*((int*) arg));
    makecontext(&(newThread->context),function,1,arg);
   // printf("After make context\n");
    enQueue(runQueue, (queue_item) newThread);
  //  printf("After enque\n");
    *thread = threadCount++;
    timer->it_interval.tv_sec = 0;
    timer->it_interval.tv_usec = timeQuantam;
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = timeQuantam;
    timerhandler.sa_handler = &schedule;
    setitimer(ITIMER_PROF, timer, NULL);
    sigaction(SIGPROF, &timerhandler, NULL);
 
    return 0;
}



/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {
 // printf("Entering yield");
//  ucontext_t currentContext;
//  getcontext(&currentContext);
//  currentThread = (tcb* )deQueue(runQueue);
  runningThread->state = Ready;
//  runningThread->context = currentContext; //is this redundant
//  enQueue(runQueue,(queue_item)currentThread);

  if (swapcontext(&(runningThread->context), &scheduler_context) == -1){
    printf("Error  while swapping context\n");
        return -1;

  }
   return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
  // Deallocated any dynamic memory created when starting this thread

  // YOUR CODE HERE
 // printf("ExeThread \n");
  timer->it_interval.tv_sec = 0;
  timer->it_interval.tv_usec = 0;
  timer->it_value.tv_sec = 0;
  timer->it_value.tv_usec = 0;
  setitimer(ITIMER_PROF, timer, NULL);

 //tcb* currentThread;
 // currentThread = (tcb* )(front(runQueue)->item);
  runningThread->state = Terminated;
  //deQueue(runQueue);
//  printf("Exiting a thread pk %lu \n",runningThread->threadId);

  terminated[(int)(runningThread->threadId)] = 1; 
 // printf("Before freeing");
  free(runningThread);
 // printf("After freeing");
  setcontext(&scheduler_context);
  //raise(SIGPROF);
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {
  struct node *iter;
   tcb* iterThread;
  iter = front(runQueue2);
  while(iter!=NULL){

      iterThread = (tcb *)iter->item;
   //   printf("current thread %d %d",iterThread->threadId,iterThread->state);
      if (iterThread->state == Ready) {
             return runQueue2;
      }
      iter = iter->next;
  }
  // printf("Waiting on thread  %lu \n", thread);
  while(1) {
  //printf("still waiting");
        if (terminated[(int)(thread)] == 1){
      break;  
    }
  }

  return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  //initialize data structures for this mutex

  if(mutex != NULL) {
 //   printf("mutex not null");
    mutex->threadId = NULL;
    mutex->mutexid = mutexCount;
    mutexes[mutexCount] = mutex;
    mutexCount = mutexCount + 1;
    return 0;
  }
 // printf("mutex null");
  return -1;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //
        // context switch to the scheduler thread

    while(1){
    if (mutexes[mutex->mutexid]->threadId == NULL){
      mutexes[mutex->mutexid]->threadId = runningThread->threadId;
      break;
       }
    else{
       mypthread_yield();
       }
    
        return 0;
}}

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
  // Release mutex and make it available again.
  // Put threads in block list to run queue
  // so that they could compete for mutex later.

  // YOUR CODE HERE

  mutexes[mutex->mutexid]->threadId = NULL;
  return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
  // Deallocate dynamic memory created in mypthread_mutex_init
//  printf("mutex id is %d ",mutex->mutexid);
  mutexes[mutex->mutexid] = NULL;
//  free(mutex);
  return 0;
};



struct node* newNode(queue_item item) 
{ 
    struct node* temp = (struct node*)malloc(sizeof(struct node)); 
    temp->item = item; 
    temp->next = NULL; 
    return temp; 
} 
  
struct queue* createQueue() 
{ 
    struct queue* q = (struct queue*)malloc(sizeof(struct queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 


void enQueue(struct queue* q, queue_item item) 
{ 
    struct node* temp = newNode(item);  
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    } 
    q->rear->next = temp; 
    q->rear = temp; 
} 
  

queue_item deQueue(struct queue* q) 
{ 
    if (q->front == NULL) 
        return NULL; 
    struct node* temp = q->front; 
    
    q->front = q->front->next; 
    return temp->item;
    
    if (q->front == NULL) 
        q->rear = NULL; 
  
    free(temp); 
    return NULL;
} 

struct node* front(struct queue* q){
  
  return q->front;
}



// int main() {
//   tcb t1,t2;
//   mypthread_init(50000L);
//  // mypthread_init( &t1, thr1, NULL);
//    //mypthread_init( &t1, thr1, NULL);
//   return EXIT_SUCCESS;
// }

// Feel free to add any other functions you need

// YOUR CODE HERE
