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

//functions for customers threads
void goToAppClerkLine(int);
void goToPicClerkLine(int);
void goToPassClerkLine(int);
void goToCashierLine(int);

//functions for clerk threads
void appGetCustomer(int);
void passGetCustomer(int);
void picGetCustomer(int);
void cashGetCustomer(int);


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
		application = false;
		picture = false;
		verified = false;
		got_passport = false;
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
	bool leftOffice;
	int liked; //keep track of pic-recursive
	int pass_punished; //keep track of punishment by pass clerk- recursive
	int cash_punished; //keep track of punishment by cashier- recursive
	int pic_liking; //probability of liking the picture
	int clerk_pick; //which clerk to go first- app or pic clerk?
	bool senator;
	
	//customer constructor
	Customer(string n, int ss){
		this->name = n;
		this->social_security = ss;
		liked = 0;
		pass_punished = 0;
		cash_punished = 0;

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
	 	leftOffice = false;
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

//passport clerk struct

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

//cashier struct
struct Cashier{
	
	string name;
	int lineCount;
	int money;
	int ssn;
	ClerkState state;
	
	//constructor that initialize
	Cashier(string n){
		this->name = n;
		this->money = 0; //money is set to 0
		this->state = available; //clerk is available
		this->lineCount = 0; //no one is in line
	}
};

//manager struct
struct Manager{

	string name;
	int appClerkMoney;
	int picClerkMoney;
	int passClerkMoney;
	int cashierMoney;
	int totalMoney;
	
	//constructor that initialize
	Manager(string n){
		this->name = n;
		this->appClerkMoney = 0;
		this->picClerkMoney = 0;
		this->passClerkMoney = 0;
		this->cashierMoney = 0;
		this->totalMoney = 0;
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

//cashier and their locks and cv
vector<Cashier*> cashier_clerks;

//manager
Manager manager_clerk("Manager");

vector<Lock*> CashierLock; //lock used to interact- at the register
vector<Condition*> CashierCV; //cv used to interact- at the register
vector<Lock*> CashierLineLock; //lock used for line
vector<Condition*> CashierLineCV; //cv used for line
Lock PickCashierLineLock("PickCashierLineLock"); //lock used to pick which cashier line to go to

int senator_counter = 0;
Condition SenatorAppCV("SenatorAppCV");
Condition SenatorPicCV("SenatorPicCV");
Condition SenatorPassCV("SenatorPassCV");
Condition SenatorCashCV("SenatorCashCV");

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
		 	picture_clerks[myLine]->state = busy; //go to the clerk and set his/her state busy
		}
		picture_clerks[myLine]->bribe = false;
		PicClerkLineLock[myLine]->Release(); //since I am at the register, release the line lock
	}
	else{ // bribed
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
		 	picture_clerks[myLine]->state = busy; //go to the clerk and set his/her state busy
		 	picture_clerks[myLine]->bribe = false;
		}			
		PicClerkBribeLineLock[myLine]->Release(); //since I am at the register, release the line lock
	}
	PicClerkLock[myLine]->Acquire(); //acquire my clerk's lock
	// for(int i=0; i<20; i++){
// 			currentThread->Yield();
// 		}
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
	//if the number is greater, customer does not like it
	else {
		cout<<currentThread->getName()<<"does not like their picture from PictureClerk["<<myLine<<"]"<<endl;//didn't like the picture
		picture_clerks[myLine]->pic = false; //picture is rejected
		PicClerkCV[myLine]->Signal(PicClerkLock[myLine]); //signal
		PicClerkLock[myLine]->Release(); //release the lock so other customer can access it
		customers[arg]->liked++; //keeps track of recursive-ness
		goToPicClerkLine(arg); //recursive call- get in line again to retake
		PicClerkLock[myLine]->Acquire(); //acquire the lock
		customers[arg]->liked--; //decrement every time customer returns from recursive call
	}

	PicClerkLock[myLine]->Release(); //release the lock- business done with picture clerk

	//if customer went to pic clerk first and did not go to passport clerk in recursive call
	if (customers[arg]->clerk_pick <= 10 && customers[arg]->atPassClerk == false){ 
		goToAppClerkLine(arg); //move onto app clerk
	}
	else{
		//if this is the only/last call (from recursion) and did not go to passport clerk in recursive call
		if(customers[arg]->liked == 0 && customers[arg]->atPassClerk == false){
			goToPassClerkLine(arg); //move onto passport clerk
		}
	}
}

//pic clerk
void picGetCustomer(int arg){
	while (true){
		int myLine = arg;
		if(picture_clerks[myLine]->bribeLineCount>0){ //if there is anyone in bribe line
			PicClerkBribeLineLock[myLine]->Acquire(); //acquire lock
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
				CustomerData *c_d = new CustomerData(picture_clerks[myLine]->ssn);
				customer_data.push_back(c_d);				
			}
			for (unsigned int i=0; i<customer_data.size(); i++){
				//if customer's data is found
				if (customer_data[i]->SSN == picture_clerks[myLine]->ssn){
					int r = rand() % 81 +20; //randomize yield
					//loop to call yield- wait time to file in picture
					for (int k=0; k<r; k++){
						currentThread->Yield(); 
					}
					customer_data[i]->picture = true; //picture is taken
					customers[picture_clerks[myLine]->ssn]->atPicClerk = true; //at picture clerk
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

//customer function- going to app clerk line
void goToAppClerkLine(int arg){
	int myLine = -1;//my line is initially set to -1
	int lineSize = 1000;//maximum number of customers in a line
	int bribeLineSize = 1000; //maximum number of customers in a line
	bool bribed = false; // bribed initially false
	PickAppClerkLineLock.Acquire();	//acquire pick app clerk line lock
	if (customers[arg]->money > 100){
		for (unsigned int i = 0; i<application_clerks.size(); i++){
				if(application_clerks[i]->bribeLineCount<bribeLineSize && application_clerks[i]->state != onBreak){
				//if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)
					myLine = i; //this is my new line
					bribeLineSize = application_clerks[i]->bribeLineCount;// bribe line size is set to this line's size
					}
		}
		bribed = true; //we can bribe
	}
	else{
		for (unsigned int i=0; i<application_clerks.size(); i++){
			if(application_clerks[i]->lineCount<lineSize && application_clerks[i]->state != onBreak){
			//if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)
				myLine = i; //this is my new line
				lineSize = application_clerks[i]->lineCount; //line size is set to this line's size
			}
		}
		bribed = false; //can't bribe
	}
	PickAppClerkLineLock.Release(); //release line picking lock 
	
	if (bribed == false){ //if customer didn'`t bribe
		AppClerkLineLock[myLine]->Acquire();
		if(application_clerks[myLine]->state == busy){ // if clerk is not busy
			application_clerks[myLine]->lineCount++; //customer gets in line- increment line number	
			cout<<currentThread->getName()<<" has gotten in regular line for "<< application_clerks[myLine]->name<<"\n";
			AppClerkLineCV[myLine]->Wait(AppClerkLineLock[myLine]); //wait on clerks waiting queue until he signal customer
			application_clerks[myLine]->lineCount--;	//decrement line count of the clerk, means he signalled customer
			
		}
		else{
		 	application_clerks[myLine]->state = busy; //else he is busy going right to register
		}
		application_clerks[myLine]->bribe = false; // not bribing
		AppClerkLineLock[myLine]->Release();
	}
	else{ //customer bribed
		AppClerkBribeLineLock[myLine]->Acquire();
		if(application_clerks[myLine]->state == busy){ // if clerk is not busy
			application_clerks[myLine]->bribeLineCount++;	//customer gets in line- increment line number	
			cout<<currentThread->getName()<<" has gotten in bribe line for "<< application_clerks[myLine]->name<<"\n";
			AppClerkBribeLineCV[myLine]->Wait(AppClerkBribeLineLock[myLine]);//wait on clerks waiting queue until he signal customer
			application_clerks[myLine]->bribeLineCount--;	// decrement line count of the clerk, means he signalled customer
			application_clerks[myLine]->bribe = true; //customer is bribing
			customers[arg]->money = customers[arg]->money - 500; // reducing customer's money
		}
		else{
		 	application_clerks[myLine]->state = busy;// no one is in the line going straight to him
		 	application_clerks[myLine]->bribe = false; //not bribing
		}
		AppClerkBribeLineLock[myLine]->Release();	
	}
	AppClerkLock[myLine]->Acquire();
	for(int i=0; i<70; i++){ //Yielding to give clerk time to wait on lock
			currentThread->Yield();
		}
	application_clerks[myLine]->ssn = arg;
	AppClerkCV[myLine]->Signal(AppClerkLock[myLine]); //wakeup the clerk
	cout<<currentThread->getName()<<" has given SSN ["<<arg<<"] to ApplicationClerk["<<myLine<<"]"<<endl;
	
	AppClerkCV[myLine]->Wait(AppClerkLock[myLine]); //no need to wait

	AppClerkLock[myLine]->Release();

	//if customer went to app clerk first
	if (customers[arg]->clerk_pick > 10){ 
		goToPicClerkLine(arg); //move onto pic clerk
	}
	//if customer went to pic clerk first
	else{
		goToPassClerkLine(arg); //move onto passport clerk
		}
}

//application clerk function- getting customer
void appGetCustomer(int arg){

	 while(true){
 		int myLine = arg;	
		if (application_clerks[myLine]->bribeLineCount>0){ //if someone on bribe line
			AppClerkBribeLineLock[myLine]->Acquire();
			cout<<currentThread->getName()<<" has signalled a Customer to come to their counter \n";
			AppClerkBribeLineCV[myLine]->Signal(AppClerkBribeLineLock[myLine]); //wake them up, call to register
			application_clerks[myLine]->state = busy; //make myself busy
			AppClerkBribeLineLock[myLine]->Release(); 
		}
 		else if(application_clerks[myLine]->lineCount>0){ // if someone on regular line  
 			AppClerkLineLock[myLine]->Acquire();
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			AppClerkLineCV[myLine]->Signal(AppClerkLineLock[myLine]); // wake up waiting customer
 			application_clerks[myLine]->state = busy; //state is busy now
 			AppClerkLineLock[myLine]->Release();
		}
		//if no one is in line
  		else{
 			application_clerks[myLine]->state = available; //app clerk is available
 		}	 
 	
 		AppClerkLock[myLine]->Acquire(); //acquire interaction lock
		AppClerkCV[myLine]->Wait(AppClerkLock[myLine]); //wait for customer to give SSN
		cout<<currentThread->getName()<<" has received SSN ["<<application_clerks[myLine]->ssn<<"] from Customer ["<<application_clerks[myLine]->ssn<<"]"<<endl;

	    bool flag = false;
	    for (unsigned int i=0; i< customer_data.size(); i++){
	    	if (customer_data[i]->SSN == application_clerks[myLine]->ssn){
	    		flag = true; 
	    	}
	    }
	    if (flag == false){
	    	//create and add customer data for this particular customer
			CustomerData *c_d = new CustomerData(application_clerks[myLine]->ssn);
			customer_data.push_back(c_d);
		}
		
		//loop each customer data
		for (unsigned int i=0; i<customer_data.size(); i++){
			//if customer's data is found
			if (customer_data[i]->SSN == application_clerks[myLine]->ssn){
				int r = rand() % 81 +20;
				for (int k=0; k<10; k++){
					currentThread->Yield();		
				}
				customer_data[i]->application = true;
				customers[customer_data[i]->SSN]->atAppClerk = true;
			}
		}
		AppClerkCV[myLine]->Signal(AppClerkLock[myLine]);
		cout<<currentThread->getName()<<" has recorded a completed application for Customer["<<application_clerks[myLine]->ssn<<"]"<<endl;
		if (application_clerks[arg]->bribe == true){ //if customer bribed me, add $500 
			cout<<currentThread->getName()<<" has received $500 from Customer["<<application_clerks[myLine]->ssn<<"]"<<endl;
			application_clerks[arg]->money = application_clerks[arg]->money + 500;
			application_clerks[arg]->bribe = false;
		}
		AppClerkCV[myLine]->Signal(AppClerkLock[myLine]); //signal the customer that record is completed
		AppClerkLock[myLine]->Release(); //release the lock

	}
}

void goToPassClerkLine(int arg){

	int myLine = -1; //initially -1 
	int lineSize = 1000; //max line 
	int bribeLineSize = 1000; //max line
	bool bribed = false; //initially false
	PickPassClerkLineLock.Acquire();
	if (customers[arg]->money>100){ // if customer has more than 100 dollars
		for (unsigned int i=0; i<passport_clerks.size(); i++){
			if(passport_clerks[i]->bribeLineCount<lineSize && passport_clerks[i]->state != onBreak){
				myLine = i; //my new clerk's line number
				bribeLineSize = passport_clerks[i]->bribeLineCount; //new bribelinesize
			}
		}
		bribed = true; //i can bribe
	}
	else{ //if i dont have enough money to bribe find the regular line
		for (unsigned int i=0; i<passport_clerks.size(); i++){
			if(passport_clerks[i]->lineCount<lineSize && passport_clerks[i]->state != onBreak){
				myLine = i;
				lineSize = passport_clerks[i]->lineCount;
			}
		}
		bribed = false; //cant bribe 
	}
	PickPassClerkLineLock.Release();
	if (bribed == false){//if won't bribe
		PassClerkLineLock[myLine]->Acquire();
 			if(passport_clerks[myLine]->state == busy){
				passport_clerks[myLine]->lineCount++;	//increment line count, waiting
				cout<<currentThread->getName()<<" has gotten in regular line for "<< passport_clerks[myLine]->name<<"\n";
				PassClerkLineCV[myLine]->Wait(PassClerkLineLock[myLine]); //"go" to wait queue
				passport_clerks[myLine]->lineCount--;	//clerk woke me up
		}
			else{
		 		passport_clerks[myLine]->state = busy; //else  going straight to register
		}
		passport_clerks[myLine]->bribe = false;
		PassClerkLineLock[myLine]->Release();
	}
	else{
		PassClerkBribeLineLock[myLine]->Acquire();
 			if(passport_clerks[myLine]->state == busy){
				passport_clerks[myLine]->bribeLineCount++;	//increment line count, waiting
				cout<<currentThread->getName()<<" has gotten in bribe line for "<< passport_clerks[myLine]->name<<"\n";
				PassClerkBribeLineCV[myLine]->Wait(PassClerkBribeLineLock[myLine]);//"go" to wait queue
				passport_clerks[myLine]->bribeLineCount--;	//clerk woke me up
				passport_clerks[myLine]->bribe = true;//bribed
				customers[arg]->money = customers[arg]->money - 500; //deduct my money
		}
			else{
		 		passport_clerks[myLine]->state = busy; //going straight to register
		 		passport_clerks[myLine]->bribe = false; //not bribing
		}
		PassClerkBribeLineLock[myLine]->Release();
	}

	// PassClerkLock[myLine]->Acquire();
// 	for(int i=0; i<50; i++){
// 			currentThread->Yield();
// 		}
	passport_clerks[myLine]->ssn = arg; // passing SSn to clerk
	cout<<currentThread->getName()<<" has given SSN ["<<arg<<"] to PassportClerk["<<myLine<<"]"<<endl;
	PassClerkCV[myLine]->Signal(PassClerkLock[myLine]); //signaling about passing the SSN
	PassClerkCV[myLine]->Wait(PassClerkLock[myLine]);//wait until clerk signals me back
	
	for(unsigned int k=0; k<customer_data.size(); k++){
		if(customer_data[k]->SSN == arg){ //if i'm in customer data base
			if(customer_data[k]->verified == false){ //if i'm not verified
				cout<<currentThread->getName()<<" has gone to PassportClerk ["<<myLine<<"] too soon. They are going to the back of the line"<<endl;
				int yield_random = rand() % 901 + 100;
				for(int i=0; i<yield_random; i++){ //yield 
					currentThread->Yield();
				}
				//go back to the end of the line
				PassClerkLock[myLine]->Release(); 
				customers[arg]->pass_punished++;
				goToPassClerkLine(arg);		// go to clerk again
				PassClerkLock[myLine]->Acquire();
				customers[arg]->pass_punished--;
			}
			break;
		}
	}
	
	PassClerkLock[myLine]->Release();
	
	if(customers[arg]->pass_punished == 0 && customers[arg]->atPassClerk == true){ //if verified and not punished 
		goToCashierLine(arg); //go to cashier now
	}
}

void passGetCustomer(int arg){
	while(true){
 		int myLine = arg; //my line number
 		if(passport_clerks[myLine]->bribeLineCount>0){ // if there is anyone on bribe line
 			PassClerkBribeLineLock[myLine]->Acquire();	
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			PassClerkBribeLineCV[myLine]->Signal(PassClerkBribeLineLock[myLine]); //signal customer to come
 			passport_clerks[myLine]->state = busy; //busy now
 			PassClerkBribeLineLock[myLine]->Release();
		}
 		else if(passport_clerks[myLine]->lineCount>0){// if there is anyone on line
 			PassClerkLineLock[myLine]->Acquire(); //acquire line lock
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			PassClerkLineCV[myLine]->Signal(PassClerkLineLock[myLine]); //signal customer to come to regsiter
 			passport_clerks[myLine]->state = busy; //busy now
 			PassClerkLineLock[myLine]->Release(); //release line lock
		}
  		else{
 			passport_clerks[myLine]->state = available; //no one in line, available
 		}	 
 	
 		PassClerkLock[myLine]->Acquire();
		//wait for social security number
		PassClerkCV[myLine]->Wait(PassClerkLock[myLine]); 
		cout<<currentThread->getName()<<" has received SSN ["<<passport_clerks[myLine]->ssn<<"] from Customer ["<<passport_clerks[myLine]->ssn<<"]"<<endl;
		//iterate through each customer data
		for(unsigned int k=0; k<customer_data.size(); k++){
			if(customer_data[k]->SSN == passport_clerks[myLine]->ssn) //find the customer with SSN
			{
				//if he has both app and pic
				if(customer_data[k]->application == true && customer_data[k]->picture == true){
		
					cout<<currentThread->getName()<<" had determined that Customer ["<<passport_clerks[myLine]->ssn<<"] has both their application and picture completed"<<endl;
					if (passport_clerks[arg]->bribe == true){ // if customer bribed
						cout<<currentThread->getName()<<" has received $500 from Customer["<<passport_clerks[myLine]->ssn<<"]"<<endl;
						passport_clerks[arg]->money = passport_clerks[arg]->money + 500; //add $500 to my wallet
						passport_clerks[arg]->bribe = false;//reset bribe 
					}
			
					//yield for delay
					int yield_random = rand() % 81 + 20;
					for(int g=0; g<yield_random; g++){ //yield until 
						currentThread->Yield();
					}
					customer_data[k]->verified = true; //verified
					customers[customer_data[k]->SSN]->atPassClerk = true; //at passport clerk state updated
					PassClerkCV[myLine]->Signal(PassClerkLock[myLine]);
					cout<<currentThread->getName()<<" has recorded Customer ["<<passport_clerks[myLine]->ssn<<"] passport documentation"<<endl;
				}
				//both app and pic are not verified or filed -> punished
				else{
				
					PassClerkCV[myLine]->Signal(PassClerkLock[myLine]); //signal customer
					cout<<currentThread->getName()<<" had determined that Customer ["<<passport_clerks[myLine]->ssn<<"] does not have both their application and picture completed"<<endl;
					if (passport_clerks[arg]->bribe == true){
						cout<<currentThread->getName()<<" has received $500 from Customer["<<passport_clerks[myLine]->ssn<<"]"<<endl;
						passport_clerks[arg]->money = passport_clerks[arg]->money + 500; //collect bribe money
						passport_clerks[arg]->bribe = false;
					}
				}	
				break;	
			}
		}		
		PassClerkLock[myLine]->Release(); //release interaction lock
	}
}



void goToCashierLine(int arg){
	int myLine = -1; //my line is initially set to -1
	int lineSize = 1000; //maximum number of customers in a line
	
	PickCashierLineLock.Acquire();
	for (unsigned int i=0; i<cashier_clerks.size(); i++){
		if(cashier_clerks[i]->lineCount<lineSize && cashier_clerks[i]->state != onBreak){
			myLine = i;
			lineSize = cashier_clerks[i]->lineCount;
		}
	}
	PickCashierLineLock.Release();
	
	CashierLineLock[myLine]->Acquire();
 	if(cashier_clerks[myLine]->state == busy){
			cashier_clerks[myLine]->lineCount++;	
			cout<<currentThread->getName()<<" has gotten in regular line for "<< cashier_clerks[myLine]->name<<"\n";
			CashierLineCV[myLine]->Wait(CashierLineLock[myLine]);
			cashier_clerks[myLine]->lineCount--;	
	}
	else{
		 	cashier_clerks[myLine]->state = busy;
	}
	CashierLineLock[myLine]->Release();
	
	CashierLock[myLine]->Acquire();
	// for(int i=0; i<50; i++){
// 			currentThread->Yield();
// 		}
	cashier_clerks[myLine]->ssn = arg;
	cout<<currentThread->getName()<<" has given SSN ["<<arg<<"] to Cashier["<<myLine<<"]"<<endl;
	
	CashierCV[myLine]->Signal(CashierLock[myLine]);
	CashierCV[myLine]->Wait(CashierLock[myLine]); 
	
	customers[arg]->money -=100; //pays 100 dollars
	cout<<currentThread->getName()<<" has given Cashier["<<myLine<<"] $100"<<endl;
	//customers[arg]->paidCashier = true; //mark that the customer paid cashier
	
	CashierCV[myLine]->Signal(CashierLock[myLine]);
	CashierCV[myLine]->Wait(CashierLock[myLine]); 
	
	for(unsigned int k=0; k<customer_data.size(); k++){
		if(customer_data[k]->SSN == arg){
			if(customer_data[k]->got_passport == false){
				cout<<currentThread->getName()<<" has gone to Cashier["<<myLine<<"] too soon. They are going to the back of the line"<<endl;
				int yield_random = rand() % 901 + 100;
				for(int i=0; i<yield_random; i++){
					currentThread->Yield();
				}
				//go back to the end of the line
				CashierLock[myLine]->Release();
				customers[arg]->cash_punished++;
				goToCashierLine(arg);	
				CashierLock[myLine]->Acquire();
				customers[arg]->cash_punished--;
			}
			break;
		}
	}
	if(customers[arg]->cash_punished == 0 && customers[arg]->atCashier == true){
		cout<<"Customer [" <<arg<<"] is leaving Passport Office"<<endl;
		CashierLock[myLine]->Release();
	}
}

//cashier function
void cashGetCustomer(int arg){
	while(true){
 		int myLine = arg;
		CashierLineLock[myLine]->Acquire();	 //acquire cashier lock
		
		//if there is more than one person in line
 		if(cashier_clerks[myLine]->lineCount>0){
 			cout<< currentThread->getName()<<" has signalled a Customer to come to their counter \n";
 			CashierLineCV[myLine]->Signal(CashierLineLock[myLine]); //signal customer to come
 			cashier_clerks[myLine]->state = busy; //cashier is busy
		}
		//if no one is in line
  		else{
			//implement onBreak
 		}	 
 	
 		CashierLock[myLine]->Acquire(); //acquire interaction lock
		CashierLineLock[myLine]->Release(); //release line lock
		
		CashierCV[myLine]->Wait(CashierLock[myLine]); //wait for customer to send in social security number
		cout<<currentThread->getName()<<" has received SSN ["<<cashier_clerks[myLine]->ssn<<"] from Customer ["<<cashier_clerks[myLine]->ssn<<"]"<<endl;
		
		//iterate through each customer data
		for(unsigned int k=0; k<customer_data.size(); k++){
			//when we find the customer in data
			if(customer_data[k]->SSN == cashier_clerks[myLine]->ssn)
			{
				//if the passport clerk verified the application and picture
				if(customer_data[k]->verified == true){ 
	
					cout<<currentThread->getName()<<" has verified that Customer ["<<cashier_clerks[myLine]->ssn<<"] has been certified by a PassportClerk"<<endl;
					
					CashierCV[myLine]->Signal(CashierLock[myLine]); //verified- let customer know
					CashierCV[myLine]->Wait(CashierLock[myLine]);
			
					cashier_clerks[myLine]->money += 100; //receives money from the customer
					cout<<currentThread->getName()<<" has received the $100 from Customer ["<<cashier_clerks[myLine]->ssn<<"] after certification"<<endl;

					//randomize yield for delay
					int yield_random = rand() % 81 + 20; 
					for(int g=0; g<yield_random; g++){
						currentThread->Yield();
					}
					customer_data[k]->got_passport = true; //customer got passport
					cout<<currentThread->getName()<<" has provided Customer ["<<cashier_clerks[myLine]->ssn<<"] their completed passport"<<endl;

					customers[customer_data[k]->SSN]->atCashier = true; //customer came to cashier- status update
					CashierCV[myLine]->Signal(CashierLock[myLine]); //signal the customer
					cout<<currentThread->getName()<<" has recorded Customer ["<<cashier_clerks[myLine]->ssn<<"] passport documentation"<<endl;
				}
				//if the passport clerk did not verify yet
				else{
					CashierCV[myLine]->Signal(CashierLock[myLine]); //signal the customer
					CashierCV[myLine]->Wait(CashierLock[myLine]);
			
					cashier_clerks[myLine]->money += 100; //receives money from the customer
					cout<<currentThread->getName()<<" has received the $100 from Customer ["<<cashier_clerks[myLine]->ssn<<"] before certification. They are to go to the back of my line."<<endl;
					
					CashierCV[myLine]->Signal(CashierLock[myLine]); //signal the customer
					CashierCV[myLine]->Wait(CashierLock[myLine]);

				}	
				break;	
			}
		}		
		CashierLock[myLine]->Release(); //release interaction lock
	}
}

//manager function ***not fully implemented
void managerJob(int arg){
	int yield_time = 0;
	int keep_going = 0;
	bool run_job = true;
	while(run_job){
		keep_going = 1;
		if(customer_data.size() > 0){
			keep_going = 0;
		}
		//iterate through each customer data
		for (unsigned int i=0; i<customer_data.size(); i++){
			//if customer did not leave office
			if(customers[customer_data[i]->SSN]->leftOffice == false){
				keep_going++; //increment counter
			}
		}
		//randomize yield time 
		yield_time = rand() % 101 + 50;
		for(int g=0; g<yield_time; g++){
			currentThread->Yield();
		}
		manager_clerk.cashierMoney = 0; //initialize/reset to 0
		//iterate through each cashier 
		for (unsigned int i=0; i<cashier_clerks.size(); i++){
			manager_clerk.cashierMoney += cashier_clerks[i]->money; //add up cashier money
		}
		cout<<"Manager has counted a total of $"<<manager_clerk.cashierMoney<<" for Cashiers"<<endl;
		
		manager_clerk.passClerkMoney = 0; //initialize/reset to 0
		//iterate through each passport clerk
		for (unsigned int j=0; j<passport_clerks.size(); j++){
			manager_clerk.passClerkMoney += passport_clerks[j]->money; //add up passport clerk money
		}
		cout<<"Manager has counted a total of $"<<manager_clerk.passClerkMoney<<" for PassportClerks"<<endl;
		
		manager_clerk.picClerkMoney = 0; //initialize/reset to 0
		//iterate through each picture clerk
		for (unsigned int k=0; k<picture_clerks.size(); k++){
			manager_clerk.picClerkMoney += picture_clerks[k]->money; //add up picture clerk money
		}
		cout<<"Manager has counted a total of $"<<manager_clerk.picClerkMoney<<" for PictureClerks"<<endl;
		
		manager_clerk.appClerkMoney = 0; //initialize/reset to 0
		//iterate through each picture clerk
		for (unsigned int l=0; l<application_clerks.size(); l++){
			manager_clerk.appClerkMoney += application_clerks[l]->money; //add up application clerk money
		}
		cout<<"Manager has counted a total of $"<<manager_clerk.appClerkMoney<<" for ApplicationClerks"<<endl;

		manager_clerk.totalMoney = 0; //initialize/reset to 0
		manager_clerk.totalMoney = manager_clerk.appClerkMoney + manager_clerk.cashierMoney + manager_clerk.picClerkMoney + manager_clerk.passClerkMoney; 
		
		cout<<"Manager has counted a total of $"<<manager_clerk.totalMoney<<" for the passport office"<<endl;

		if(keep_going == 0){
			run_job = false;
		}
	}
}


//senator function- not implemented
void senatorGoToAppClerkLine(int arg){}

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
char *cashier_name;
char *senator_name;
char *manager_name;

void Problem2(int c, int a, int p, int pass, int cash){

	Thread *thread; //threads
	Customer *customer; //customers
	
	//application clerk locks and CVs
	ApplicationClerk *app_clerk;
	Lock *appClerkLineLock;
    Condition *appClerkLineCV;
    Lock *appClerkBribeLineLock;
    Condition *appClerkBribeLineCV;
    Lock *appClerkLock;
    Condition *appClerkCV;

	//application clerk
	for(int i=0; i<a; i++){
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
		
		thread = new Thread(app_name); //create new thread
		thread->Fork((VoidFunctionPtr)appGetCustomer,i); //fork the thread
		
		name="";
	}
	
	//picture clerk locks and CVs
	PictureClerk *pic_clerk;
	Lock *picClerkLineLock;
    Condition *picClerkLineCV;
    Lock *picClerkBribeLineLock;
    Condition *picClerkBribeLineCV;
    Lock *picClerkLock;
    Condition *picClerkCV;
    
    for (int i=0; i<p; i++){
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
		
		thread = new Thread(pic_name); //create a new thread
		thread->Fork((VoidFunctionPtr)picGetCustomer,i); //fork each thread
		
		name="";
    }
    
    //passport clerk locks and CVs
    PassportClerk *pass_clerk;
    Lock *passClerkLineLock;
    Condition *passClerkLineCV;
    Lock *passClerkBribeLineLock;
    Condition *passClerkBribeLineCV;
    Lock *passClerkLock;
    Condition *passClerkCV;
    
    for(int j=0; j<pass; j++){
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
		
		thread = new Thread(pass_name); //create new thread
		thread->Fork((VoidFunctionPtr)passGetCustomer,j); //fork each thread
		
		name="";
	}
	
	//cashier locks and CVs
	Cashier *cash_clerk;
	Lock *cashierLineLock;
    Condition *cashierLineCV;
    Lock *cashierLock;
    Condition *cashierCV;
	
	for(int j=0; j<cash; j++){
		name = new char [20];
		cashier_name = new char [20];
		sprintf(cashier_name, "Cashier[%d]",j);
		
		cash_clerk = new Cashier(cashier_name);
		cashier_clerks.push_back(cash_clerk);
		
		sprintf(name,"CashierLineLock[%d]",j);
		
		cashierLineLock = new Lock(name);
		CashierLineLock.push_back(cashierLineLock);
		
		sprintf(name,"CashierLineCV[%d]",j);
		
		cashierLineCV = new Condition(name);
		CashierLineCV.push_back(cashierLineCV);
		
		sprintf(name,"CashierLock[%d]",j);
		
		cashierLock = new Lock(name);
		CashierLock.push_back(cashierLock);
		
		sprintf(name,"CashierCV[%d]",j);
		
		cashierCV = new Condition(name);
		CashierCV.push_back(cashierCV);
		
		thread = new Thread(cashier_name); //create new thread
		thread->Fork((VoidFunctionPtr)cashGetCustomer,j); //fork each thread
		
		name="";
	}
	
	//****un-comment the following manager lines if want to test counting money- but it may run forever since 
	//****current state of the program does not allow all customers to complete their processes
	//****thus, the manager keeps running on while loop because the program does not end
	
	/*manager_name = new char [20];
	sprintf(manager_name, "Manager[%d]", 0);
	
	thread = new Thread(manager_name); //create a new thread of manager
	thread->Fork((VoidFunctionPtr)managerJob, 0); //fork the thread*/
	
	//customer
	for(int i=0; i<c; i++){
		name = new char [20];
		sprintf(name,"Customer[%d]",i);	
		customer = new Customer(name, i);
		customer->senator = false;
		customers.push_back(customer);
		thread = new Thread(name);
		//fork thread-> choose whether to go to app clerk or pic clerk first
		if (customer->clerk_pick >10){
			thread->Fork((VoidFunctionPtr)goToAppClerkLine,i);
		}
		else {
			thread->Fork((VoidFunctionPtr)goToPicClerkLine,i);
		}
	 }
	 
	 
	//  for(int i=7; i<8; i++){
// 		name = new char [20];
// 		sprintf(name,"Senator[%d]",i);
// 		
// 		customer = new Customer(name, i);
// 		customer->senator = true;
// 		customer->money = 100;
// 		customers.push_back(customer);
// 		
// 		thread = new Thread(name);
// 		if (customer->clerk_pick >10){
// 			thread->Fork((VoidFunctionPtr)goToAppClerkLine,i);
// 			cout<<thread->getName()<<" goes to app clerk first"<<endl;
// 		}
// 		else {
// 			thread->Fork((VoidFunctionPtr)goToPicClerkLine,i);
// 			cout<<thread->getName()<<" goes to pic clerk first"<<endl;
// 		}
// 	 }
		
}

