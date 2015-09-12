// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include <iostream>
using namespace std;

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
	name = debugName;
	waitQueue = new List;
	status = FREE;
	lockOwner = NULL;
}
Lock::~Lock() {
	delete waitQueue;	
}

bool Lock::isHeldByCurrentThread(){
	if (lockOwner == currentThread){
		return true;
	}
	else{
		return false;
	}
}

void Lock::Acquire() {
	IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts

	if(isHeldByCurrentThread()){ //if current thread is a lock owner
		(void) interrupt->SetLevel(oldLevel); //restore interrupts
		return;
	}
	if (status == FREE){ //if lock is available
		status = BUSY; //make it unavailable
		lockOwner = currentThread; //make current thread the owner	
	}
	else{
		waitQueue->Append((void *)currentThread);
		currentThread->Sleep();
	}

	(void) interrupt->SetLevel(oldLevel);	// restore interrupts
}

void Lock::Release() {
	Thread *thread;
	IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts

	if(!isHeldByCurrentThread()){
		cout<< currentThread->getName() <<" thread is not lock owner";
		(void) interrupt->SetLevel(oldLevel);	// restore interrupts
		return;
	}
	if (!waitQueue->IsEmpty()){
		thread = (Thread *)waitQueue->Remove(); //remove a thread from waitQueue
		lockOwner = thread; //Make it lock owner
		scheduler->ReadyToRun(thread); //wakeup the thread
	} else{
		status = FREE; //lock is available
		lockOwner = NULL; // resetting lockowner to null
	}
	(void) interrupt->SetLevel(oldLevel);	// restore interrupts
}

Condition::Condition(char* debugName) { 
	name = debugName;
	queue = new List;
	waitingLock = new Lock;
}

Condition::~Condition() { 
	delete queue;
}
void Condition::Wait(Lock* conditionLock) { //ASSERT(FALSE); 
	IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
	if (conditionLock == null){
		cout << "lock equals null"; //??
		(void)interrupt->SetLevel(oldLevel); //re-enable interrupt
		return
	}
	if (waitingLock == null){ //no one waiting
		waitingLock = conditionLock;
	}
	if (waitingLock != conditionLock)
	{
		cout << "waiting lock does not equal lock";  //??
		(void)interrupt->SetLevel(oldLevel); //re-enalbe interrupt
		return
	}
	queue->Append((void *)currentThread);
	conditionLock->Release();
	currentThread->Sleep(); 
	conditionLock->Acquire();
	(void)interrupt->SetLevel(oldLevel); //re-enable interrupt
}

void Condition::Signal(Lock* conditionLock) { 
	Thread *thread;
	IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
	if (waitingLock == null){
		(void)interrupt->SetLevel(oldLevel); //re-enable interrupts
		return
	}
	if (waitingLock != conditionLock)
	{
		cout << " Error: conditionLock does not equal to waitingLock"; //??
		(void)interrupt->SetLevel(oldLevel); //re-enable interrupt
		return
	}
	//wakeup 1 waiting thread??
	thread = (Thread *)queue->Remove(); //remove a thread from waitQueue
	scheduler->ReadyToRun(thread); //put on readyQueue
	if (!queue->IsEmpty())
	{
		waitingLock = null;
	}
	(void)interrupt->SetLevel(oldLevel); //re-enable interrupts
}

void Condition::Broadcast(Lock* conditionLock) { 
	IntStatus oldLevel = interupt->SetLevel(IntOff); //disable interrupts
	if (conditionLock == null){
		(void)interrupt->SetLevel(oldLevel); //re-enable interrupts
		return
	}
	if (conditionLock != waitingLock){
		cout << "conditionLock does not equal to waitingLock"; //??
		(void)interrupt->SetLevel(oldLevel); //re-enable interrupts
		return
	}
	(void)interrupt->SetLevel(oldLevel);	// re-enable interrupts
	while (!queue->IsEmpty()){
		Signal(conditionLock);
	}
}
