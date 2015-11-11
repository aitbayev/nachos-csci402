// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.



#ifndef SYSTEM_H
#define SYSTEM_H
#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"


class AddrSpace;
using namespace std;






// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Lock *processLock;
extern int threadCounter;
extern int processCounter;


extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock


#ifdef USER_PROGRAM
#include "machine.h"


extern Machine* machine;	// user program memory and registers

//lock code
struct KernelLock{
	Lock *lock;
	AddrSpace *addrSpace;
	bool isToBeDeleted;
	int counter;
};

extern KernelLock locks[];
extern Lock *locksTableLock;
extern int lockIndex;
extern int maxLockTableSize;

//condition code
struct KernelCV{
	Condition *condition;
	AddrSpace *addrSpace;
	bool isToBeDeleted;
	int counter;
};


struct MV{
	char *name;
	int ID;
	int value;

};

struct ServerLock{
	char *name;
	int state; //1 if available 
	int machineID;
	int mailbox;
	List *lockWaitQueue;
	int usage_counter;
	bool isToBeDeleted;
	int id;
	bool deleted;
};


struct ServerCV{
	char *name;
	int serverConditionLock; //index to the array
	List *cvWaitQueue;
	int machineID;
	int mailbox;
	bool isToBeDeleted;
	int usage_counter;
	int id;
	int deleted;
};

struct WaitingClient{
	int machineID;
	int mailbox;
};

extern MV MVs[100];
extern int mv_index;
extern ServerLock server_locks[100];
extern ServerCV server_cvs[100];
extern int server_lock_counter;
extern int server_cv_counter;

extern KernelCV conditions[];
extern Lock *conditionsTableLock;
extern int conditionIndex;
extern int maxConditionTableSize;

#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
