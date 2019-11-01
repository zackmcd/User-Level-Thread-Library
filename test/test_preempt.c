#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void* arg)
{
	printf("thread3\n");
	return 0;
}

int thread2(void* arg)
{
	//this loop will cause it to go into the signal handler
	// it switches threads showing that preemption works but 
	// thread 2 will never finish
	// i couldnt think of any other way to test it
	//while (1) {} // to test preemption
	
	uthread_join(uthread_create(thread3, NULL), NULL);
	return 0;
}

int thread1(void* arg)
{
	uthread_join(uthread_create(thread2, NULL), NULL);
	return 0;
}

int main(void)
{
	uthread_join(uthread_create(thread1, NULL), NULL);
	return 0;
}
