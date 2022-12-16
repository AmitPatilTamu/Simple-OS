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
#include "blocking_disk.H"
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
	this->BD = NULL;
	Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
	if (BD!=NULL && BD->BD_queue_size!=0 && BD->is_ready()) {
		Thread* candidate = BD->BD_queue.dequeue();
		BD->BD_queue_size--;
                Thread::dispatch_to(candidate);
	}
	else {
	if(queue_size == 0) {
	} else {
		queue_size--;
		Thread* candidate = thread_queue.dequeue();
		Thread::dispatch_to(candidate);
	}
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

void Scheduler::add_BD(BlockingDisk *BD_disk) {
	BD = BD_disk;
}
