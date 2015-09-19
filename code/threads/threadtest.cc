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
#include "test_code.cc"
#include <iostream>
#include <string>
#include <cstdlib>
#include "synch.h"
#include <vector>

using namespace std;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

enum CustomerState {atAppClerk, atPictureClerk, atPassportClerk, atCashier};
enum ClerkState {busy, available, onBreak};



struct CustomerData{
	string name;
	int SSN;
	bool application;
	bool picture;
	bool verified;
	bool got_passport;
	
	CustomerData(int s){
		this->SSN = s;
	}
	
};

//vector <*CustomerData> customer_data;
int customers_count = 0;

struct Customer{
	string name;
	int money;
	bool application;
	int social_security;
	bool atAppClerk;
	bool atPicClerk;
	bool atPassClerk;
	bool atCashier;
	
Customer(string n, int ss){
	this->name = n;
	this->social_security = ss;
	application = true;
	int money_rand = rand() % 4;
	if (money_rand == 0){
		money = 100;
}
	else if (money_rand == 1){
		money = 600;
}
	else if (money_rand == 2){
		money = 1100;
}
	else if (money_rand == 3){
		money = 1600;
}
	 atAppClerk = false;
	 atPicClerk = false;
	 atPassClerk = false;
	 atCashier = false;
	
}
};



struct ApplicationClerk{

	string name;
	int lineCount;
	int money;
	ClerkState state;
	int ssn;
	
	
	ApplicationClerk(string n){
		this->name = n;
		this->money = 0;
		this->state = busy;
		this->lineCount = 0;
	}
	
	
};


struct PictureClerk{

	string name;
	int lineCount;
	int money;
	ClerkState state;
	
	PictureClerk(string n){
		this->name = n;
		this->money = 0;
		this->state = busy;
		this->lineCount = 0;
	}
};

vector<CustomerData*> customer_data;
int counter = 0;

vector<Customer*> customers;

//application clerks and their locks and CV
vector<ApplicationClerk*> application_clerks;

vector<Lock*> AppClerkLock;
vector<Condition*> AppClerkCV;
vector<Lock*> AppClerkLineLock;
vector<Condition*> AppClerkLineCV;
Lock PickAppClerkLineLock("PickAppClerkLineLock");


//picture clerk and their data
vector<PictureClerk*> picture_clerks;
vector<Lock*> PicClerkLock;
vector<Condition*> PicClerkCV;
Lock PicClerkLineLock("PicClerkLineLock");
Condition PicClerkLineCV("PicClerkLineCV");



void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
} 

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------



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
	
	AppClerkCV[myLine]->Wait(AppClerkLock[myLine]);
	customers[myLine]->atAppClerk = true;
	AppClerkLock[myLine]->Release();
	
}

//clerk
void getCustomer(int arg){

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

		CustomerData *c_d = new CustomerData(application_clerks[myLine]->ssn);
		c_d->application = true;
		customer_data.push_back(c_d);
		
		for(int i=0; i<20; i++){
			currentThread->Yield();
		}
		
		//yield
//do my job, customer now waiting 
		
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
		
        thread = new Thread(app_name);
		thread->Fork((VoidFunctionPtr)getCustomer,i);
		
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
		
		name="";
	}
	
	
	for(int i=0; i<3; i++){
		name = new char [20];
		sprintf(name,"Customer[%d]",i);
		
		thread = new Thread(name);
		thread->Fork((VoidFunctionPtr)goToAppClerkLine,i);
	
		customer = new Customer(name, i);
		customers.push_back(customer);
	}
		
}

