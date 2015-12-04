// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "syscall.h"
#include "../userprog/server.cc"
#include "../userprog/servers.cc"


// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
//     PacketHeader outPktHdr, inPktHdr;
//     MailHeader outMailHdr, inMailHdr;
//     char *data = "Hello there!";
//     char *ack = "Got it!";
//     char buffer[MaxMailSize];
// 
//     // construct packet, mail header for original message
//     // To: destination machine, mailbox 0
//     // From: our machine, reply to: mailbox 1
//     outPktHdr.to = farAddr;		
//     outMailHdr.to = 0;
//     outMailHdr.from = 1;
//     outMailHdr.length = strlen(data) + 1;
// 
//     // Send the first message
//     bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
// 
//     if ( !success ) {
//       printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
//       interrupt->Halt();
//     }
// 
//     // Wait for the first message from the other machine
//     postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
//     printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
//     fflush(stdout);
// 
//     // Send acknowledgement to the other machine (using "reply to" mailbox
//     // in the message that just arrived
//     outPktHdr.to = inPktHdr.from;
//     outMailHdr.to = inMailHdr.from;
//     outMailHdr.length = strlen(ack) + 1;
//     success = postOffice->Send(outPktHdr, outMailHdr, ack); 
// 
//     if ( !success ) {
//       printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
//       interrupt->Halt();
//     }
// 
//     // Wait for the ack from the other machine to the first message we sent.
//     postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
//     printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
//     fflush(stdout);
// 
//     // Then we're done!
//     interrupt->Halt();
}
//successful test of creating, deleting, setting and getting MV, no bad args passed
void MV_test(){

	// int mv1 = CreateMV("abc", 3);
// 	cout<<"Created MV at index "<<mv1<<endl;
// 	
// 	int set = SetMV(mv1, 5);
// 	
// 	cout<<"Set value of MV at index "<< set<<endl;
// 	
// 	int get = GetMV(mv1);
// 	cout<<"Get value of MV at index 0 = " <<get<<endl;
// 	
// 	int removeMV = DestroyMV(mv1);
// 	
// 	if (removeMV){
// 		cout<<"MV was deleted at index 0"<<endl;
// 	}
// 	else{
// 		cout<<"Failed to delete MV at index 0"<<endl;
// 	}
}

void Lock_test(){
	
	//int lock1 = CreateLock("lock", 4);
	//cout<<"Lock was created at index "<<lock1<<endl;

}

void Server_test(){
	server();
}

void Servers(int id, int num){
	multiple_servers(id, num);
}
