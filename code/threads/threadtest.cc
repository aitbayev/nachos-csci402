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

#include "list.h"
using namespace std;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

enum CustomerState {arrivedAtOffice, atAppClerk, atPictureClerk, atPassportClerk, atCashier};
enum AppClerkState {busy, available, onBreak};
Lock AppClerkLineLock("AppClerkLineLock");
Condition AppClerkLineCV("AppClerkLineCV");
Lock AppClerkLock("AppClerkLock");
List customer_list;

struct Customer{
	string name;
	int money;
	bool application;
	int social_security;
	CustomerState state;
	
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
	state = arrivedAtOffice;
}
};

struct ApplicationClerk{

	string name;
	int lineCount;
	int money;
	AppClerkState state;
	
	
	ApplicationClerk(string n){
		this->name = n;
		this->money = 0;
		this->state = busy;
		this->lineCount = 0;
	}
	
};

ApplicationClerk *app_clerk = NULL;

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

void goToLine(){
	AppClerkLineLock.Acquire();
	Customer *c = new Customer("customer1", 11111111);
		if(app_clerk->state == busy){
			cout<< c->name<<" App clerk is busy \n";
			app_clerk->lineCount++;	
			AppClerkLineCV.Wait(&AppClerkLineLock);
			app_clerk->lineCount--;	
		}
		else{
			cout<< c->name<<" App clerk is available \n";
			app_clerk->state = busy;
		}
	
	AppClerkLineLock.Release();
}

void getCustomer(){
	ApplicationClerk *appClerk = new ApplicationClerk("appClerk1");
	app_clerk = appClerk;
	while(true){
		AppClerkLineLock.Acquire();
		if(appClerk->lineCount>0){
			cout<< appClerk->name<<" someone is in my Line \n";
			AppClerkLineCV.Signal(&AppClerkLineLock);
			appClerk->state = busy;
			
		}
		else{
			cout<< appClerk->name<<" nobody is in my line \n";
			appClerk->state = available;
			
		}
		AppClerkLineLock.Release();
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

void Problem2(){
	Thread *appClerk = new Thread("appClerk1");
	appClerk->Fork((VoidFunctionPtr)getCustomer,0);
	Thread *customer = new Thread("customer1");
	customer->Fork((VoidFunctionPtr)goToLine,0);
	customer = new Thread("customer2");
	customer->Fork((VoidFunctionPtr)goToLine,0);
	
}

