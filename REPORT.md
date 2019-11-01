# User-level-thread-lib
project2 for ECS150

This project implements a basic user-level thread library for Linux. In order
to keep track of scheduling and hold all the threads, we created a queue.

In our queue implementation we used a linked list in order to connect all the
different thread objects. This made it easy to be able to enqueue and dequeue
objects into the queue correctly. So in our queue struct we have a head and a
tail and each node points to the next node in line in the queue. Each node in
the queue stores a pointer to the next node and a void pointer to store the
desired data. In our case the date will be pointers to thread objects.

In order to implement the uthread library, we created a thread struct that act
as the TCB. It stores the tid, the state, the context, a pointer to the top of
the stack, the function being used, the argument for the function, and a pointer
to a thread object that it joined from. So if a thread is joined from another
we store that thread in the struct; we decided to do this because when we exit
a thread we need to keep track if it joined from a thread or not. For all of our
uthread library functions we use a global queue and a global thread object that
holds the current thread. This allows us to keep track of scheduling and keep
track of the thread that is currently running.

In our uthread library we have the typical functions such as join, create, exit,
and yield, but we also wrote some helper functions. We wrote a uthread_lib_init
which is called whenever the first thread is created. This establishes the main
thread and starts preemption for all the future threads. We wrote a is_tid
function that just checks if the two pointers passed in have equal TIDs. We use
this to find the thread that the current thread was joined from. So when we
exit, if the thread joined from another then we find that thread in the queue
and move it to the end of the queue. Also we wrote a find_next_ready function
that just finds the next ready thread in the queue. We use this function in
yield when we are switching context between threads.

In our library we handle the context of threads in context.c; this file just
calls functions in order to allocate memory for thread stacks, and switch
contexts when we yield.

For preemption, we install a signal handler for SIGVTALRM. We create a timer
interrupt for 100HZ so whenever a thread runs for the set amount of time it
sends a signal and calls the signal handler. In our signal handler we just
call our uthread yield function so it switches to the next thread that is ready
in the queue. We enable preemption at the beginning of the uthread library
functions but whenever our changing the current thread we disable preemption
so it does not get interrupted by the timer.

To test our queue and thread library we created some programs to check to
make sure everything was working correctly. In test_queue.c we test the 
implementation of our queue by calling enqueue, dequeue, and iterate on
different cases. In uthread_yield.c we test our join and yield functions.
In uthread_hello.c we test our join function more thoroughly and in
test_preempt.c we test our preemption.

Overall, we impliment a queue with a linked list and in each node we store a
thread struct which holds all the necessary information about each individual
thread. The queue schedules all the threads and allows us to keep track of
context and preemption. 
