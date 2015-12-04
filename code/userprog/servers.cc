#include "post.h"
#include <string.h>
#include <stdio.h>
#include <sstream>

PacketHeader outPaketHdr, inPaketHdr;
MailHeader outMailHeader, inMailHeader;

//locks, cvs and mvs belonging to this server
ServerLock s_locks[10]; 
ServerCV s_cvs[10];
MV s_mvs[10];

ServerRequest requests[200];
int request_counter = 0;
int s_lock_counter = 0;
int s_cvs_counter = 0;
int s_mvs_counter = 0;

char *currentMessage = new char[20];

//function that sends replies to the clients
void send(int result){
	stringstream ss;
    ss<<result;
    char *reply = (char *)ss.str().c_str();
	bool success = postOffice->Send(outPaketHdr, outMailHeader, reply);
    if (!success){
 		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
 	}	
}

void sendRequest(char *type, int machineID, int serverNum){
	cout<<"going to send request"<<endl;
	stringstream ss;
    ss<<type;
    
    char *reply = (char *)ss.str().c_str();
    int sendToMachine;

	
    if (machineID+1 == serverNum){
   		sendToMachine = 0;
   } 
	else{
    	sendToMachine = machineID+1;
    }
    cout<<"Chose server "<<sendToMachine<<endl;
    outPaketHdr.to = sendToMachine;
    outMailHeader.to = 0;
    
    bool success = postOffice->Send(outPaketHdr, outMailHeader, reply);
    if (!success){
 		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
 	}
 	
 	char *buffer = new char[20];
 
}

void multiple_servers(int machineID, int serverNum){



mv_index = 0;


	while(true){
	
		char buffer[20];
			
   		outMailHeader.to = 0;
    	outPaketHdr.from = machineID;
    	outMailHeader.from = 0;
    	outMailHeader.length = strlen(buffer) + 1;
	
		postOffice->Receive(0, &inPaketHdr, &inMailHeader, buffer);
		outPaketHdr.to = inPaketHdr.from;	//to make sure that server will always reply to client that sent the message

		sprintf(currentMessage, "%s", buffer);
		
		char *request = strtok(buffer, " "); //splitting requesting 
		char *requestType; //first string
		char *name; //what comes after request type name or index
		char *value; // either value or index
		char *request_from;
		int counter = 0;
		
	
		while(request != NULL){
			if(counter == 0){
				requestType = request;    //get request type
			}
			else if (counter == 1){
				name = request;     //name or index, value
			}
			else if (counter == 2){
				value = request; //index or value
			}
			else if (counter == 3){
				request_from = request;
			}
			request = strtok(NULL, " ");
			counter++;
		}
		
		
		sprintf(requests[request_counter].message, "%s", requestType);
		requests[request_counter].machine_id = inPaketHdr.from;
		requests[request_counter].mailbox = inMailHeader.from;
		request_counter++;
		
		bool error = false;
		bool found = false;
		
//CREATE MV	
		if (strcmp (requestType,"CCM") == 0){
			cout<<"Create Monitor variable."<<endl;
			error = false;
			found = false;	
			//check if MV with the same name exist
			
			
			if (mv_index > 4){
				cout<<"Cannot create MV, array is full, let other server create lock"<<endl;
				sendRequest("M", machineID, serverNum);
				error = true;
			}
			
			for (int i=0; i<s_mvs_counter; i++){
				if (strcmp (name, s_mvs[i].name) == 0){
					cout<<"Monitor variable with name "<<name<<" already exists. Cannot create MV."<<endl;
					found = true;
				}
			}
			//check if array is full
			
			//if name is ok and array not full create new MV
			if (error == false && found == false){
				cout<<"Creating new MV with name "<<name<<endl;
				MVs[mv_index].name = new char[30];
  				sprintf(MVs[mv_index].name, "%s", name);
 				MVs[mv_index].ID = mv_index;
 				MVs[mv_index].value = 0;	
 				
 				s_mvs[s_mvs_counter].name = new char[30];
 				sprintf(s_mvs[s_mvs_counter].name, "%s", name);
 				s_mvs[s_mvs_counter].ID = s_mvs_counter;
 				MVs[s_mvs_counter].value = 0;
 				
				send(mv_index); //send reply with index to client

				mv_index++;

 				s_mvs_counter++;		//increment MV counter
			}
			else{
			//else if name not ok or array is full send -1 to client
				send(-1);
			}	
		}
		else if(strcmp (requestType,"M") == 0){
			cout<<"Server request to create MV from machine"<< inPaketHdr.from<<endl;
			cout<<"Creating new MV with name "<< name <<endl;
				MVs[mv_index].name = new char[30];
  				sprintf(MVs[mv_index].name, "%s", name);
 				MVs[mv_index].ID = mv_index;
 				MVs[mv_index].value = 0;
			
				s_mvs[s_mvs_counter].name = new char[30];
 				sprintf(s_mvs[s_mvs_counter].name, "%s", name);
 				s_mvs[s_mvs_counter].ID = s_mvs_counter;
 				MVs[s_mvs_counter].value = 0;
			
				outMailHeader.to = 0;
				outPaketHdr.to = 2;
				send(mv_index); //send reply with index to client

 				mv_index++;		//increment MV counter
 				s_mvs_counter++;
		}
//DESTROY MV		
		else if (strcmp (requestType,"DM") == 0){
			cout<<"Delete Monitor variable."<<endl;
			int index = atoi(name);
			error = false;
			found = false;
			//check if MV exist
			for (int i=0; i<mv_index; i++){
				if (index == MVs[i].ID){
					found = true;
				}
			}
			//check if index is not out of bound
			if (index >= mv_index || index < 0){
				cout<<"Cannot delete MV, wrong argument."<<endl;
				error = true;
			}
			//then destroy mv
			if (found == true && error == false){
				MVs[index].name = "";
 				MVs[index].ID = -1;
 				MVs[index].value = -1;	
 				
 				send(index);
			}
			else{
			//if mv doesn't exist send -1 to client
				cout<<"MV doesn't exist or wrong argument was passed."<<endl;
				send(-1);	
			}
		}
//SET MV			
		else if (strcmp (requestType,"CSM") == 0){
			cout<<"Set Monitor variable."<<endl;
			int val = atoi(value);
			int index = atoi(name);
			error = false;
			found = false;
			//check if MV exist
			for (int i=0; i<s_mvs_counter; i++){
				if (index == s_mvs[i].ID){
					found = true;
				}
			}
			
			if (found == false){
				cout<<"I don't have that MV, let's ask another server"<<endl;
				sendRequest("S", machineID, serverNum);
			}
			//check if mv index out of bound
			if (index >= mv_index || index < 0){
				cout<<"Cannot set MV, wrong argument."<<endl;
				error = true;
			}
			//if ok set value
			if (found == true && error == false){
 				MVs[index].value = val; 
 				send(index);
			}
			else{
			//if mv doesn't exist send -1 to client
				cout<<"MV doesn't exist or wrong argument was passed."<<endl;
				send(-1);
			}
		}
		else if (strcmp (requestType,"S") == 0){
			cout<<"Got request from another server to set MV"<<endl;
			int val = atoi(value);
			int index = atoi(name);
			s_mvs[index].value = val; 
			send(index);
		}
//GET MV		
			else if (strcmp (requestType,"CGM") == 0){
			cout<<"Get Monitor variable."<<endl;
			
			int index = atoi(name);
			error = false;
			found = false;
			//check if MM exist
			for (int i=0; i<s_mvs_counter; i++){
				if (index == s_mvs[i].ID){
					found = true;
				}
			}
			if (found == false){
				cout<<"I don't have that MV, let's ask another server"<<endl;
				sendRequest("G", machineID, serverNum);
				
			}
			
			
			//check if index in out of bound 
			if (index >= mv_index || index < 0){
				cout<<"Cannot get MV, wrong argument."<<endl;
				error = true;
			}
			//if ok send value to client
			if (found == true && error == false){
     			send(MVs[index].value);	
			}
			//else send error to client
			else{
				cout<<"MV doesn't exist or wrong argument was passed."<<endl;
				send(-1);
			}
		}
		
		else if (strcmp (requestType,"G") == 0){
		int index = atoi(name);
			cout<<"Got request from another server to return MV value"<<endl;
			cout<<"Returning MV value "<<endl;
			outMailHeader.to = 0;
			outPaketHdr.to = 2;
			send(s_mvs[index].value);
		}
//CREATE LOCK
		else if (strcmp (requestType,"CL") == 0){
			cout<<"Create Lock."<<endl;
			found = false;
			error = false;
			//check if name with this name already exist
			for (int i=0; i<server_lock_counter; i++){
				if (strcmp (name, server_locks[i].name) == 0){
					cout<<"Lock with this name already exists. Cannot create lock."<<endl;
					found = true;
				}
			}
			//check if array is full
			if (server_lock_counter>=5){
				cout<<"You cannot create locks anymore, table is full."<<endl;
				error = true;
			}
			//if ok create lock
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
     			send(server_lock_counter);	
     			server_lock_counter++;
			}
			//else send -1 to client
			else{
				send(-1);
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
					send(-1);
				}
				//if lock is not being used delete it
				else{
					server_locks[index].name = "";
					server_locks[index].deleted = true;
					server_locks[index].id = -1;
					send(1);
				}	
			}
			//if argument is wrong or lock wasn't sent msg that destroy is failed 
			else{
					cout<<"Unable to destroy lock"<<endl;
					send(-1);	
				}
			}
//ACQUIRE LOCK
			else if (strcmp (requestType,"AL") == 0){
			cout<<"Acquire Lock."<<endl;
				found = false;
				error = false;
				int index = atoi(name);
				//check if lock exist
				for (int i=0; i<server_lock_counter; i++){
					if (index == server_locks[i].id){
						found = true;
					}
				}
				//validate index
				if (index < 0 || index >= server_lock_counter){
					cout<<"Invalid index."<<endl;
					error = true;
				}
				//check if the client already owns a lock 
				if (server_locks[index].machineID == inPaketHdr.from && server_locks[index].mailbox == inMailHeader.from){
					cout<<"Machine "<<server_locks[index].machineID <<" already owns the lock "<<server_locks[index].name<<endl;
					send(-1);	
				}
				else{
				//if ok acquire lock
					if (found == true && error == false){
						server_locks[index].usage_counter++;
						//if lock is available then acquire
						if (server_locks[index].state == 1){
							cout<<"Machine id "<<inPaketHdr.from<<" got the lock "<< server_locks[index].name<<endl; 
							server_locks[index].state = 0;
							server_locks[index].machineID = inPaketHdr.from;
							server_locks[index].mailbox = inMailHeader.from;
							send(1);
						}
						//if lock is busy add client to waitqueue
						else{
							cout<<"The lock is busy"<<endl;
							server_locks[index].state = 0;
							WaitingClient *waiting_client = new WaitingClient;
							waiting_client->machineID = inPaketHdr.from;
							waiting_client->mailbox = inMailHeader.from;
							server_locks[index].lockWaitQueue->Append((void*)waiting_client);
							send(-1);
						}
					}
					else{
					//error, send -1 
						cout<<"Cannot acquire the lock, not found or wrong arg."<<index<<endl;
						send(-1);
					}
				}
			}
//RELEASE LOCK
			else if (strcmp (requestType,"RL") == 0){
			cout<<"Release Lock."<<endl;
				found = false;
				error = false;
				int index = atoi(name);
				//check if lock exists
				for (int i=0; i<server_lock_counter; i++){
					if (index == server_locks[i].id){
						found = true;
					}
				}
				//check if argument is valid 
				if (index < 0 || index >= server_lock_counter){
					cout<<"Invalid index."<<endl;
					error = true;
				}
				//if argument ok and lock exists
				if (found == true && error == false){
					//if thread is not lock owner send -1
					if (server_locks[index].machineID != inPaketHdr.from || server_locks[index].mailbox != inMailHeader.from){
						cout<<"The thread is not a lock owner "<<index<<endl;
						send(-1);
					}
					else{
						//if thread is lock owner
						server_locks[index].usage_counter--;
						//delete if you should
						if (server_locks[index].isToBeDeleted == true && server_locks[index].usage_counter == 0){
							cout<<"Lock "<<index<<" is deleted"<<endl;
							server_locks[index].id = -1;
							server_locks[index].deleted = true;
							send(2);
						}
						else{
							//if lock waitQueue is empty make lock available
							if (server_locks[index].lockWaitQueue->IsEmpty()){
								cout<<"Wait queue of lock "<<index<<" is empty, lock is available now"<<endl;
								server_locks[index].state = 1;
								server_locks[index].machineID = -1;
								server_locks[index].mailbox = -1;
								send(1);
							}
							else{
							//else take one thread and make it owner
								    WaitingClient *client;
								    client = (WaitingClient*)server_locks[index].lockWaitQueue->Remove();
									server_locks[index].machineID = client->machineID;
									server_locks[index].mailbox = client->mailbox;
									server_locks[index].state = 0; //not available
									cout<<"Released. Client "<< client->machineID<< " acquired the lock."<<endl;
									send(1);	
							}
						}
					}
				}
				else{
					cout<<"Cannot release the lock, not found or wrong arg."<<index<<endl;
					send(-1);
				}
			}
//CREATE CV	
			else if (strcmp (requestType,"CC") == 0){
			cout<<"Create CV."<<endl;
				found = false;
				error = false;
				//check if cv with the same name exists
				for (int i=0; i<server_cv_counter; i++){
					if (strcmp (name, server_cvs[i].name) == 0){
						cout<<"CV with this name already exists. Cannot create lock."<<endl;
						found = true;
					}
				}
				//check if array is full
				if (server_cv_counter>=200){
					cout<<"You cannot create CVs anymore, table is full."<<endl;
					error = true;
				}
				//if name is ok and array is not full create cv
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
					send(server_cv_counter);
					server_cv_counter++;
				}
				else{
				//if not send fail 
					send(-1);
				}
			}
//DESTROY CV	
			else if (strcmp (requestType,"DC") == 0){
			cout<<"Destroy CV."<<endl;
				found = false;
				error = false;
				//check if cv exist
				int index = atoi(name);
				for (int i=0; i<server_cv_counter; i++){
					if (index == server_cvs[i].id){
						found = true;
					}
				}
				//check if arg is not out of scope
				if (index >= server_cv_counter || index < 0 ){
					cout<<"Wrong argument passed"<<endl;
					error = true;
				}
				//if cv exists and arg is fine delete it
				if (found == true && error == false){
				//if cv is being used don't delete it now, but set isToBeDeleted
					if (server_cvs[index].usage_counter>0){
						cout<<"CV with index "<<index<<" is being used. Can't destroy it"<<endl;
						server_cvs[index].isToBeDeleted = true;
					}
					//if not used delete it
					else{
						server_cvs[index].deleted = true;
						server_cvs[index].id = -1;
						server_cvs[index].name = "";
						cout<<"CV with index "<<index<<" is destroyed"<<endl;
						send(1);
					}	
				}
				else{
					send(-1);
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
			//else lock and cv exists
				if (cv_found == true && lock_found == true && error == false){
				//if cv or lock are to be deleted then dont wait
					if (server_locks[lock_index].isToBeDeleted == true || server_cvs[cv_index].isToBeDeleted==true){
						cout<<"CV or Lock is to be deleted. Cannot wait"<<endl;
						send(-1);
					}
					else{
					//if wrong waiting lock don;t wait
						if (server_cvs[cv_index].serverConditionLock != lock_index && server_cvs[cv_index].serverConditionLock != -1){
							cout<<"Wrong waiting lock index"<<endl;
							send(-1);
						}
						else{
							server_cvs[cv_index].usage_counter++;
							//if first client to wait set waiting lock
								if (server_cvs[cv_index].serverConditionLock == -1){
									cout<<"This is the first client to wait on cv"<<endl;
									server_cvs[cv_index].serverConditionLock = lock_index;
								}
								waiting_client->machineID = inPaketHdr.from;
								waiting_client->mailbox = inMailHeader.from;
								server_cvs[cv_index].cvWaitQueue->Append((void*)waiting_client);
								send(1);
								//locks wait queue is not empty
								if (!(server_locks[lock_index].lockWaitQueue->IsEmpty())){
									cout<<"Lock waitlist is not empty"<<endl;
									waiting_client = (WaitingClient*)server_locks[lock_index].lockWaitQueue->Remove();
									server_locks[lock_index].machineID = waiting_client->machineID;
									server_locks[lock_index].mailbox = waiting_client->mailbox;
									cout<<"Machine "<<server_locks[lock_index].machineID<<" got the lock "<< server_locks[lock_index].name<<endl;
									server_locks[lock_index].usage_counter--;			
								}
								else{
									server_locks[lock_index].machineID = -1;
									server_locks[lock_index].mailbox = -1;
									server_locks[lock_index].usage_counter=0;
									server_locks[lock_index].state = 1;
								}
						}
					}	
				}		
				else{
					cout<<"ERROR: cv, lock not found or wrong index"<<endl;
					send(-1);
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
				//check if lock or cv is to be deleted
					if (server_locks[lock_index].isToBeDeleted == true || server_cvs[cv_index].isToBeDeleted==true){
						cout<<"CV or Lock is to be deleted. Cannot wait"<<endl;
						send(-1);
					}
					else{
						//nothing to signal on
						if (server_cvs[cv_index].usage_counter<1){
							cout<<"No thread waiting in this CV, nothing to wake"<<endl;
							send(-1);
						}
						else if (server_cvs[cv_index].serverConditionLock != lock_index){
						//if lock index passed doesn't match cv lock index
							cout<<"Server cond "<<server_cvs[cv_index].serverConditionLock<<endl;
							cout<<"lock ind "<<lock_index<<endl;
							cout<<"Wrong lock"<<endl;
							send(-1);
						}
						else {
						//add client to wait list of lock and remove cv from cv queue
							server_cvs[cv_index].usage_counter--;
							waiting_client = (WaitingClient*)server_cvs[cv_index].cvWaitQueue->Remove();
							server_locks[lock_index].lockWaitQueue->Append((void*)waiting_client);
							//if cv wait is empty set lock index to -1
							if (server_cvs[cv_index].cvWaitQueue->IsEmpty()){
								server_cvs[cv_index].serverConditionLock = -1;
							}
							//if cv was to be destroyed delete it now
							if (server_cvs[cv_index].isToBeDeleted && server_cvs[cv_index].usage_counter == 0){
								cout<<"CV with index "<<cv_index<<" destroyed"<<endl;
								server_cvs[cv_index].deleted = true;
								server_cvs[cv_index].id = -1;
								server_cvs[cv_index].name = "";
							}
							send(1);
						}
					}
				}
				else{
					cout<<"ERROR: cv, lock not found or wrong index"<<endl;
					send(-1);
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
				//if lock and cv are fine
				if (cv_found == true && lock_found == true && error == false){
				//check if lock or cv are to be deleted 
					if (server_locks[lock_index].isToBeDeleted == true || server_cvs[cv_index].isToBeDeleted==true){
						cout<<"CV or Lock is to be deleted. Cannot wait"<<endl;
						send(-1);
					}
					//wrong lock index passed
					else if(server_cvs[cv_index].serverConditionLock != lock_index){
						cout<<"Wrong index of waiting lock"<<endl;
						send(-1);
					}
					//if cv wait queue empty than nothing to broadcast
					else if (server_cvs[cv_index].cvWaitQueue->IsEmpty()){
							cout<<"CV wait queue is empty! Can't broadcast"<<endl;
							send(-1);
						}
					else{
					//id not empty do the broadcast
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
							send(1);
						}
				}
				else{
					cout<<"ERROR: cv, lock not found or wrong index"<<endl;
					send(-1);
				}
			
			}
//UNKNOWN 			
			
	}	

}









// #include "post.h"
// #include <string.h>
// #include <stdio.h>
// #include <sstream>
// 
// //locks, cvs and mvs belonging to this server
// ServerLock s_locks[20]; 
// ServerCV s_cvs[20];
// MV s_mvs[20];
// 
// ServerRequest requests[200];
// int request_counter = 0;
// 
//  
// void send(int index){
// 	cout<<"sending "<<index<<endl;
// 	stringstream ss;
//     ss<<index;
//     char *reply = (char *)ss.str().c_str();
// 	bool success = postOffice->Send(outPaketHdr, outMailHeader, reply);
//     if (!success){
//  		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
//         interrupt->Halt();
//  	}
// 
// } 
// 
// 
// void multiple_servers(int machineID, int numServers){
// 	PacketHeader outPaketHdr, inPaketHdr;
// 	MailHeader outMailHeader, inMailHeader;
// 
// 	cout<<machineID<<endl;
// 	cout<<numServers<<endl;
// 
// 
// 	
// 	char buffer[10];
// 			
//    		outMailHeader.to = 0;
//     	outMailHeader.from = 1;
//     	outMailHeader.length = strlen(buffer) + 1;
// 	
// 		postOffice->Receive(0, &inPaketHdr, &inMailHeader, buffer);
// 		outPaketHdr.to = inPaketHdr.from;	//to make sure that server will always reply to client that sent the message
// 
// 		char *request = strtok(buffer, " "); //splitting requesting 
// 		char *requestType; //first string
// 		char *name; //what comes after request type name or index
// 		char *value; // either value or index
// 		int counter = 0;
// 		
// 		
// 		while(request != NULL){
// 			if(counter == 0){
// 				requestType = request;    //get request type
// 			}
// 			else if (counter == 1){
// 				name = request;     //name or index, value
// 			}
// 			else if (counter == 2){
// 				value = request; //index or value
// 			}
// 			request = strtok(NULL, " ");
// 			counter++;
// 		}
// 		
// 		bool error = false;
// 		bool found = false;
// 		
// while(1){		
// 		
// //CREATE MV	
// 		if (strcmp (requestType,"CM") == 0){
// 			cout<<"Create Monitor variable."<<endl;
// 			error = false;
// 			found = false;	
// 			//check if MV with the same name exist
// 			for (int i=0; i<mv_index; i++){
// 				if (strcmp (name, MVs[i].name) == 0){
// 					cout<<"Monitor variable with name "<<name<<" already exists. Cannot create MV."<<endl;
// 					found = true;
// 				}
// 			}
// 			//check if array is full
// 			if (mv_index >= 200){
// 				cout<<"Cannot create MV, array is full."<<endl;
// 				error = true;
// 			}
// 			//if name is ok and array not full create new MV
// 			if (error == false && found == false){
// 				cout<<"Creating new MV."<<endl;
// 				MVs[mv_index].name = new char[30];
//   				sprintf(MVs[mv_index].name, "%s", name);
//  				MVs[mv_index].ID = mv_index;
//  				MVs[mv_index].value = 0;	
// 				send(mv_index); //send reply with index to client
//  				mv_index++;		//increment MV counter
// 			}
// 			else{
// 			//else if name not ok or array is full send -1 to client
// 				send(-1);
// 			}	
// 		}
// 	
// 	
// 	
// }	
// 	
// 	
// 	// while(1){
// // 		cout<<"In server loop, going to check requests"<<endl;
// // 		char *message = new char[20];
// // 		bool nothing = true;
// // 		outMailHeader.to = 0;
// //     	outPaketHdr.from = 1;
// //     	
// //     	outMailHeader.length = strlen(message) + 1;
// // 		
// // 		postOffice->Receive(0, &inPaketHdr, &inMailHeader, message);
// // 		
// // 		cout<<message<<endl;
// // 		
// // 		cout<<"inpaket "<< inPaketHdr.from <<endl;
// // 		cout<<"inmail "<<inMailHeader.from<<endl;
// // 		
// // 		outPaketHdr.to = inPaketHdr.from;
// // 
// // 		char *request = strtok(message, " "); //splitting requesting 
// // 		char *requestType; //first string
// // 		char *name; //what comes after request type name or index
// // 		char *value; // either value or index
// // 		int counter = 0;
// // 		
// // 		
// // 		while(request != NULL){
// // 			if(counter == 0){
// // 				requestType = request;    //get request type
// // 			}
// // 			else if (counter == 1){
// // 				name = request;     //name or index, value
// // 			}
// // 			else if (counter == 2){
// // 				value = request; //index or value
// // 			}
// // 			request = strtok(NULL, " ");
// // 			counter++;
// // 		}
// // 		if (message != NULL){
// // 			if (strcmp (requestType,"CM") == 0){
// // 				cout<<"Create Monitor variable."<<endl;
// // 				error = false;
// // 				found = false;	
// // 				//check if MV with the same name exist
// // 				for (int i=0; i<mv_index; i++){
// // 					if (strcmp (name, MVs[i].name) == 0){
// // 						cout<<"Monitor counter "<<mv_index<<endl;
// // 						cout<<"Monitor variable with name "<<name<<" already exists. Cannot create MV."<<endl;
// // 						found = true;
// // 					}
// // 				}
// // 				//check if array is full
// // 				if (mv_index >= 200){
// // 					cout<<"Cannot create MV, array is full."<<endl;
// // 					error = true;
// // 				}
// // 				//if name is ok and array not full create new MV
// // 				if (error == false && found == false){
// // 					cout<<"Creating new MV."<<endl;
// // 					MVs[mv_index].name = new char[30];
// // 					sprintf(MVs[mv_index].name, "%s", name);
// // 					MVs[mv_index].ID = mv_index;
// // 					MVs[mv_index].value = 0;	
// // 				
// // 					send(mv_index); //send reply with index to client
// // 
// // 					mv_index++;		//increment MV counter
// // 				}
// // 				else{
// // 				//else if name not ok or array is full send -1 to client
// // 					send(-1);
// // 				}	
// // 			}
// // 		}
// 		
// 		
// 	
// 
// }