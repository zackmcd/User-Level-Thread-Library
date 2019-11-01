#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */

#define HZ 100
#define MICRO 10000

struct sigaction action;
struct itimerval timer;

void sigvt_handler(int signum) 
{
  uthread_yield(); // forces the current process to yield
}

void preempt_disable(void)
{
  sigprocmask(SIG_BLOCK, &action.sa_mask, NULL); // blocks the sigvtalrm
}

void preempt_enable(void)
{
  sigprocmask(SIG_UNBLOCK, &action.sa_mask, NULL); // enables the timer interrupt
}

void preempt_start(void)
{
  action.sa_handler = sigvt_handler;
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, SIGVTALRM);
  action.sa_flags = 0;
  
  sigaction(SIGVTALRM, &action, NULL); // creates signal handler

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = MICRO;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = MICRO;

  setitimer(ITIMER_VIRTUAL, &timer, NULL); // starts timer
}
