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
void goToPassClerkLine(int);
void picGetCustomer(int);
void appGetCustomer(int);
void passGetCustomer(int);

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
	int bribeLineCount;
	int money;
	int ssn;
	ClerkState state;
	bool bribe;
	//constructor that initializes
	ApplicationClerk(string n){
		this->name = n;
		this->money = 0; //money is set to 0 
		this->state = available; //clerk is available
		this->lineCount = 0; //no one is in line
		this->bribeLineCount = 0;
		this->bribe = false;
	}	
};

//picture clerk struct
struct PictureClerk{

	string name;
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	bool pic;
	ClerkState state;
	bool bribe;

	//constructor that initialize
	PictureClerk(string n){
		this->name = n;
		this->money = 0; //money is set to 0
		this->state = available; //clerk is available
		this->lineCount = 0; //no one is in line
		this->bribeLineCount = 0;//no one in bribe line
		this->bribe = false;
	}
};

//struct passport struct
struct PassportClerk{

	string name;
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	bool bribe;
	ClerkState state;

	//constructor that initialize		
	PassportClerk(string n){
		this->name = n;
		this->money = 0; //money is set to 0
		this->state = available; //clerk is available
		this->lineCount = 0; //no one is in line
		this->bribeLineCount = 0; //no one is in line
		this->bribe = false;
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
vector<Lock*> AppClerkBribeLineLock; //lock used for bribe line
vector<Condition*> AppClerkBribeLineCV; //cv used for bribe line


//picture clerk and their locks and cv
vector<PictureClerk*> picture_clerks;

vector<Lock*> PicClerkLock; //lock used to interact- at the register
vector<Condition*> PicClerkCV; //cv used to interact- at the register
vector<Lock*> PicClerkLineLock; //lock used for line
vector<Condition*> PicClerkLineCV; //cv used for line
Lock PickPicClerkLineLock("PickPicClerkLineLock"); //lock used to pick which pic clerk line to go to
vector<Lock*> PicClerkBribeLineLock; //lock used for bribe line
vector<Condition*> PicClerkBribeLineCV; //cv used for bribe line


//passport clerk and their locks and cv
vector<PassportClerk*> passport_clerks;

vector<Lock*> PassClerkLock; //lock used to interact- at the register
vector<Condition*> PassClerkCV; //cv used to interact- at the register
vector<Lock*> PassClerkLineLock; //lock used for line
vector<Condition*> PassClerkLineCV; //cv used for line
Lock PickPassClerkLineLock("PickPassClerkLineLock"); //lock used to pick which pic clerk line to go to
vector<Lock*> PassClerkBribeLineLock; //lock used for bribe line
vector<Condition*> PassClerkBribeLineCV; //cv used for bribe line


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
	int bribeLineSize = 1000;
	PickPicClerkLineLock.Acquire(); //customer acquires pick lock	
	//for each picture clerk (1-5)
	bool bribed = false;
	if (customers[arg]->money > 100){
		for (unsigned int i=0; i<picture_clerks.size(); i++){
		//if the clerk is not on break and line is shorter than the previous line I checked (if first time, it is satisfied)
			if(picture_clerks[i]->bribeLineCount<bribeLineSize && picture_clerks[i]->state != onBreak){
				myLine = i; //this is my new line
				bribeLineSize = picture_clerks[i]->bribeLineCount; //line size is set to this line's size
			}
		}
		
		bribed = true;
	}
	else{
		for (unsigned int i=0; i<picture_clerks.size(); i++){
		//if the clerk is not on break and line is shorter than the previous line I checked (if first time, it is satisfied)
			if(picture_clerks[i]->lineCount<lineSize && picture_clerks[i]->state != onBreak){
				myLine = i; //this is my new line
				lineSize = picture_clerks[i]->lineCount; //line size is set to this line's size
			}
		}
		
		bribed = false;
	}
	PickPicClerkLineLock.Release(); //since the customer chose a line, release the lock so other customer can get in line
	//if my clerk is busy
	
	if (bribed == false){
		PicClerkLineLock[myLine]->Acquire(); //acquire my clerk's line
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
		picture_clerks[myLine]->bribe = false;
		PicClerkLineLock[myLine]->Release(); //since I am at the register, release the line lock
	}
	else{
		PicClerkBribeLineLock[myLine]->Acquire(); //acquire my clerk's line
 		if(picture_clerks[myLine]->state == busy){
			picture_clerks[myLine]->bribeLineCount++; //get in line- increment the size of the line
			cout<<currentThread->getName()<<" has gotten in bribe line for "<< picture_clerks[myLine]->name<<"\n";
			PicClerkBribeLineCV[myLine]->Wait(PicClerkBribeLineLock[myLine]); //wait for the clerk to signal 'me'
			picture_clerks[myLine]->bribeLineCount--; //get out of line to go to the counter- decrement
			picture_clerks[myLine]->bribe = true;
			customers[arg]->money = customers[arg]->money - 500;
		}
		else{ //if my clerk is available
			cout<<currentThread->getName()<<"at the register of "<<picture_clerks[myLine]->name<<endl;
		 	picture_clerks[myLine]->state = busy; //go to the clerk and set his/her state busy
		 	picture_clerks[myLine]->bribe = false;
		}
					
		PicClerkBribeLineLock[myLine]->Release(); //since I am at the register, release the line lock
	}
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
		goToPicClerkLine(arg);
		customers[arg]->liked--;
		PicClerkLock[myLine]->Acquire();
	}
	//PicClerkCV[myLine]->Signal(PicClerkLock[myLine]); I think i don't need it now ince already signal

	PicClerkLock[myLine]->Release();
	if (customers[arg]->clerk_pick<= 10){
	//if (customers[arg]->clerk_pick <= 10){ // got to pic clerk first
		goToAppClerkLine(arg);
	}
	else{
		if(customers[arg]->liked == 0 && customers[arg]->atPassClerk == false){
			cout<<currentThread->getName()<< " now I need to go to passport clerk"<<endl;
			goToPassClerkLine(arg);
			//customers[arg]->atPassClerk = true; //testing***
		}
	}
}

//pic clerk
void picGetCustomer(int arg){
	while (true){
		int myLine = arg;
		if(picture_clerks[myLine]->bribeLineCount>0){
			PicClerkBribeLineLock[myLine]->Acquire();
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			PicClerkBribeLineCV[myLine]->Signal(PicClerkBribeLineLock[myLine]);
 			picture_clerks[myLine]->state = busy;
 			PicClerkBribeLineLock[myLine]->Release();
		}
		else if(picture_clerks[myLine]->lineCount>0){
			PicClerkLineLock[myLine]->Acquire();
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			PicClerkLineCV[myLine]->Signal(PicClerkLineLock[myLine]);
 			picture_clerks[myLine]->state = busy;
 			PicClerkLineLock[myLine]->Release();
		}
  		else{
 			picture_clerks[myLine]->state = available;
 		}	 
 	
 		PicClerkLock[myLine]->Acquire();
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
			if (picture_clerks[arg]->bribe == true){
						cout<<currentThread->getName()<<" has received $500 from Customer["<<picture_clerks[myLine]->ssn<<"]"<<endl;
						picture_clerks[arg]->money = picture_clerks[arg]->money + 500;
						picture_clerks[arg]->bribe = false;
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
			if (picture_clerks[arg]->bribe == true){
						cout<<currentThread->getName()<<" has received $500 from Customer["<<picture_clerks[myLine]->ssn<<"]"<<endl;
						picture_clerks[arg]->money = picture_clerks[arg]->money + 500;
						picture_clerks[arg]->bribe = false;
					}
			cout<<currentThread->getName()<<" has been told that Customer["<<picture_clerks[myLine]->ssn<<"] does not like their picture"<<endl;
		}
		
		PicClerkLock[myLine]->Release();
	}

}

//customer
void goToAppClerkLine(int arg){

	int myLine = -1;
	int lineSize = 1000;
	int bribeLineSize = 1000;
	PickAppClerkLineLock.Acquire();
	bool bribed = false;
	if (customers[arg]->money > 100){
		for (unsigned int i = 0; i<application_clerks.size(); i++){
				if(application_clerks[i]->bribeLineCount<bribeLineSize && application_clerks[i]->state != onBreak){
					myLine = i;
					bribeLineSize = application_clerks[i]->bribeLineCount;
				}
		}
		//application_clerks[myLine]->bribe = true;
		bribed = true;
	}
	else{
		for (unsigned int i=0; i<application_clerks.size(); i++){
			if(application_clerks[i]->lineCount<lineSize && application_clerks[i]->state != onBreak){
				myLine = i;
				lineSize = application_clerks[i]->lineCount;
			}
		}
		//application_clerks[myLine]->bribe = false;
		bribed = false;
	}
	PickAppClerkLineLock.Release();
	
	if (bribed == false){ //if customer didn'`t bribe
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
		application_clerks[myLine]->bribe = false;
		AppClerkLineLock[myLine]->Release();
	}
	else{ //customer bribed
		AppClerkBribeLineLock[myLine]->Acquire();
		if(application_clerks[myLine]->state == busy){
			application_clerks[myLine]->bribeLineCount++;	
			cout<<currentThread->getName()<<" has gotten in bribe line for "<< application_clerks[myLine]->name<<"\n";
			AppClerkBribeLineCV[myLine]->Wait(AppClerkBribeLineLock[myLine]);
			application_clerks[myLine]->bribeLineCount--;	
			application_clerks[myLine]->bribe = true;
			customers[arg]->money = customers[arg]->money - 500;
		}
		else{
			cout<<currentThread->getName()<<"at the register of "<<application_clerks[myLine]->name<<endl;
		 	application_clerks[myLine]->state = busy;
		 	application_clerks[myLine]->bribe = false;
		}
		AppClerkBribeLineLock[myLine]->Release();	
	}
	AppClerkLock[myLine]->Acquire();
	application_clerks[myLine]->ssn = arg;
	cout<<currentThread->getName()<<" has given SSN ["<<arg<<"] to ApplicationClerk["<<myLine<<"]"<<endl;
	AppClerkCV[myLine]->Signal(AppClerkLock[myLine]);
	
	AppClerkCV[myLine]->Wait(AppClerkLock[myLine]); //no need to wait
	//customers[arg]->atAppClerk = true;
	AppClerkLock[myLine]->Release();

	if (customers[arg]->clerk_pick > 10){	
//	if (customers[arg]->clerk_pick > 10){ //go to app clerk first
		goToPicClerkLine(arg);
	}
	else{
		cout<<currentThread->getName()<< " now I need to go to passport clerk- app"<<endl;
		goToPassClerkLine(arg);
		//customers[arg]->atPassClerk = true; //testing***

	}
}


//clerk
void appGetCustomer(int arg){

	 while(true){
 		int myLine = arg;	
		if (application_clerks[myLine]->bribeLineCount>0){
			AppClerkBribeLineLock[myLine]->Acquire();
			cout<<currentThread->getName()<<" has signalled a Customer to come to their counter \n";
			AppClerkBribeLineCV[myLine]->Signal(AppClerkBribeLineLock[myLine]);
			application_clerks[myLine]->state = busy;
			AppClerkBribeLineLock[myLine]->Release();
		}
 		else if(application_clerks[myLine]->lineCount>0){
 			AppClerkLineLock[myLine]->Acquire();
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			AppClerkLineCV[myLine]->Signal(AppClerkLineLock[myLine]);
 			application_clerks[myLine]->state = busy;
 			AppClerkLineLock[myLine]->Release();
		}
  		else{
 			application_clerks[myLine]->state = available;
 		}	 
 		AppClerkLock[myLine]->Acquire();
		
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
		if (application_clerks[arg]->bribe == true){
			cout<<currentThread->getName()<<" has received $500 from Customer["<<application_clerks[myLine]->ssn<<"]"<<endl;
			application_clerks[arg]->money = application_clerks[arg]->money + 500;
			application_clerks[arg]->bribe = false;
		}
		//AppClerkCV[myLine]->Wait(AppClerkLock[myLine]);
		
		AppClerkLock[myLine]->Release();
	}
}

void goToPassClerkLine(int arg){

	int myLine = -1;
	int lineSize = 1000;
	int bribeLineSize = 1000;
	bool bribed = false;
	PickPassClerkLineLock.Acquire();
	if (customers[arg]->money>100){
		for (unsigned int i=0; i<passport_clerks.size(); i++){
			if(passport_clerks[i]->bribeLineCount<lineSize && passport_clerks[i]->state != onBreak){
				myLine = i;
				bribeLineSize = passport_clerks[i]->bribeLineCount;
			}
		}
		bribed = true;
	}
	else{
		for (unsigned int i=0; i<passport_clerks.size(); i++){
			if(passport_clerks[i]->lineCount<lineSize && passport_clerks[i]->state != onBreak){
				myLine = i;
				lineSize = passport_clerks[i]->lineCount;
			}
		}
		bribed = false;
	}
	PickPassClerkLineLock.Release();
	if (bribed == false){
		PassClerkLineLock[myLine]->Acquire();
 			if(passport_clerks[myLine]->state == busy){
				passport_clerks[myLine]->lineCount++;	
				cout<<currentThread->getName()<<" has gotten in regular line for "<< passport_clerks[myLine]->name<<"\n";
				PassClerkLineCV[myLine]->Wait(PassClerkLineLock[myLine]);
				passport_clerks[myLine]->lineCount--;	
		}
			else{
				cout<<currentThread->getName()<<"at the register of "<<passport_clerks[myLine]->name<<endl;
		 		passport_clerks[myLine]->state = busy;
		}
		application_clerks[myLine]->bribe = false;
		PassClerkLineLock[myLine]->Release();
	}
	else{
		PassClerkBribeLineLock[myLine]->Acquire();
 			if(passport_clerks[myLine]->state == busy){
				passport_clerks[myLine]->bribeLineCount++;	
				cout<<currentThread->getName()<<" has gotten in bribe line for "<< passport_clerks[myLine]->name<<"\n";
				PassClerkBribeLineCV[myLine]->Wait(PassClerkBribeLineLock[myLine]);
				passport_clerks[myLine]->bribeLineCount--;	
				application_clerks[myLine]->bribe = true;
				customers[arg]->money = customers[arg]->money - 500;
		}
			else{
				cout<<currentThread->getName()<<"at the register of "<<passport_clerks[myLine]->name<<endl;
		 		passport_clerks[myLine]->state = busy;
		 		application_clerks[myLine]->bribe = false;
		}
		PassClerkBribeLineLock[myLine]->Release();
	}
	PassClerkLock[myLine]->Acquire();
	passport_clerks[myLine]->ssn = arg;
	cout<<currentThread->getName()<<" has given SSN ["<<arg<<"] to PassportClerk["<<myLine<<"]"<<endl;
	PassClerkCV[myLine]->Signal(PassClerkLock[myLine]);
	PassClerkCV[myLine]->Wait(PassClerkLock[myLine]);
	
	for(unsigned int k=0; k<customer_data.size(); k++){
		if(customer_data[k]->SSN == arg){
			if(customer_data[k]->verified == false){
				int yield_random = rand() % 901 + 100;
				for(int i=0; i<yield_random; i++){
					currentThread->Yield();
				}
				//go back to the end of the line
				goToPassClerkLine(arg);					
			}
			break;
		}
	}
	
	customers[myLine]->atPassClerk = true;
	PassClerkLock[myLine]->Release();
	
}

void passGetCustomer(int arg){
	while(true){
 		int myLine = arg;
 		if(passport_clerks[myLine]->bribeLineCount>0){
 			PassClerkBribeLineLock[myLine]->Acquire();	
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			PassClerkBribeLineCV[myLine]->Signal(PassClerkBribeLineLock[myLine]);
 			passport_clerks[myLine]->state = busy;
 			PassClerkBribeLineLock[myLine]->Release();
		}
 		else if(passport_clerks[myLine]->lineCount>0){
 			PassClerkLineLock[myLine]->Acquire();	
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			PassClerkLineCV[myLine]->Signal(PassClerkLineLock[myLine]);
 			passport_clerks[myLine]->state = busy;
 			PassClerkLineLock[myLine]->Release();
		}
  		else{
 			passport_clerks[myLine]->state = available;
 		}	 
 	
 		PassClerkLock[myLine]->Acquire();
 //wait for Customer Data 
		PassClerkCV[myLine]->Wait(PassClerkLock[myLine]); 
		cout<<currentThread->getName()<<" has received SSN ["<<passport_clerks[myLine]->ssn<<"] from Customer ["<<passport_clerks[myLine]->ssn<<"]"<<endl;
		
		for(unsigned int k=0; k<customer_data.size(); k++){
			if(customer_data[k]->SSN == passport_clerks[myLine]->ssn)
			{
				//customer_data[k]->picture = true;
				if(customer_data[k]->application == true && customer_data[k]->picture == true){
		
					cout<<currentThread->getName()<<" had determined that Customer ["<<passport_clerks[myLine]->ssn<<"] has both their application and picture completed"<<endl;
					customer_data[k]->verified = true;
					if (passport_clerks[arg]->bribe == true){
						cout<<currentThread->getName()<<" has received $500 from Customer["<<passport_clerks[myLine]->ssn<<"]"<<endl;
						passport_clerks[arg]->money = passport_clerks[arg]->money + 500;
						passport_clerks[arg]->bribe = false;
					}
					int yield_random = rand() % 81 + 20;
					for(int g=0; g<yield_random; g++){
						currentThread->Yield();
					}
					customer_data[k]->verified = true;
					customers[customer_data[k]->SSN]->atPassClerk = true;
					PassClerkCV[myLine]->Signal(PassClerkLock[myLine]);
					cout<<currentThread->getName()<<" has recorded Customer ["<<passport_clerks[myLine]->ssn<<"] passport documentation"<<endl;
				}
				else{
					if (passport_clerks[arg]->bribe == true){
						cout<<currentThread->getName()<<" has received $500 from Customer["<<passport_clerks[myLine]->ssn<<"]"<<endl;
						passport_clerks[arg]->money = passport_clerks[arg]->money + 500;
						passport_clerks[arg]->bribe = false;
					}
					cout<<currentThread->getName()<<" had determined that Customer ["<<passport_clerks[myLine]->ssn<<"] does not have both their application and picture completed"<<endl;
					
				}	
				break;	
			}
		}
		
		//PassClerkCV[myLine]->Wait(PassClerkLock[myLine]);
		
		PassClerkLock[myLine]->Release();
	}
}


void ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}
char *name;
char *app_name;
char *pic_name;
char *pass_name;

void Problem2(){

	Thread *thread;
	Customer *customer;
	
	ApplicationClerk *app_clerk;
	Lock *appClerkLineLock;
    Condition *appClerkLineCV;
    Lock *appClerkBribeLineLock;
    Condition *appClerkBribeLineCV;
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
		
		sprintf(name,"AppClerkBribeLineLock[%d]",i);
		
		appClerkBribeLineLock  = new Lock(name);
		AppClerkBribeLineLock.push_back(appClerkBribeLineLock);
		
		sprintf(name,"AppClerkBribeLineCV[%d]",i);
		
		appClerkBribeLineCV = new Condition(name);
		AppClerkBribeLineCV.push_back(appClerkBribeLineCV);
		
		
		thread = new Thread(app_name);
		thread->Fork((VoidFunctionPtr)appGetCustomer,i);
		
		name="";
	}
	
	PictureClerk *pic_clerk;
	Lock *picClerkLineLock;
    Condition *picClerkLineCV;
    Lock *picClerkBribeLineLock;
    Condition *picClerkBribeLineCV;
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
		
		sprintf(name,"PicClerkBribeLineLock[%d]",i);
		
		picClerkBribeLineLock  = new Lock(name);
		PicClerkBribeLineLock.push_back(picClerkBribeLineLock);
		
		sprintf(name,"PicClerkBribeLineCV[%d]",i);
		
		picClerkBribeLineCV = new Condition(name);
		PicClerkBribeLineCV.push_back(picClerkBribeLineCV);
		
		thread = new Thread(pic_name);
		thread->Fork((VoidFunctionPtr)picGetCustomer,i);
		
		name="";
    }
    
    PassportClerk *pass_clerk;
    Lock *passClerkLineLock;
    Condition *passClerkLineCV;
    Lock *passClerkBribeLineLock;
    Condition *passClerkBribeLineCV;
    Lock *passClerkLock;
    Condition *passClerkCV;
    
    for(int j=0; j<2; j++){
		name = new char [20];
		pass_name = new char [20];
		sprintf(pass_name, "PassportClerk[%d]",j);
		
		pass_clerk = new PassportClerk(pass_name);
		passport_clerks.push_back(pass_clerk);
		
		sprintf(name,"PassClerkLineLock[%d]",j);
		
		passClerkLineLock  = new Lock(name);
		PassClerkLineLock.push_back(passClerkLineLock);
		
		sprintf(name,"PassClerkLineCV[%d]",j);
		
		passClerkLineCV = new Condition(name);
		PassClerkLineCV.push_back(passClerkLineCV);
		
		sprintf(name,"PassClerkLock[%d]",j);
		
		passClerkLock  = new Lock(name);
		PassClerkLock.push_back(passClerkLock);
		
		sprintf(name,"PassClerkCV[%d]",j);
		
		passClerkCV  = new Condition(name);
		PassClerkCV.push_back(passClerkCV);
		
		sprintf(name,"PassClerkBribeLineLock[%d]",j);
		
		passClerkBribeLineLock  = new Lock(name);
		PassClerkBribeLineLock.push_back(passClerkBribeLineLock);
		
		sprintf(name,"PassClerkBribeLineCV[%d]",j);
		
		passClerkBribeLineCV = new Condition(name);
		PassClerkBribeLineCV.push_back(passClerkBribeLineCV);
		
		
		thread = new Thread(pass_name);
		thread->Fork((VoidFunctionPtr)passGetCustomer,j);
		
		name="";
	}
	
	for(int i=0; i<7; i++){
		name = new char [20];
		sprintf(name,"Customer[%d]",i);
		
		customer = new Customer(name, i);
		//customer->money = 100;
		customers.push_back(customer);
		
		cout<<customer->name<<" has "<<customer->money<<endl;
		
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

