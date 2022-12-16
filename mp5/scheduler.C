/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
	queue_size = 0;
	Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
	if(queue_size == 0) {
		Console::puts("No new thread available to dispatch\n");
	} else {
		queue_size--;
		Thread* candidate = thread_queue.dequeue();
		Thread::dispatch_to(candidate);
		//Console::puts("queue_size = ");Console::puti(queue_size);Console::puts("\n");
	}

}

void Scheduler::resume(Thread * _thread) {
        thread_queue.enqueue(_thread);
        queue_size++;
}

void Scheduler::add(Thread * _thread) {
	thread_queue.enqueue(_thread);
	queue_size++;
}

void Scheduler::terminate(Thread * _thread) {
	for(int i = 0;i <queue_size;i++) {
		Thread* candidate = thread_queue.dequeue();
	if (candidate->ThreadId() == _thread->ThreadId()) {	
		queue_size--;
	} else {
		thread_queue.enqueue(candidate);
	}
}
}
