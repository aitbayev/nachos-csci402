// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "test_code.cc"
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <time.h>

using namespace std;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

//customer states
enum CustomerState {atAppClerk, atPictureClerk, atPassportClerk, atCashier};
//clerk states
enum ClerkState {busy, available, onBreak};
//predeclare functions
void goToAppClerkLine(int);
void goToPicClerkLine(int);
void picGetCustomer(int);
void appGetCustomer(int);

//customer data struct that stores customer information and passport status reports
struct CustomerData{
	string name;
	int SSN; //social security number
	bool application; //whether application clerk filed the application
	bool picture; //whether picture clerk took a picture and filed it
	bool verified; //whether passport clerk verified and filed it
	bool got_passport; //whether customer received his/her passport from cashier
	
	CustomerData(int s){ //constructor that sets social security number
		this->SSN = s;
	}
	
};

//customer struct
struct Customer{
	string name;
	int money; 
	int social_security;
	bool application; //application completed?
	bool atAppClerk; //whether customer went to app clerk
	bool atPicClerk; //whether customer went to pic clerk
	bool atPassClerk; //whether customer went to passport clerk
	bool atCashier; //whether customer went to cashier
	int liked; 
	int pic_liking; //probability of liking the picture
	int clerk_pick; //which clerk to go first- app or pic clerk?
	
	//customer constructor
	Customer(string n, int ss){
		this->name = n;
		this->social_security = ss;
		liked = 0;
		application = true; //application is completed by the customer
		
		int money_rand = rand() % 4; //randomize the amount of money customer will have
		if (money_rand == 0){
			money = 100;
			pic_liking = 25;
		}
		else if (money_rand == 1){
			money = 600;
			pic_liking = 50;
		}
		else if (money_rand == 2){
			money = 1100;
			pic_liking = 75;
		}
		else if (money_rand == 3){
			money = 1600;
			pic_liking = 90;
		}
	
		clerk_pick = rand() % 20; //randomize which clerk to go to first- app or pic
	
		//initially, the customer hasn't gone to any of these clerks/cashier yet
	 	atAppClerk = false;
	 	atPicClerk = false;
	 	atPassClerk = false;
	 	atCashier = false;
	}
};

//application clerk struct
struct ApplicationClerk{

	string name;
	int lineCount;
	int money;
	int ssn;
	ClerkState state;
	
	//constructor that initializes
	ApplicationClerk(string n){
		this->name = n;
		this->money = 0; //money is set to 0 
		this->state = available; //clerk is available
		this->lineCount = 0; //no one is in line
	}	
};

//picture clerk struct
struct PictureClerk{

	string name;
	int lineCount;
	int money;
	int ssn;
	bool pic;
	ClerkState state;

	//constructor that initialize
	PictureClerk(string n){
		this->name = n;
		this->money = 0; //money is set to 0
		this->state = available; //clerk is available
		this->lineCount = 0; //no one is in line
	}
};

vector<CustomerData*> customer_data; //customer data storage globally declared
vector<Customer*> customers; //customers

//application clerks and their locks and CV
vector<ApplicationClerk*> application_clerks;

vector<Lock*> AppClerkLock; //lock used to interact- at the register
vector<Condition*> AppClerkCV; //cv used to interact- at the register
vector<Lock*> AppClerkLineLock; //lock used for line
vector<Condition*> AppClerkLineCV; //cv used for line
Lock PickAppClerkLineLock("PickAppClerkLineLock"); //lock used to pick which app clerk line to go to

//picture clerk and their locks and cv
vector<PictureClerk*> picture_clerks;

vector<Lock*> PicClerkLock; //lock used to interact- at the register
vector<Condition*> PicClerkCV; //cv used to interact- at the register
vector<Lock*> PicClerkLineLock; //lock used for line
vector<Condition*> PicClerkLineCV; //cv used for line
Lock PickPicClerkLineLock("PickPicClerkLineLock"); //lock used to pick which pic clerk line to go to

void SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
} 

//customer function- going to pic clerk line
void goToPicClerkLine(int arg){
	int myLine = -1; //my line is initially set to -1
	int lineSize = 1000; //maximum number of customers in a line
	
	PickPicClerkLineLock.Acquire(); //customer acquires pick lock
	
	//for each picture clerk (1-5)
	for (unsigned int i=0; i<picture_clerks.size(); i++){
		//if the clerk is not on break and line is shorter than the previous line I checked (if first time, it is satisfied)
		if(picture_clerks[i]->lineCount<lineSize && picture_clerks[i]->state != onBreak){
			myLine = i; //this is my new line
			lineSize = picture_clerks[i]->lineCount; //line size is set to this line's size
		}
	}
	PickPicClerkLineLock.Release(); //since the customer chose a line, release the lock so other customer can get in line
	PicClerkLineLock[myLine]->Acquire(); //acquire my clerk's line
	
	//if my clerk is busy
 	if(picture_clerks[myLine]->state == busy){
			picture_clerks[myLine]->lineCount++; //get in line- increment the size of the line
			cout<<currentThread->getName()<<" has gotten in regular line for "<< picture_clerks[myLine]->name<<"\n";
			PicClerkLineCV[myLine]->Wait(PicClerkLineLock[myLine]); //wait for the clerk to signal 'me'
			picture_clerks[myLine]->lineCount--; //get out of line to go to the counter- decrement
	}
	else{ //if my clerk is available
			cout<<currentThread->getName()<<"at the register of "<<picture_clerks[myLine]->name<<endl;
		 	picture_clerks[myLine]->state = busy; //go to the clerk and set his/her state busy
	}
	
	PicClerkLineLock[myLine]->Release(); //since I am at the register, release the line lock
	PicClerkLock[myLine]->Acquire(); //acquire my clerk's lock
	
	picture_clerks[myLine]->ssn = arg; //give my social security number to the clerk
	cout<<currentThread->getName()<<" has given SSN ["<<arg<<"] to PictureClerk["<<myLine<<"]"<<endl;
	PicClerkCV[myLine]->Signal(PicClerkLock[myLine]); //signal the clerk that I gave the social security number
	PicClerkCV[myLine]->Wait(PicClerkLock[myLine]); //wait until clerk signals me
	
	int pic = rand() % 100 + 1; //randomize for a number
	
	//if the number less than or equal to the customer's pre-defined pic probability, customer likes it
	if (pic <= customers[arg]->pic_liking){
		cout<<currentThread->getName()<<"does like their picture from PictureClerk["<<myLine<<"]"<<endl;//liked the picture
		picture_clerks[myLine]->pic = true; //picture is taken and accepted by the customer
		PicClerkCV[myLine]->Signal(PicClerkLock[myLine]); //signal

	}
	else {
		cout<<currentThread->getName()<<"does not like their picture from PictureClerk["<<myLine<<"]"<<endl;//didn't like the picture
		picture_clerks[myLine]->pic = false;
		PicClerkLock[myLine]->Release();
		PicClerkCV[myLine]->Signal(PicClerkLock[myLine]);
		customers[arg]->liked++;
		cout << "**liked(i): " << customers[arg]->liked << endl;
		goToPicClerkLine(arg);
		customers[arg]->liked--;
		cout << "**liked(d): " << customers[arg]->liked << endl;
		PicClerkLock[myLine]->Acquire();
	}
	//PicClerkCV[myLine]->Signal(PicClerkLock[myLine]); I think i don't need it now ince already signal

	PicClerkLock[myLine]->Release();
	if (customers[arg]->atAppClerk == false){
	//if (customers[arg]->clerk_pick <= 10){ // got to pic clerk first
		goToAppClerkLine(arg);
	}
	else{
		if(customers[arg]->liked == 0 && customers[arg]->atPassClerk == false){
			cout<<currentThread->getName()<< " now I need to go to passport clerk"<<endl;
			customers[arg]->atPassClerk = true; //testing***
		}
	}
}

//pic clerk
void picGetCustomer(int arg){
	while (true){
		int myLine = arg;
		PicClerkLineLock[myLine]->Acquire();
		if(picture_clerks[myLine]->lineCount>0){
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			PicClerkLineCV[myLine]->Signal(PicClerkLineLock[myLine]);
 			picture_clerks[myLine]->state = busy;
		}
  		else{
 			picture_clerks[myLine]->state = available;
 		}	 
 	
 		PicClerkLock[myLine]->Acquire();
		PicClerkLineLock[myLine]->Release();
		PicClerkCV[myLine]->Wait(PicClerkLock[myLine]); 
		
		cout<<currentThread->getName()<<" has received SSN ["<<picture_clerks[myLine]->ssn<<"] from Customer ["<<picture_clerks[myLine]->ssn<<"]"<<endl;
		cout<<currentThread->getName()<<" has taken a picture of Customer["<<picture_clerks[myLine]->ssn<<"]"<<endl;
		PicClerkCV[myLine]->Signal(PicClerkLock[myLine]);
		PicClerkCV[myLine]->Wait(PicClerkLock[myLine]);
		bool flag = false;
		if (picture_clerks[myLine]->pic == true){
			cout<<currentThread->getName()<<" has been told that Customer["<<picture_clerks[myLine]->ssn<<"] does like their picture"<<endl;
			for (unsigned int i=0; i<customer_data.size(); i++){
				if(customer_data[i]->SSN == picture_clerks[myLine]->ssn){
					flag = true;
				}
			}
			if (flag == false){
				CustomerData *c_d = new CustomerData(application_clerks[myLine]->ssn);
				customer_data.push_back(c_d);				
			}
			for (unsigned int i=0; i<customer_data.size(); i++){
				if (customer_data[i]->SSN == picture_clerks[myLine]->ssn){
					int r = rand() % 81 +20;
					for (int k=0; k<r; k++){
						currentThread->Yield();
					}
					customer_data[i]->picture = true;
					customers[customer_data[i]->SSN]->atPicClerk = true;

				}
			}
			
		}
		else{
			cout<<currentThread->getName()<<" has been told that Customer["<<picture_clerks[myLine]->ssn<<"] does not like their picture"<<endl;
		}
		
		PicClerkLock[myLine]->Release();
	}

}

//customer
void goToAppClerkLine(int arg){

	int myLine = -1;
	int lineSize = 1000;
	PickAppClerkLineLock.Acquire();
	for (unsigned int i=0; i<application_clerks.size(); i++){
		if(application_clerks[i]->lineCount<lineSize && application_clerks[i]->state != onBreak){
			myLine = i;
			lineSize = application_clerks[i]->lineCount;
		}
	}
	PickAppClerkLineLock.Release();
	
	AppClerkLineLock[myLine]->Acquire();
 	if(application_clerks[myLine]->state == busy){
			application_clerks[myLine]->lineCount++;	
			cout<<currentThread->getName()<<" has gotten in regular line for "<< application_clerks[myLine]->name<<"\n";
			AppClerkLineCV[myLine]->Wait(AppClerkLineLock[myLine]);
			application_clerks[myLine]->lineCount--;	
	}
	else{
			cout<<currentThread->getName()<<"at the register of "<<application_clerks[myLine]->name<<endl;
		 	application_clerks[myLine]->state = busy;
	}
	AppClerkLineLock[myLine]->Release();
	AppClerkLock[myLine]->Acquire();
	application_clerks[myLine]->ssn = arg;
	cout<<currentThread->getName()<<" has given SSN ["<<arg<<"] to ApplicationClerk["<<myLine<<"]"<<endl;
	AppClerkCV[myLine]->Signal(AppClerkLock[myLine]);
	
	AppClerkCV[myLine]->Wait(AppClerkLock[myLine]); //no need to wait
	//customers[arg]->atAppClerk = true;
	AppClerkLock[myLine]->Release();

	if (customers[arg]->atPicClerk == false){	
//	if (customers[arg]->clerk_pick > 10){ //go to app clerk first
		goToPicClerkLine(arg);
	}
	else{
		cout<<currentThread->getName()<< " now I need to go to passport clerk- app"<<endl;
		customers[arg]->atPassClerk = true; //testing***

	}
}


//clerk
void appGetCustomer(int arg){

	 while(true){
 		int myLine = arg;
		AppClerkLineLock[myLine]->Acquire();	
 		 if(application_clerks[myLine]->lineCount>0){
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			AppClerkLineCV[myLine]->Signal(AppClerkLineLock[myLine]);
 			application_clerks[myLine]->state = busy;
		}
  		else{
 			application_clerks[myLine]->state = available;
 		}	 
 	
 		AppClerkLock[myLine]->Acquire();
		AppClerkLineLock[myLine]->Release();
		
 //wait for Customer Data 
		AppClerkCV[myLine]->Wait(AppClerkLock[myLine]); 
		cout<<currentThread->getName()<<" has received SSN ["<<application_clerks[myLine]->ssn<<"] from Customer ["<<application_clerks[myLine]->ssn<<"]"<<endl;
	    bool flag = false;
	    for (unsigned int i=0; i< customer_data.size(); i++){
	    	if (customer_data[i]->SSN == application_clerks[myLine]->ssn){
	    		flag = true; 
	    	}
	    }
	    if (flag == false){
			CustomerData *c_d = new CustomerData(application_clerks[myLine]->ssn);
			customer_data.push_back(c_d);
		}
		for (unsigned int i=0; i<customer_data.size(); i++){
			if (customer_data[i]->SSN == application_clerks[myLine]->ssn){
				int r = rand() % 81 +20;
				for (int k=0; k<r; k++){
					currentThread->Yield();		
				}
				customer_data[i]->application = true;
				customers[customer_data[i]->SSN]->atAppClerk = true;
			}
		}
		AppClerkCV[myLine]->Signal(AppClerkLock[myLine]);
		cout<<currentThread->getName()<<" has recorded a completed application for Customer["<<application_clerks[myLine]->ssn<<"]"<<endl;
		
		//AppClerkCV[myLine]->Wait(AppClerkLock[myLine]);
		
		AppClerkLock[myLine]->Release();
}
}

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}
char *name;
char *app_name;
char *pic_name;

void Problem2(){

	Thread *thread;
	
	Customer *customer;
	
	ApplicationClerk *app_clerk;
	Lock *appClerkLineLock;
    Condition *appClerkLineCV;
    Lock *appClerkLock;
    Condition *appClerkCV;

	for(int i=0; i<2; i++){
		name = new char [20];
		app_name = new char[20];
		sprintf(app_name,"ApplicationClerk[%d]",i);
		
		app_clerk = new ApplicationClerk(app_name);
		application_clerks.push_back(app_clerk);
		
		sprintf(name,"AppClerkLineLock[%d]",i);
		
		appClerkLineLock  = new Lock(name);
		AppClerkLineLock.push_back(appClerkLineLock);
		
		sprintf(name,"AppClerkLineCV[%d]",i);
		
		appClerkLineCV = new Condition(name);
		AppClerkLineCV.push_back(appClerkLineCV);
		
		sprintf(name,"AppClerkLock[%d]",i);
		
		appClerkLock  = new Lock(name);
		AppClerkLock.push_back(appClerkLock);
		
		sprintf(name,"AppClerkCV[%d]",i);
		
		appClerkCV  = new Condition(name);
		AppClerkCV.push_back(appClerkCV);
		
		thread = new Thread(app_name);
		thread->Fork((VoidFunctionPtr)appGetCustomer,i);
		
		name="";
	}
	
	PictureClerk *pic_clerk;
	Lock *picClerkLineLock;
    Condition *picClerkLineCV;
    Lock *picClerkLock;
    Condition *picClerkCV;
    
    for (int i=0; i<2; i++){
    	name = new char [20];
		pic_name = new char[20];
		sprintf(pic_name,"PictureClerk[%d]",i); 
		
		pic_clerk = new PictureClerk(pic_name);
		picture_clerks.push_back(pic_clerk);
		
		sprintf(name,"PicClerkLineLock[%d]",i);
		
		picClerkLineLock  = new Lock(name);
		PicClerkLineLock.push_back(picClerkLineLock);
		
		sprintf(name,"PicClerkLineCV[%d]",i);
		
		picClerkLineCV = new Condition(name);
		PicClerkLineCV.push_back(picClerkLineCV);
		
		sprintf(name,"PicClerkLock[%d]",i);
		
		picClerkLock  = new Lock(name);
		PicClerkLock.push_back(picClerkLock);
		
		sprintf(name,"PicClerkCV[%d]",i);
		
		picClerkCV  = new Condition(name);
		PicClerkCV.push_back(picClerkCV);
		
		thread = new Thread(pic_name);
		thread->Fork((VoidFunctionPtr)picGetCustomer,i);
		
		name="";
    }
	
	for(int i=0; i<5; i++){
		name = new char [20];
		sprintf(name,"Customer[%d]",i);
		
		customer = new Customer(name, i);
		customers.push_back(customer);
		//cout<<customer->clerk_pick<<endl;
		
		thread = new Thread(name);
		if (customer->clerk_pick >10){
			thread->Fork((VoidFunctionPtr)goToAppClerkLine,i);
			cout<<thread->getName()<<" goes to app clerk first"<<endl;
		}
		else {
			thread->Fork((VoidFunctionPtr)goToPicClerkLine,i);
			cout<<thread->getName()<<" goes to pic clerk first"<<endl;
		}
		
	
	}
	
		
}

