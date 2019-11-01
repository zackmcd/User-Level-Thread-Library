#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

#define MAXTID 65535

enum State{
	RUNNING,
	READY,
	BLOCKED,
	EXITED
};

typedef struct tcb{
	uthread_t tid;
	enum State state;
	uthread_ctx_t ctx;
	void *top_of_stack;
	uthread_func_t func;//function to execute
	void *arg;//arguments of function
	int retval;
	uthread_t joinfrom_tid;
}tcb,*tcb_t;

queue_t uthread_lib;
tcb_t current_thread;

static int is_tid(void *data, void *arg);
int uthread_lib_init(void);
void uthread_destroy(tcb_t tcb);
static int printid(void *data, void *arg);
void print_all();

int uthread_lib_init(void)
{
  if(uthread_lib!=NULL) return 0;
  uthread_lib = queue_create();
	
  //create the mainthread
  tcb_t temp = (tcb_t)malloc(sizeof(tcb));
	
  //memory allocation failure
  if(!temp) return -1; 
  
  temp->tid=0;
  temp->state = RUNNING;
  current_thread = temp;

  preempt_start(); // start preemption

  return 0;
}

static int is_tid(void *data, void *arg) // checks if the TIDs match for a temp thread and the current thread
{ 
  tcb_t tcb = (tcb_t)data;
  uthread_t tid= *(uthread_t*)arg;
  if(tcb->tid == tid) return 1;
  else return 0;
}

static int printid(void *data, void *arg) // print function used to help debug
{
  tcb_t tcb = (tcb_t)data;
  printf("%d and state is %d",(int)(tcb->tid),tcb->state);
  return 0;
}

void print_all() // print function used to help debug
{
  printf("\n");
  queue_iterate(uthread_lib,printid,NULL,NULL);
  printf("\n");
}

tcb_t find_next_ready(){
  
  // function is used to find the next thread that is READY
  // and returns it to the yield function in order to 
  // switch to the proper context

  tcb_t tcb;
  int flag;
  int count =0;
  int len = queue_length(uthread_lib);
  flag = queue_dequeue(uthread_lib,(void **)&tcb);
	
  while(flag ==0 && tcb->state!=READY && count<len)
  {
    queue_enqueue(uthread_lib,(void *)tcb);
    flag = queue_dequeue(uthread_lib,(void **)&tcb);
    count ++;
  }
  return tcb;
}

void uthread_destroy(tcb_t tcb)
{
  // frees the thread table
  uthread_ctx_destroy_stack(tcb->top_of_stack);
  free(tcb);
}

void uthread_yield(void)
{
  preempt_enable(); // start preemption
  preempt_disable();

  if(current_thread->state!=EXITED)
  {
    // puts current thread at end of queue since we are switching to the next ready thread
    queue_enqueue(uthread_lib,(void *)current_thread); 
  }                                                                            
  tcb_t next_ready = find_next_ready(); // finds the next ready thread
	
  if(next_ready==NULL||next_ready->tid==current_thread->tid) 
  {
    preempt_enable();
    return;// no other thread ready
  }

  tcb_t prev = current_thread;
  if(prev->state!=BLOCKED && prev->state!=EXITED) prev->state=READY;
  next_ready->state = RUNNING; // sets new current thread to running
  current_thread=next_ready;
  uthread_ctx_switch(&(prev->ctx), &(next_ready->ctx)); // switches context registers
	
  preempt_enable(); // enables preemption again
}

uthread_t uthread_self(void)
{
  // returns the tid of a thread
  preempt_enable();
  return current_thread->tid;
}

int uthread_create(uthread_func_t func, void *arg)
{
  // this function creates a new thread and puts it in the queue
  if(!uthread_lib) uthread_lib_init();

  preempt_enable(); 
  preempt_disable();
	
  tcb_t temp = (tcb_t)malloc(sizeof(tcb)); // creats temp thread
	
  if(!temp) 
  {
    preempt_enable();
    return-1; //memory allocation failure
  }
	
  if(queue_length(uthread_lib)+1>MAXTID) 
  {
    preempt_enable();
    return-1;// tid overflow
  }
	
  // assigns appropriate info to the tcb
  temp->tid=(unsigned short)(queue_length(uthread_lib)+1);
  temp->joinfrom_tid=temp->tid; //initialized as an impossible value
  temp->state = READY;
  temp->top_of_stack = uthread_ctx_alloc_stack();
	
  if(!(temp->top_of_stack)) 
  {
    preempt_enable();
    return-1;//memory allocation failure
  }
	
  temp->func=func;
  temp->arg=arg;	
	
  if(uthread_ctx_init(&(temp->ctx), temp->top_of_stack,temp->func,temp->arg)==-1)
  {
    preempt_enable();
    return-1;//context creation failure
  }
	
  queue_enqueue(uthread_lib,(void*)temp); // places new thread into queue
  preempt_enable();
  return temp->tid;
}

void uthread_exit(int retval)
{
  preempt_enable();
  preempt_disable();
  current_thread->retval = retval;
  current_thread->state = EXITED; // current thread is set to EXITED
  if(current_thread->joinfrom_tid!=current_thread->tid)
  { // means that this was joined from somewhere
    tcb_t joinfrom;
    queue_iterate(uthread_lib,is_tid,(void*)(&current_thread->joinfrom_tid),(void**)&joinfrom);
    joinfrom->state =READY; // unblock T2 and schedule T2 after all the currently runnable threads
    queue_delete(uthread_lib,(void*)joinfrom);
    queue_enqueue(uthread_lib,(void*)joinfrom);		
  }
  queue_delete(uthread_lib,(void*)current_thread); // removes current thread
  uthread_yield(); // goes to yeild to change to the next thread
  preempt_enable();
}

int uthread_join(uthread_t tid, int *retval)
{
  preempt_enable();
  preempt_disable();
	
  if(tid==0) 
  {
    preempt_enable();
    return -1;// the 'main' thread cannot be joined
  }

  if(current_thread->tid==tid) 
  {
    preempt_enable();
    return -1;// tid is the TID of the calling thread
  }

  tcb_t rst;
  queue_iterate(uthread_lib,is_tid,(void*)(&tid),(void**)&rst);
	
  if(rst==NULL) 
  {
    preempt_enable();
    return -1;// thread @tid cannot be found
  }
  else if(rst->joinfrom_tid!=rst->tid) 	
  {
    preempt_enable();
    return -1;// thread @tid is already being joined
  }
  else if(rst->state==READY)
  { // T2 is still an active thread, T1 must be blocked
    rst->joinfrom_tid=current_thread->tid;
    current_thread->state=BLOCKED;
    uthread_yield();
  }
	
  // when T2 is unblocked, it should come back here
  
  if(rst->state==EXITED) 
  { //  T2 is already dead, T1 can collect T2 right away
    if(retval!=NULL) *retval = rst->retval;
    uthread_destroy(rst);
  }

  preempt_enable();
  return 0;	
}
