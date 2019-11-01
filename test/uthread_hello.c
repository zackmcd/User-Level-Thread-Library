/*
 * Simple hello world test
 *
 * Tests the creation of a single thread and its successful return.
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
int thread2(void* arg)
{
	printf("thread%d\n", uthread_self());
	return 2;
}

int thread1(void* arg)
{
    uthread_t tid;
    int retval;
    tid = uthread_create(thread2, NULL);
	uthread_join(tid,&retval);
	printf("thread%d received retval %d\n", uthread_self(),retval);
	uthread_yield();
	return 0;
}

int main(void)
{
	uthread_t tid;
	tid = uthread_create(thread1, NULL);
	uthread_join(tid, NULL);
	return 0;
}
