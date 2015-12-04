// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
//#include "../test/setup.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "../network/post.h"
#include <time.h>

using namespace std;


int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    	printf("%s","Bad pointer passed to to write: data not written\n");
	   		delete[] buf;
	   		return;
		}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

//exit() syscall
void Exit_Syscall(){

	processLock -> Acquire();
	// 1. Thread calls Exit (not the last executing thread in process)
	// 2. Last executing thread in last process
	// 3. Last executing thread in a process- not last process and still left in waitQueue
	if(threadCounter == 1 && processCounter > 1){
		currentThread -> space = NULL;
		processCounter--;
		
		for(unsigned int i=0; i < currentThread -> space -> getNumPages(); i++){
			delete currentThread -> space;
		}
		
		processLock -> Release();
		currentThread -> Finish();
	}
	else if(threadCounter == 1 && processCounter == 1){
		for(unsigned int i=0; i < currentThread -> space -> getNumPages(); i++){
			delete currentThread -> space;
		}
		
		processLock -> Release();
		interrupt->Halt();
	}
	else{
		threadCounter--;
		processLock -> Release();
		currentThread -> Finish();
	}
	
	currentThread->Finish();
}


void Yield_Syscall(){
	currentThread->Yield();
}

//create lock syscall implementation
int CreateLock_Syscall(unsigned int name, int len){
// #ifdef NETWORK
	//cout<<"In Create Lock"<<endl;
	char *buf = new char[len+1];
	
		if (len < 1 || len >30){
 		cout<<"Error: name is too long"<<endl;
 		return -1;
 	}
	copyin(name, len, buf);
	stringstream ss;
     
    ss<<"CL "<<buf;
 	char *msg = new char[40];
 	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int index = atoi(buffer);
    
    if (index == -1){
    	printf("ERROR! Couldn't create lock with name %s \n", buf);
    	}
    else{
    	printf("Lock with name %s at index %d was created/retrieved \n", buf, index);
    }
    fflush(stdout);
    
    return index;
    
//#else	
// 	char *buf = new char[len+1];	// Kernel buffer to put the name in
// 
// 	 if ( !(buf = new char[len]) ) {
// 		printf("%s","Error allocating kernel buffer for create lock!\n");
// 		return -1;
//     } else {
//         if ( copyin(name,len,buf) == -1 ) {
// 	   		printf("%s","Bad pointer passed to to create: data not written\n");
// 	    	delete[] buf;
// 	    	return -1;
// 		}
//     }
//     locksTableLock->Acquire();
//  	if (len > 0  && lockIndex < maxLockTableSize && len < 40){
//  		
//  		locks[lockIndex].lock = new Lock(buf);
//  		locks[lockIndex].addrSpace = currentThread->space;
//  		
//  		locks[lockIndex].isToBeDeleted = false;
//  		locks[lockIndex].counter = 0;
//  		
//  		int index = lockIndex;
//  		lockIndex++;
//  		
//  		cout<<"Lock "<<locks[index].lock->getName()<<", with index "<<index<<" was created"<<endl;
//  		locksTableLock->Release();
//  		return index;
//  	
//  	}
//  	else if (lockIndex >= maxLockTableSize){
//  		cout<<"Lock table is full"<<endl;
//  		locksTableLock->Release();
//  		return -1;
//  	}
//  	else if(len <= 0){
//  		cout<<"Invalid lock name: lock name is too short"<<endl;
//  		locksTableLock->Release();
//  		return -1;
//  	}
//  	else if(len>=40){
//  		cout<<"Invalid lock name: name is too long"<<endl;
//  		locksTableLock->Release();
//  		return -1;
//  	}
//  	return -1;
//#endif
}

//destroy lock syscall implementation
void DestroyLock_Syscall(int index){
	
	stringstream ss;
    
    cout<<"In Destroy Lock"<<endl;
     
    ss<<"DL "<<index;
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int result = atoi(buffer);
    
    if (result == -1){
    	printf("Couldn't destroy lock with index %d \n", index);
    }
    else{
    	printf("Destroyed lock with index %d \n", index);
    }
    fflush(stdout);	
    
   
	// locksTableLock->Acquire();
// 	if (index<0 || index >= maxLockTableSize){
// 		cout<<"Index is out of scope"<<endl;
// 		locksTableLock->Release();
// 		return;
// 	}
// 	else{
// 		if (locks[index].lock == NULL){
// 			cout<<"No lock at given location "<< index <<endl;
// 			locksTableLock->Release();
// 			return;
// 		}
// 		locksTableLock->Acquire();
// 		if (locks[index].counter > 0){
// 			locks[index].isToBeDeleted = true;
// 			locksTableLock->Release();
// 			cout<<"Cannot delete. Lock is busy"<<endl;
// 			return;
// 		}
// 		if (locks[index].counter == 0 || locks[index].isToBeDeleted == true){
// 			cout<<"Lock "<<locks[index].lock->getName()<<" at index "<< index <<" is deleted"<<endl;
// 			delete locks[index].lock;
// 			locks[index].lock = NULL;
// 			locksTableLock->Release();
// 		}
// 	}

}

//Acquire lock syscall implementation

void Acquire_Syscall(int index){

	stringstream ss;
    
    //cout<<"In Acquire Lock"<<endl;
     
    ss<<"AL "<<index;
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int result = atoi(buffer);
    if (result == 1){
    	printf("Acquired a lock with index %d \n", index);

    }
    /*else if (result == -2){
    	//currentThread->Sleep();
    	printf("Couldn't acquire a lock with index %d- asleep(lock is busy) \n", index);
    }
    else{
    	printf("Couldn't acquire a lock with index %d \n", index);
    }*/
    fflush(stdout);	

	// locksTableLock->Acquire();
// 
// 	if (index < 0  || index >= maxLockTableSize){
// 		cout<<"Invalid index "<<endl;
// 		locksTableLock->Release();
// 		return;
// 	}
// 	else{
// 		if (locks[index].lock == NULL){
// 			cout<<"No lock at given location"<<endl;
// 			locksTableLock->Release();
// 			return;
// 		}
// 		if (locks[index].addrSpace == currentThread->space){
// 			cout<<"Acquired "<<locks[index].lock->getName()<<" lock with index "<<index<<endl;
// 			locks[index].counter++;
// 			locksTableLock->Release();
// 			locks[index].lock->Acquire();
// 		}
// 		else{
// 			cout<<"the thread doesn't belong to the process"<<endl;
// 			locksTableLock->Release();
// 			return;
// 		}
// 	}
}

void Release_Syscall(int index){

	stringstream ss;
    
    //cout<<"In Release lock"<<endl;
     
    ss<<"RL "<<index;
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    
    
    int result = atoi(buffer);
    if (result == -1){
    	printf("Couldn't release a lock with index %d \n", index); //uncomment after testing
    }
    else if (result == 2){
    	printf("Lock %d was deleted \n", index);
    }
    else if (result == -2){
    	printf("Woke up waiting thread and gave it Lock %d \n", index);
    }
    else {
    	printf("Released Lock with index %d \n", index);
    }
   
    fflush(stdout);	

// 	locksTableLock->Acquire();
// 	if (index < 0 || index >= maxLockTableSize){
// 		cout<<"Invalid index"<<endl;
// 		locksTableLock->Release();
// 		return;
// 	}
// 	else{
// 		if (locks[index].lock == NULL){
// 			cout<<"No lock at given location"<<endl;
// 			locksTableLock->Release();
// 			return;
// 		}
// 		if (locks[index].addrSpace == currentThread->space){
// 			cout<<"Released "<<locks[index].lock->getName()<<" lock at index "<<index<<endl;
// 			locks[index].counter--;
// 			locksTableLock->Release();
// 			locks[index].lock->Release();
// 			if (locks[index].isToBeDeleted == true){
// 				DestroyLock_Syscall(index);
// 			}
// 			return;
// 		}
// 		else{
// 			cout<<" the thread doesn't belong to the process"<<endl;
// 			locksTableLock->Release();
// 			return;
// 		}
// 	
// 	}

}

//create codition syscall implementation
int CreateCondition_Syscall(unsigned int name, int len){
	char *buf = new char[len+1];
	if (len < 1 || len >30){
 		cout<<"Error: name is too long"<<endl;
 		return -1;
 	}
	copyin(name, len, buf);
	stringstream ss;
    
    //cout<<"In create CV"<<endl;
     
    ss<<"CC "<<buf;
    char *msg = new char[40];
 	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int index = atoi(buffer);
    if (index == -1){
    	printf("ERROR! Couldn't create CV with name %s \n", buf);
    }
    else{
    	printf("CV with name %s at index %d was created/retrieved \n", buf, index);
    }
    fflush(stdout);	
    
    return index;
    

	// char *buf = new char[len+1];	// Kernel buffer to put the name in
// 
// 	 if ( !(buf = new char[len]) ) {
// 		printf("%s","Error allocating kernel buffer for create lock!\n");
// 		return -1;
//     } else {
//         if ( copyin(name,len,buf) == -1 ) {
// 	   		printf("%s","Bad pointer passed to to create: data not written\n");
// 	    	delete[] buf;
// 	    	return -1;
// 		}
//     }
//     conditionsTableLock->Acquire();
//  	if (len > 0  && conditionIndex < maxConditionTableSize && len < 40){	
//  		conditions[conditionIndex].condition = new Condition(buf);
//  		conditions[conditionIndex].addrSpace = currentThread->space;
//  		conditions[conditionIndex].isToBeDeleted = false;
//  		conditions[conditionIndex].counter = 0;
//  		 		
//  		int index = conditionIndex;
//  		conditionIndex++;
//  		
//  		cout<<"Condition variable "<<conditions[index].condition->getName()<<", with index "<<index<<" was created"<<endl;
//  		conditionsTableLock->Release();
//  		return index;
//  	
//  	}
//  	else if (conditionIndex >= maxConditionTableSize){
//  		cout<<"Condition table is full"<<endl;
//  		conditionsTableLock->Release();
//  		return -1;
//  	}
// 
//  	else if(len <= 0){
//  		cout<<"Invalid CV name: CV name is too short"<<endl;
//  		conditionsTableLock->Release();
//  		return -1;
//  	}
//  	else if(len>=40){
//  		cout<<"Invalid CV name: CV is too long"<<endl;
//  		conditionsTableLock->Release();
//  		return -1;
//  	}
//  	return -1;
}


void DestroyCondition_Syscall(int index){
	stringstream ss;
    
    cout<<"In destroy CV"<<endl;
     
    ss<<"DC "<<index;
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int result = atoi(buffer);
    
    if (result == -1){
    	printf("Couldn't destroy CV with index %d \n", index);
    }
    else{
    	printf("Destroyed CV with index %d \n", index);
    }
    fflush(stdout);	

	// conditionsTableLock->Acquire();
// 	if (index<0 || index >= maxConditionTableSize){
// 		cout<<"Invalid condition variable index"<<endl;
// 		conditionsTableLock->Release();
// 		return;
// 	}
// 	else{
// 		if (conditions[index].condition == NULL){
// 			cout<<"No condition variable at given location"<<endl;
// 			conditionsTableLock->Release();
// 			return;
// 		}
// 		if (conditions[index].counter > 0){
// 			conditions[index].isToBeDeleted = true;
// 			conditionsTableLock->Release();
// 			cout<<"Condition variable is being used, has threads on its waiting list"<<endl;
// 			return;
// 		}
// 		if (conditions[index].counter == 0 || conditions[index].isToBeDeleted == true){
// 			cout<<"Condition "<<conditions[index].condition->getName()<<" is deleted"<<endl;
// 			delete conditions[index].condition;
// 			conditions[index].condition = NULL;			
// 			conditionsTableLock->Release();
// 		}
// 	}
}
// Wait Condition syscall

void Wait_Syscall(int lk, int cv){
	stringstream ss;
    
    cout<<"In Wait CV"<<endl;
     
    ss<<"WC "<<lk<<" "<<cv;
    
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int result = atoi(buffer);
    
    if (result == -1){
    	printf("Couldn't wait on lock %d , CV with index %d \n", lk, cv);
    }
    // else{
//     	printf("Waiting on lock %d , CV with index %d \n", lk, cv);
//     }
    fflush(stdout);	
    
	// conditionsTableLock -> Acquire();
// 	if(cv < 0 || cv >= maxConditionTableSize){ //lack second half
// 		cout<<"CV index out of scope"<<endl;
// 		conditionsTableLock -> Release();
// 		return;
// 	}
// 	if(conditions[cv].condition == NULL){
// 		cout<<"No such condition variable exist"<<endl;
// 		conditionsTableLock -> Release();
// 		return;
// 	}
// 	if(conditions[cv].addrSpace != currentThread -> space){
// 		cout<<"Condition you requested doesn't belong to the current thread"<<endl;
// 		conditionsTableLock->Release();
// 		return;
// 	}
// 	conditions[cv].counter++;
// 	conditionsTableLock -> Release();
// 	
// 	locksTableLock -> Acquire();
// 	if(lk < 0 || lk >= maxConditionTableSize){ //lack second half
// 		cout<<"Index of lock out of scope"<<endl;
// 		locksTableLock -> Release();
// 		return;
// 	}
// 	if(locks[lk].lock == NULL){
// 		cout<<"No such lock exist"<<endl;
// 		locksTableLock -> Release();
// 		return;
// 	}
// 	if(locks[lk].addrSpace != currentThread -> space){
// 		cout<<"Lock you requested doesn't belong to the current thread"<<endl;
// 		locksTableLock -> Release();
// 		return;
// 	}
// 	
// 	
// 	locks[lk].counter++;
// 	locksTableLock -> Release();
// 	cout<<"Condition variable "<<conditions[cv].condition->getName()<<" waiting on "<< locks[lk].lock->getName()<<endl;
// 	conditions[cv].condition -> Wait(locks[lk].lock);
}

void Signal_Syscall(int lk, int cv){
	stringstream ss;
    
    //cout<<"In Signal CV"<<endl;
     
    ss<<"SC "<<lk<<" "<<cv;
    
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int result = atoi(buffer);
    
    if (result == -1){
    	printf("Couldn't signal on lock %d , CV with index %d \n", lk, cv);
    }
    else{
    	printf("Signalling on lock %d , CV with index %d \n", lk, cv);
    }
    fflush(stdout);	
    


//	conditionsTableLock -> Acquire();
// 	if(cv < 0 || cv>=maxConditionTableSize){ //lack second half
// 	cout<<"Index of Condition Variable is out of scope"<<endl;
// 		conditionsTableLock -> Release();
// 		return;
// 	}
// 		if(conditions[cv].condition == NULL){
// 		cout<<"No such condition variable exist"<<endl;
// 		conditionsTableLock -> Release();
// 		return;
// 	}
// 	
// 	if(conditions[cv].addrSpace != currentThread -> space){
// 		cout<<"Condition variable you requested doesn't belong to the current thread"<<endl;
// 		conditionsTableLock -> Release();
// 		return;
// 	}
// 
// 	conditions[cv].counter--;
// 	conditionsTableLock -> Release();
// 	
// 	locksTableLock -> Acquire();
// 	if(lk < 0 || lk >= maxLockTableSize){ //lack second half
// 		cout<<"No such lock exist"<<endl;
// 		locksTableLock -> Release();
// 		return;
// 	}
// 	if(locks[lk].lock == NULL){
// 		cout<<"No such lock exist"<<endl;
// 		locksTableLock-> Release();
// 		return;
// 	}
// 	if(locks[lk].addrSpace != currentThread -> space){
// 	    cout<<"Lock you requested doesn't belong to the current thread"<<endl;
// 		locksTableLock -> Release();
// 		return;
// 	}
// 	
// 	locks[lk].counter--;
// 	cout<<"Condition variable "<<conditions[cv].condition->getName()<<" signaling on  "<< locks[lk].lock->getName()<<endl;
// 	locksTableLock -> Release();
// 	
// 	conditions[cv].condition -> Signal(locks[lk].lock);
// 	if(conditions[cv].isToBeDeleted == true){
// 		DestroyCondition_Syscall(cv);
// 	}
}

void Broadcast_Syscall(int lk, int cv){
	stringstream ss;
    
    //cout<<"In Broadcast CV"<<endl;
     
    ss<<"BC "<<lk<<" "<<cv;
    
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int result = atoi(buffer);
    
    if (result == -1){
    	printf("Couldn't broadcast on lock %d , CV with index %d \n", lk, cv);
    }
    else{
    	printf("Broadcasting on lock %d , CV with index %d \n", lk, cv);
    }
    fflush(stdout);	

// 	int temp = 0;
// 	conditionsTableLock -> Acquire();
// 	if(cv < 0 || cv >= maxConditionTableSize){ //lack second half
// 		cout<<"Index of CV is out of scope"<<endl;
// 		conditionsTableLock -> Release();
// 		return;
// 	}
// 	if(conditions[cv].condition == NULL){
// 		conditionsTableLock -> Release();
// 		cout<<"No such condition variable exist"<<endl;
// 		return;
// 	}
// 	if(conditions[cv].addrSpace != currentThread -> space){
// 		cout<<"Condition variable you requested doesn't belong to the current thread"<<endl;
// 		conditionsTableLock -> Release();
// 		return;
// 	}
// 	
// 	temp = conditions[cv].counter;
// 	conditions[cv].counter--;
// 	conditionsTableLock -> Release();
// 	
// 	locksTableLock -> Acquire();
// 	if(locks[lk].addrSpace != currentThread -> space){
// 		cout<<"Index of lock is out of scope"<<endl;
// 		locksTableLock -> Release();
// 		return;
// 	}
// 	if(locks[lk].lock == NULL){
// 		locksTableLock -> Release();
// 		cout<<"No such lock exist"<<endl;
// 		return;
// 	}
// 	if(lk < 0 || lk>=maxLockTableSize){ //lack second half
// 		cout<<"Lock you requested doesn't belong to the current thread"<<endl;
// 		locksTableLock -> Release();
// 		return;
// 	}
// 	
// 	if(conditions[cv].counter - temp < 0){
// 		locks[lk].counter = 0;
// 	}
// 	else{
// 		conditions[cv].counter = locks[lk].counter - temp;
// 	}
// 	cout<<"Condition variable "<<conditions[cv].condition->getName()<<" broadcasting on "<< locks[lk].lock->getName()<<endl;
// 	locksTableLock -> Release();
// 	conditions[cv].condition -> Broadcast(locks[lk].lock);

}

void PrintNum_Syscall(int num){
	cout<<num<<endl;
}


int CreateMV_Syscall(unsigned int name, int len){
 	char *buf = new char[len+1];
 	
 	if (len < 1 || len >30){
 		cout<<"Error: name is too long"<<endl;
 		return -1;
 	}
 		
 	copyin(name,len,buf); 
    stringstream ss;
         
    ss<<"CCM "<<buf;
    char *msg = new char[40];
 	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

	int server_num = rand()%2;

	//cout<<"Sending to "<<server_num<<endl;
       outPktHdr.to = 0;	//destination machine id 	
	   outMailHdr.to = 0;
       outMailHdr.from = currentThread->mailboxNum;
       outMailHdr.length = strlen(msg) + 1;
 
//     // Send the first message
     bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 
 
    if ( !success ) {
       printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
       interrupt->Halt();
     }
 
   // Wait for the first message from the other machine
   	   postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
   	   int index = atoi(buffer);
   	   //if successfully created 
   	   if (index != -1){
      		printf("Created/retrieved MV at index %d with name %s \n",index, buf);
       }
       else{
			printf("Couldn't create MV with name %s, wrong name or array is full \n", buf);
       }
      fflush(stdout);	
      
      return index;
      
}

int DestroyMV_Syscall(int id){
    stringstream ss;
         
    ss<<"DM "<<id;
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    int index = atoi(buffer);
	if (index != -1){
    	printf("MV at index %d successfully deleted\n",id);
    }
    else{
    	printf("Wasn't able to destroy MV at index %d \n", id);
    }
    
    fflush(stdout);	
    return index;
}


int SetMV_Syscall(int id, int value){
 	
    stringstream ss;
    
    //cout<<"In set MV"<<endl;
     
    ss<<"CSM "<<id<<" "<<value;
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
	
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    
    int index = atoi(buffer);
    
    if (index != -1){
    	printf("Set MV at index %d with value %d was updated\n",id, value);
    }
    else{
    	printf("Wasn't able to set MV at index %d \n", id);
    }
    
    
    fflush(stdout);
    return index;	
}

int GetMV_Syscall(int index){
 	
    stringstream ss;
    
    //cout<<"In get MV"<<endl;
     
    ss<<"CGM "<<index;
 	char *msg = new char[40];
	strcpy(msg,(char *) ss.str().c_str());

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0; //to server		
	outMailHdr.to = 0;
    outMailHdr.from = currentThread->mailboxNum;
    outMailHdr.length = strlen(msg) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, msg); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(currentThread->mailboxNum, &inPktHdr, &inMailHdr, buffer);
    int value = atoi(buffer);
    if (value != -1){
    	printf("Got value %d of MV at index %d \n",value, index);
    }
    else{
    	printf("Couldn't get value at index %d \n", index);
    }
    fflush(stdout);	
    
    return value;
}

//exec function
void exec_thread() {

  currentThread->space->InitRegisters(); //initialize register
  currentThread->space->RestoreState(); //restore
  machine->Run(); //run the machine
  
}

int Exec_Syscall(unsigned int addr, int len){

	//from (copied) OpenFile- open file
	char *buf = new char[len+1]; 
  	OpenFile *f;                  
  	int fileID;                       
  
    //check error
	if (!buf) {
    	printf("%s","Can't allocate kernel buffer in Exec\n");
    	return -1;
  	}
    if( copyin(addr,len,buf) == -1 ) {
    	printf("%s","Bad pointer passed to Exec\n");
    	delete[] buf;
    	return -1;
  	}

  	buf[len]='\0';

  	f = fileSystem->Open(buf);
	
	//check error, return -1
  	if(!f) {
    	printf("Error opening file at %s\n", buf);
    	return -1;
  	}
  	
  	AddrSpace* addrSpace_process = new AddrSpace(f); //create a new address space for exec
  	Thread* next_thread = new Thread(buf); //create a new thread
  	
    int given_id = processTable->Put(addrSpace_process);
  	
  	next_thread->space = addrSpace_process; //set space
  	next_thread->setProcessID(given_id); //set the id
  	
  	int num = addrSpace_process->table_thread->Put(next_thread); //place next thread
  	next_thread->setThreadNum(num); //set thread number
  	
  	//fork new thread
  	next_thread->Fork((VoidFunctionPtr)exec_thread, addr);
  	
	//set mailbox number (using counter)
  	//next_thread->mailboxNum = mailboxCounter;
  	//mailboxCounter++; //increment the number/counter
  
	delete f;
  	return given_id;
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
		
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
		
		case SC_Exec:
 		DEBUG('a', "Exec.\n");
 		rv=Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
 		break;
		
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
		
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
		
		
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
		
		
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
		
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Yield:
		DEBUG('a', "Yield syscall.\n");
		Yield_Syscall();
		break;
		
		case SC_Exit:
		DEBUG('a', "Exit syscall.\n");
		Exit_Syscall();
		break;
	
		case SC_CreateLock:					
 		DEBUG('a', "Create Lock.\n");
 		rv=CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
 		break;
		
		case SC_DestroyLock:					
		DEBUG('a', "Destroy Lock.\n");
		DestroyLock_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Acquire:					
		DEBUG('a', "Acquire Lock.\n");
		Acquire_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Release:					
		DEBUG('a', "Release Lock.\n");
		Release_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_CreateCondition:					
		DEBUG('a', "Create condition.\n");
		rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
		
		case SC_DestroyCondition:					
		DEBUG('a', "Destroy Condition.\n");
		DestroyCondition_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Wait:
		DEBUG('a', "Wait(CV) syscall.\n");
		Wait_Syscall(machine->ReadRegister(4),
					machine->ReadRegister(5));
		break;
		case SC_Signal:
		DEBUG('a', "Signal(CV) syscall.\n");
		Signal_Syscall(machine->ReadRegister(4),
					machine->ReadRegister(5));
		break;
		case SC_Broadcast:
		DEBUG('a', "Broadcast(CV) syscall.\n");
		Broadcast_Syscall(machine->ReadRegister(4),
					machine->ReadRegister(5));
		break;
		case SC_PrintNum:
		DEBUG('a', "PrintNum() ssassaystem call.\n");
		PrintNum_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_CreateMV:					
 		DEBUG('a', "Create MV.\n");
 		rv=CreateMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
 		break;
 		
 		case SC_DestroyMV:					
 		DEBUG('a', "Destroy MV.\n");
 		rv=DestroyMV_Syscall(machine->ReadRegister(4));
 		break;
 		
 		case SC_SetMV:					
 		DEBUG('a', "Set MV.\n");
 		rv=SetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
 		break;
 		
 		case SC_GetMV:					
 		DEBUG('a', "Get MV.\n");
 		rv=GetMV_Syscall(machine->ReadRegister(4));
 		break;
 		
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}