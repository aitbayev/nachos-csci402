#include "post.h"
#include <string.h>
#include <stdio.h>
#include <sstream>

PacketHeader outPktHdr, inPktHdr;
MailHeader outMailHdr, inMailHdr;

void sendReply(int result){
	stringstream ss;
    ss<<result;
    char *reply = (char *)ss.str().c_str();
	bool success = postOffice->Send(outPktHdr, outMailHdr, reply);
    if (!success){
 		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
 	}	
}

void server(){

	while(true){
	
		char buffer[10];
			
   		outMailHdr.to = 0;
    	outMailHdr.from = 1;
    	outMailHdr.length = strlen(buffer) + 1;
	
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		outPktHdr.to = inPktHdr.from;	

		char *request = strtok(buffer, " ");
		char *requestType;
		char *name;
		char *value;
		int counter = 0;
		
		
		while(request != NULL){
			if(counter == 0){
				requestType = request;
			}
			else if (counter == 1){
				name = request;
			}
			else if (counter == 2){
				value = request;
			}
			request = strtok(NULL, " ");
			counter++;
		}
		
		bool error = false;
		bool found = false;
		
//CREATE MV	
		if (strcmp (requestType,"CM") == 0){
			cout<<"Create Monitor variable."<<endl;
			error = false;
			found = false;	
			for (int i=0; i<mv_index; i++){
				if (strcmp (name, MVs[i].name) == 0){
					cout<<"Monitor variable with name "<<name<<" already exists. Cannot create MV."<<endl;
					found = true;
				}
			}
			if (mv_index >= 100){
				cout<<"Cannot create MV, array is full."<<endl;
				error = true;
			}
			if (error == false && found == false){
				cout<<"Creating new MV."<<endl;
				MVs[mv_index].name = new char[30];
  				sprintf(MVs[mv_index].name, "%s", name);
 				MVs[mv_index].ID = mv_index;
 				MVs[mv_index].value = 0;	
 				
				sendReply(mv_index);

 				mv_index++;		
			}
			else{
				sendReply(-1);
			}	
		}
//DESTROY MV		
		else if (strcmp (requestType,"DM") == 0){
			cout<<"Delete Monitor variable."<<endl;
			int index = atoi(name);
			error = false;
			found = false;
			
			for (int i=0; i<mv_index; i++){
				if (index == MVs[i].ID){
					found = true;
				}
			}
			if (index >= mv_index || index < 0){
				cout<<"Cannot delete MV, wrong argument."<<endl;
				error = true;
			}
			
			if (found == true && error == false){
				MVs[index].name = "";
 				MVs[index].ID = -1;
 				MVs[index].value = -1;	
 				
 				sendReply(index);
			}
			else{
				cout<<"MV doesn't exist or wrong argument was passed."<<endl;
				sendReply(-1);	
			}
		}
//SET MV			
		else if (strcmp (requestType,"SM") == 0){
			cout<<"Set Monitor variable."<<endl;
			int val = atoi(value);
			int index = atoi(name);
			error = false;
			found = false;
			for (int i=0; i<mv_index; i++){
				if (index == MVs[i].ID){
					found = true;
				}
			}
			if (index >= mv_index || index < 0){
				cout<<"Cannot set MV, wrong argument."<<endl;
				error = true;
			}
			
			if (found == true && error == false){
 				MVs[index].value = val; 
 				sendReply(index);
			}
			else{
				cout<<"MV doesn't exist or wrong argument was passed."<<endl;
				sendReply(-1);
			}
		}
//GET MV		
			else if (strcmp (requestType,"GM") == 0){
			cout<<"Get Monitor variable."<<endl;
			
			int index = atoi(name);
			error = false;
			found = false;
			for (int i=0; i<mv_index; i++){
				if (index == MVs[i].ID){
					found = true;
				}
			}
			if (index >= mv_index || index < 0){
				cout<<"Cannot get MV, wrong argument."<<endl;
				error = true;
			}
			
			if (found == true && error == false){
     			sendReply(MVs[index].value);	
			}
			else{
				cout<<"MV doesn't exist or wrong argument was passed."<<endl;
				sendReply(-1);
			}
		}
//CREATE LOCK
		else if (strcmp (requestType,"CL") == 0){
			cout<<"Create Lock."<<endl;
			found = false;
			error = false;
			for (int i=0; i<server_lock_counter; i++){
				if (strcmp (name, server_locks[i].name) == 0){
					cout<<"Lock with this name already exists. Cannot create lock."<<endl;
					found = true;
				}
			}
			
			if (server_lock_counter>=100){
				cout<<"You cannot create locks anymore, table is full."<<endl;
				error = true;
			}
			if (found == false && error == false){
				cout<<"Creating lock with name "<<name<<" at index "<<server_lock_counter<<endl;
				server_locks[server_lock_counter].name = new char[30];
  				sprintf(server_locks[server_lock_counter].name, "%s", name);				
				server_locks[server_lock_counter].state = 1; //available
				server_locks[server_lock_counter].isToBeDeleted = false;
				server_locks[server_lock_counter].machineID = -1;
				server_locks[server_lock_counter].mailbox = -1;
				server_locks[server_lock_counter].usage_counter = 0;
				server_locks[server_lock_counter].lockWaitQueue = new List();	
				server_locks[server_lock_counter].id = server_lock_counter;
				server_locks[server_lock_counter].deleted = false;
     			sendReply(server_lock_counter);	
     			server_lock_counter++;
			}
			else{
				sendReply(-1);
			}
		}
//DESTROY LOCK
		else if (strcmp (requestType,"DL") == 0){
			cout<<"Destroy Lock."<<endl;
			found = false;
			error = false;
			int index = atoi(name);
			//validate argument index
			if (index>=server_lock_counter || index<0){
				cout<<"Invalid argument is passed for destroy lock."<<endl;
				error = true;
			}
			//find lock in array
			for (int i=0; i<server_lock_counter; i++){
				if (index == server_locks[i].id){
					found = true;
				}
			}
			//if found 
			if (found == true && error == false){
				//check if lock is still being used 
				if (server_locks[index].usage_counter>0){
					server_locks[index].isToBeDeleted = true;
					cout<<"Lock is still used, toBeDeleted set to true"<<endl;
					sendReply(-1);
				}
				//if lock is not being used delete it
				else{
					server_locks[index].name = "";
					server_locks[index].deleted = true;
					server_locks[index].id = -1;
					sendReply(1);
				}	
			}
			//if argument is wrong or lock wasn't send msg that destroy is failed 
			else{
					cout<<"Unable to destroy lock"<<endl;
					sendReply(-1);	
				}
			}
//ACQUIRE LOCK
			else if (strcmp (requestType,"AL") == 0){
			cout<<"Acquire Lock."<<endl;
				found = false;
				error = false;
				int index = atoi(name);
				
				for (int i=0; i<server_lock_counter; i++){
					if (index == server_locks[i].id){
						found = true;
					}
				}
				
				if (index < 0 || index >= server_lock_counter){
					cout<<"Invalid index."<<endl;
					error = true;
				}
				if (server_locks[index].machineID == inPktHdr.from && server_locks[index].mailbox == inMailHdr.from){
					cout<<"Thread already owns the lock"<<endl;
					sendReply(-1);	
				}
				else{
					if (found == true && error == false){
						server_locks[index].usage_counter++;
					
						if (server_locks[index].state == 1){
							cout<<"Machine id "<<inPktHdr.from<<" got the lock"<<endl; 
							server_locks[index].state = 0;
							server_locks[index].machineID = inPktHdr.from;
							server_locks[index].mailbox = inMailHdr.from;
							sendReply(1);
						}
						else{
							cout<<"The lock is busy"<<endl;
							WaitingClient *waiting_client = new WaitingClient;
							waiting_client->machineID = inPktHdr.from;
							waiting_client->mailbox = inMailHdr.from;
							server_locks[index].lockWaitQueue->Append((void*)waiting_client);
						}
					}
					else{
						cout<<"Cannot acquire the lock, not found or wrong arg."<<index<<endl;
						sendReply(-1);
					}
				}
			}
//RELEASE LOCK
			else if (strcmp (requestType,"RL") == 0){
			cout<<"Release Lock."<<endl;
				found = false;
				error = false;
				int index = atoi(name);
				for (int i=0; i<server_lock_counter; i++){
					if (index == server_locks[i].id){
						found = true;
					}
				}

				if (index < 0 || index >= server_lock_counter){
					cout<<"Invalid index."<<endl;
					error = true;
				}
				
				if (found == true && error == false){
					//if thread is not lock owner send -1
					if (server_locks[index].machineID != inPktHdr.from || server_locks[index].mailbox != inMailHdr.from){
						cout<<"The thread is not a lock owner "<<index<<endl;
						sendReply(-1);
					}
					else{
						//if thread is lock owner
						server_locks[index].usage_counter--;
						//delete if you should
						if (server_locks[index].isToBeDeleted == true && server_locks[index].usage_counter == 0){
							cout<<"Lock "<<index<<" is deleted"<<endl;
							server_locks[index].id = -1;
							server_locks[index].deleted = true;
							sendReply(2);
						}
						else{
							//if lock waitQueue is empty make lock available
							if (server_locks[index].lockWaitQueue->IsEmpty()){
								cout<<"Wait queue of lock "<<index<<" is empty, lock is available now"<<endl;
								server_locks[index].state = 1;
								server_locks[index].machineID = -1;
								server_locks[index].mailbox = -1;
								sendReply(1);
							}
							else{
							//else take one thread at make it owner
								    WaitingClient *client;
								    client = (WaitingClient*)server_locks[index].lockWaitQueue->Remove();
									server_locks[index].machineID = client->machineID;
									server_locks[index].mailbox = client->mailbox;
									server_locks[index].state = 0; //not available
									cout<<"Client "<< client->machineID<< " acquired the lock."<<endl;
									sendReply(1);	
							}
						}
					}
				}
				else{
					cout<<"Cannot release the lock, not found or wrong arg."<<index<<endl;
					sendReply(-1);
				}
			}
//CREATE CV	
			else if (strcmp (requestType,"CC") == 0){
			cout<<"Create CV."<<endl;
				found = false;
				error = false;
				for (int i=0; i<server_cv_counter; i++){
					if (strcmp (name, server_cvs[i].name) == 0){
						cout<<"CV with this name already exists. Cannot create lock."<<endl;
						found = true;
					}
				}
			
				if (server_cv_counter>=100){
					cout<<"You cannot create CVs anymore, table is full."<<endl;
					error = true;
				}
				if (found == false && error == false){
					server_cvs[server_cv_counter].name = new char[30];
  					sprintf(server_cvs[server_cv_counter].name, "%s", name);
					server_cvs[server_cv_counter].isToBeDeleted = false;
					server_cvs[server_cv_counter].machineID = -1;
					server_cvs[server_cv_counter].mailbox = -1;
					server_cvs[server_cv_counter].usage_counter = 0;
					server_cvs[server_cv_counter].cvWaitQueue = new List();	
					server_cvs[server_cv_counter].id = server_cv_counter;
					server_cvs[server_cv_counter].deleted = false;
					server_cvs[server_cv_counter].serverConditionLock = -1;
					sendReply(server_cv_counter);
					server_cv_counter++;
				}
				else{
					sendReply(-1);
				}
			}
//DESTROY CV	
			else if (strcmp (requestType,"DC") == 0){
			cout<<"Destroy CV."<<endl;
				found = false;
				error = false;
				int index = atoi(name);
				for (int i=0; i<server_cv_counter; i++){
					if (index == server_cvs[i].id){
						found = true;
					}
				}
			
				if (index >= server_cv_counter || index < 0 ){
					cout<<"Wrong argument passed"<<endl;
					error = true;
				}
				if (found == true && error == false){
					if (server_cvs[index].usage_counter>0){
						cout<<"CV with index "<<index<<" is being used. Can't destroy it"<<endl;
						server_cvs[index].isToBeDeleted = true;
					}
					else{
						server_cvs[index].deleted = true;
						server_cvs[index].id = -1;
						server_cvs[index].name = "";
						cout<<"CV with index "<<index<<" is destroyed"<<endl;
						sendReply(1);
					}	
				}
				else{
					sendReply(-1);
				}
			}
//WAIT CV			
			else if (strcmp (requestType,"WC") == 0){
			cout<<"Wait CV."<<endl;
				WaitingClient *waiting_client = new WaitingClient;
				int lock_index = atoi(name);
				int cv_index = atoi(value);
				bool cv_found = false;
				bool lock_found = false;
				error = false;
				//check if lock index and cv index are valid
				if (lock_index < 0 || lock_index >= server_lock_counter || cv_index < 0 || cv_index >= server_cv_counter){
					cout<<"Lock index or CV index is invalid"<<endl;
					error = true;
				}
				//check if lock exists
				for (int i=0; i<server_lock_counter; i++){
					if (lock_index == server_locks[i].id){
						lock_found = true;
					}
				}
				//check if cv exists
				for (int i=0; i<server_cv_counter; i++){
					if (cv_index == server_cvs[i].id){
						cv_found = true;
					}
				}
				if (cv_found == true && lock_found == true && error == false){
					if (server_locks[lock_index].isToBeDeleted == true || server_cvs[cv_index].isToBeDeleted==true){
						cout<<"CV or Lock is to be deleted. Cannot wait"<<endl;
						sendReply(-1);
					}
					else{
						if (server_cvs[cv_index].serverConditionLock != lock_index && server_cvs[cv_index].serverConditionLock != -1){
							cout<<"Wrong waiting lock index"<<endl;
							sendReply(-1);
						}
						else{
							server_cvs[cv_index].usage_counter++;
							//if first client to wait set waiting lock
								if (server_cvs[cv_index].serverConditionLock == -1){
									cout<<"This is the first client to wait on cv"<<endl;
									server_cvs[cv_index].serverConditionLock = lock_index;
								}
								waiting_client->machineID = inPktHdr.from;
								waiting_client->mailbox = inMailHdr.from;
								server_cvs[cv_index].cvWaitQueue->Append((void*)waiting_client);
								sendReply(1);
								//locks wait queue is not empty
								if (!(server_locks[lock_index].lockWaitQueue->IsEmpty())){
									cout<<"Lock waitlist is not empty"<<endl;
									waiting_client = (WaitingClient*)server_locks[lock_index].lockWaitQueue->Remove();
									server_locks[lock_index].machineID = waiting_client->machineID;
									server_locks[lock_index].mailbox = waiting_client->mailbox;
									server_locks[lock_index].usage_counter--;			
								}
						}
					}	
				}		
				else{
					cout<<"ERROR: cv, lock not found or wrong index"<<endl;
					sendReply(-1);
				}
					
			}
			
//Signal CV			
			else if (strcmp (requestType,"SC") == 0){
				cout<<"Signal CV."<<endl;
				WaitingClient *waiting_client = new WaitingClient;
				int lock_index = atoi(name);
				int cv_index = atoi(value);
				bool cv_found = false;
				bool lock_found = false;
				error = false;
				//check if lock index and cv index are valid
				if (lock_index < 0 || lock_index >= server_lock_counter || cv_index < 0 || cv_index >= server_cv_counter){
					cout<<"Lock index or CV index is invalid"<<endl;
					error = true;
				}
				//check if lock exists
				for (int i=0; i<server_lock_counter; i++){
					if (lock_index == server_locks[i].id){
						lock_found = true;
					}
				}
				//check if cv exists
				for (int i=0; i<server_cv_counter; i++){
					if (cv_index == server_cvs[i].id){
						cv_found = true;
					}
				}
				if (cv_found == true && lock_found == true && error == false){
					if (server_locks[lock_index].isToBeDeleted == true || server_cvs[cv_index].isToBeDeleted==true){
						cout<<"CV or Lock is to be deleted. Cannot wait"<<endl;
						sendReply(-1);
					}
					else{
						//nothing to signal
						if (server_cvs[cv_index].usage_counter<1){
							cout<<"No thread waiting in this CV, nothing to wake"<<endl;
							sendReply(-1);
						}
						else if (server_cvs[cv_index].serverConditionLock != lock_index){
							cout<<"Server cond "<<server_cvs[cv_index].serverConditionLock<<endl;
							cout<<"lock ind "<<lock_index<<endl;
							cout<<"Wrong lock"<<endl;
							sendReply(-1);
						}
						else {
							server_cvs[cv_index].usage_counter--;
							waiting_client = (WaitingClient*)server_cvs[cv_index].cvWaitQueue->Remove();
							server_locks[lock_index].lockWaitQueue->Append((void*)waiting_client);
							if (server_cvs[cv_index].cvWaitQueue->IsEmpty()){
								server_cvs[cv_index].serverConditionLock = -1;
							}
							if (server_cvs[cv_index].isToBeDeleted && server_cvs[cv_index].usage_counter == 0){
								cout<<"CV with index "<<cv_index<<" destroyed"<<endl;
								server_cvs[cv_index].deleted = true;
								server_cvs[cv_index].id = -1;
								server_cvs[cv_index].name = "";
							}
							sendReply(1);
						}
					}
				}
				else{
					cout<<"ERROR: cv, lock not found or wrong index"<<endl;
					sendReply(-1);
				}
			}
//Broadcast CV
			else if (strcmp (requestType,"BC") == 0){
			cout<<"Broadcast CV."<<endl;
				WaitingClient *waiting_client = new WaitingClient;
				int lock_index = atoi(name);
				int cv_index = atoi(value);
				bool cv_found = false;
				bool lock_found = false;
				error = false;
				//check if lock index and cv index are valid
				if (lock_index < 0 || lock_index >= server_lock_counter || cv_index < 0 || cv_index >= server_cv_counter){
					cout<<"Lock index or CV index is invalid"<<endl;
					error = true;
				}
				//check if lock exists
				for (int i=0; i<server_lock_counter; i++){
					if (lock_index == server_locks[i].id){
						lock_found = true;
					}
				}
				//check if cv exists
				for (int i=0; i<server_cv_counter; i++){
					if (cv_index == server_cvs[i].id){
						cv_found = true;
					}
				}
				if (cv_found == true && lock_found == true && error == false){
					if (server_locks[lock_index].isToBeDeleted == true || server_cvs[cv_index].isToBeDeleted==true){
						cout<<"CV or Lock is to be deleted. Cannot wait"<<endl;
						sendReply(-1);
					}
					else if(server_cvs[cv_index].serverConditionLock != lock_index){
						cout<<"Wrong index of waiting lock"<<endl;
						sendReply(-1);
					}
					else if (server_cvs[cv_index].cvWaitQueue->IsEmpty()){
							cout<<"CV wait queue is empty! Can't broadcast"<<endl;
							sendReply(-1);
						}
					else{
							while(!(server_cvs[cv_index].cvWaitQueue->IsEmpty())){
								cout<<"Broadcasting"<<endl;
								waiting_client = (WaitingClient*)server_cvs[cv_index].cvWaitQueue->Remove();
								server_locks[lock_index].lockWaitQueue->Append((void*)waiting_client);
							}	
							
							server_cvs[cv_index].usage_counter = 0;
							server_cvs[cv_index].serverConditionLock = -1;
							//if need to delete CV delete now
							if(server_cvs[cv_index].isToBeDeleted && server_cvs[cv_index].usage_counter == 0){
								cout<<"CV with index "<<cv_index<<" destroyed"<<endl;
								server_cvs[cv_index].deleted = true;
								server_cvs[cv_index].id = -1;
								server_cvs[cv_index].name = "";
							}
							sendReply(1);
						}
				}
				else{
					cout<<"ERROR: cv, lock not found or wrong index"<<endl;
					sendReply(-1);
				}
			
			}
//UNKNOWN 			
			else{
				cout<<"Unknown request type"<<endl;
			}
	}	

}




