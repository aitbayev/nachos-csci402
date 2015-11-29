#include "post.h"
#include <string.h>
#include <stdio.h>
#include <sstream>

//locks, cvs and mvs belonging to this server
ServerLock locks[20]; 
ServerCV cvs[20];
MV mvs[20];

ServerRequest requests[200];
int request_counter = 0;


PacketHeader outPktHdr, inPktHdr;
MailHeader outMailHdr, inMailHdr;

//function that sends replies to the clients
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


int main(){

	while(true){
	
		char buffer[10];
			
   		// outMailHdr.to = 0;
//     	outMailHdr.from = 1;
//     	outMailHdr.length = strlen(buffer) + 1;
	
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		outPktHdr.to = inPktHdr.from;	//to make sure that server will always reply to client that sent the message
		outMailHdr.to = inMailHdr.from;

		char *request = strtok(buffer, " "); //splitting requesting 
		char *requestType; //first string
		char *name; //what comes after request type name or index
		char *value; // either value or index
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
			request = strtok(NULL, " ");
			counter++;
		}
		
		bool error = false;
		bool found = false;
		
		if (strcmp (requestType,"CM") == 0){
			cout<<"Client: Create Monitor variable."<<endl;
			error = false;
			found = false;	
			//check if MV with the same name exist
			
			for (int i=0; i<request_counter; i++){
				if (requests)
			}
			
			
			for (int i=0; i<mv_index; i++){
				if (strcmp (name, MVs[i].name) == 0){
					cout<<"Monitor variable with name "<<name<<" already exists. Cannot create MV."<<endl;
					found = true;
				}
			}
			//check if array is full
			if (mv_index >= 200){
				cout<<"Cannot create MV, array is full."<<endl;
				error = true;
			}
			//if name is ok and array not full create new MV
			if (error == false && found == false){
				cout<<"Creating new MV."<<endl;
				MVs[mv_index].name = new char[30];
  				sprintf(MVs[mv_index].name, "%s", name);
 				MVs[mv_index].ID = mv_index;
 				MVs[mv_index].value = 0;	
 				
				sendReply(mv_index); //send reply with index to client

 				mv_index++;		//increment MV counter
			}
			else{
			//else if name not ok or array is full send -1 to client
				sendReply(-1);
			}	
		}
	}
}