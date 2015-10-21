#include "syscall.h"


typedef enum { false, true } bool;
typedef enum {busy, available, onBreak} ClerkState;

void goToAppClerkLine(int);
void goToPicClerkLine(int);
void goToPassClerkLine(int);
void goToCashierLine(int);


void appGetCustomer(int);
void passGetCustomer(int);
void picGetCustomer(int);
void cashGetCustomer(int);


struct CustomerData{
	char name[20];
	int SSN; /* social security number */
	bool application;/* whether application clerk filed the application */
	bool picture; /*whether picture clerk took a picture and filed it */
	bool verified; /*whether passport clerk verified and filed it */
	bool got_passport; /*whether customer received his/her passport from cashier */
	
};

/*customer struct*/
struct Customer{
	char *name;
	int money; 
	int social_security;
	bool application; /*application completed?*/
	bool atAppClerk; /*whether customer went to app clerk*/
	bool atPicClerk; /*whether customer went to pic clerk*/
	bool atPassClerk; /*whether customer went to passport clerk*/
	bool atCashier; /*whether customer went to cashier*/
	bool leftOffice;
	int liked; /*keep track of pic-recursive*/
	int pass_punished; /*keep track of punishment by pass clerk- recursive*/
	int cash_punished; /*keep track of punishment by cashier- recursive*/
	int pic_liking; /*probability of liking the picture*/
	int clerk_pick; /*which clerk to go first- app or pic clerk?*/
	bool senator; /*whether the customer is a senator*/
};

/*application clerk struct*/
struct ApplicationClerk{
	char *name;	
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	ClerkState state;
	bool bribe; /*bribed or not*/
};

/*picture clerk struct*/
struct PictureClerk{
	char *name;
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	bool pic;
	ClerkState state;
	bool bribe; /*bribed or not*/
};

/*passport clerk struct*/
struct PassportClerk{
	char *name;
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	bool bribe; /*bribed or not*/
	ClerkState state;
};

/*cashier struct- we said that cashier cannot be bribed*/
struct Cashier{
	char *name;
	int lineCount;
	int money;
	int ssn;
	ClerkState state;
};

/*manager struct*/
struct Manager{
	char *name;
	int appClerkMoney;
	int picClerkMoney;
	int passClerkMoney;
	int cashierMoney;
	int totalMoney;
};
struct CustomerData customer_data[10];
struct Customer customers[10];
struct ApplicationClerk application_clerks[10];
struct PictureClerk picture_clerks[10];
struct PassportClerk passport_clerks[10];
struct Cashier cashiers[10];

int AppClerkLock[10]; /*lock used to interact- at the register*/
int AppClerkCV[10]; /*cv used to interact- at the register*/
int AppClerkLineLock[10]; /*lock used for line*/
int AppClerkLineCV[10]; /*cv used for line*/
int PickAppClerkLineLock;  
int AppClerkBribeLineCV[10]; /*cv used for bribe line*/
int AppClerkBribeLineLock[10];
int i;

int PicClerkLock[10]; /*lock used to interact- at the register*/
int PicClerkCV[10];
int PicClerkLineLock[10];
int PicClerkLineCV[10];
int PickPicClerkLineLock;
int PicClerkBribeLineCV[10];
int PicClerkBribeLineLock[10];

int PassClerkLock[10]; /*lock used to interact- at the register*/
int PassClerkCV[10]; /*cv used to interact- at the register*/
int PassClerkLineLock[10]; /*lock used for line*/
int PassClerkLineCV[10]; /*cv used for line*/
int PickPassClerkLineLock;
int PassClerkBribeLineCV[10]; /*cv used for bribe line*/
int PassClerkBribeLineLock[10];

int CashierLock[10]; /*lock used to interact- at the register*/
int CashierCV[10]; /*cv used to interact- at the register*/
int CashierLineLock[10]; /*lock used for line*/
int CashierLineCV[10]; /*cv used for line*/
int PickCashierLineLock; 
int a;

void goToAppClerkLine(int arg){
	int myLine = -1;/*my line is initially set to -1*/
	int lineSize = 1000;/*maximum number of customers in a line*/
	int bribeLineSize = 1000; /*maximum number of customers in a line*/
	bool bribed = false; /* bribed initially false*/
	Acquire(PickAppClerkLineLock);	/*acquire pick app clerk line lock*/
	if (customers[arg].money > 100){
		for (a = 0; a<2; a++){
				if(application_clerks[a].bribeLineCount<bribeLineSize && application_clerks[a].state != onBreak){
				/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
					myLine = a; /*this is my new line*/
					bribeLineSize = application_clerks[a].bribeLineCount;/* bribe line size is set to this line's size*/
					}
		}
		bribed = true; 
	}
	else{
		for (a = 0; a<2; a++){
			if(application_clerks[a].lineCount<lineSize && application_clerks[a].state != onBreak){
			/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
				myLine = a; /*this is my new line*/
				lineSize = application_clerks[a].lineCount; /*line size is set to this line's size*/
			}
		}
		bribed = false; /*can't bribe*/
	}
	Release(PickAppClerkLineLock); /*release line picking lock */
	
	if (bribed == false){ /*if customer didn'`t bribe */
		Acquire(AppClerkLineLock[myLine]);
		if(application_clerks[myLine].state == busy){ /* if clerk is not busy*/
			application_clerks[myLine].lineCount++; /*customer gets in line- increment line number	*/
			Wait(AppClerkLineLock[myLine], AppClerkLineCV[myLine]); /*wait on clerks waiting queue until he signal customer*/
			application_clerks[myLine].lineCount--;	/*decrement line count of the clerk, means he signalled customer*/
			
		}
		else{
		 	application_clerks[myLine].state = busy; /*else he is busy going right to register*/
		}
		application_clerks[myLine].bribe = false; /* not bribing*/
		Release(AppClerkLineLock[myLine]);
	}
	else{ /*customer bribed*/
		Acquire(AppClerkBribeLineLock[myLine]);
		if(application_clerks[myLine].state == busy){ /* if clerk is not busy */
			application_clerks[myLine].bribeLineCount++;	/*customer gets in line- increment line number*/	
		
			Wait(AppClerkBribeLineLock[myLine],AppClerkBribeLineCV[myLine]);/*wait on clerks waiting queue until he signal customer*/
			application_clerks[myLine].bribeLineCount--;	/* decrement line count of the clerk, means he signalled customer*/
			application_clerks[myLine].bribe = true; /*customer is bribing*/
			customers[arg].money = customers[arg].money - 500; /* reducing customer's money*/
		}
		else{
		 	application_clerks[myLine].state = busy;/* no one is in the line going straight to him*/
		 	application_clerks[myLine].bribe = false; /*not bribing*/
		}
		Release(AppClerkBribeLineLock[myLine]);	
	}

    Acquire(AppClerkLock[myLine]);
	for(a=0; i<70; i++){ /*Yielding to give clerk time to wait on lock*/
			Yield();
		}
	application_clerks[myLine].ssn = arg;
	Signal(AppClerkLock[myLine], AppClerkCV[myLine]); /*wakeup the clerk*/
		
	Wait(AppClerkLock[myLine], AppClerkCV[myLine]); /*no need to wait*/

	Release(AppClerkLock[myLine]);

	/*if customer went to app clerk first*/
	if (customers[arg].clerk_pick > 10){ 
		/*goToPicClerkLine(arg); */ 
	}
	
	else{
		/*goToPassClerkLine(arg); */ 
		}
}

void appGetCustomer(int arg){
 bool flag = false;
	 while(true){
 		int myLine = arg;	
		if (application_clerks[myLine].bribeLineCount>0){ /*if someone on bribe line */
			Acquire(AppClerkBribeLineLock[myLine]);
			Signal(AppClerkBribeLineLock[myLine], AppClerkBribeLineCV[myLine]); /*wake them up, call to register*/
			application_clerks[myLine].state = busy; /*make myself busy */
			Release(AppClerkBribeLineLock[myLine]); 
		}
 		else if(application_clerks[myLine].lineCount>0){ /* if someone on regular line  */
 			Acquire(AppClerkLineLock[myLine]);
 			Signal(AppClerkLineLock[myLine], AppClerkLineCV[myLine]); /* wake up waiting customer*/
 			application_clerks[myLine].state = busy; /*state is busy now*/
 			Release(AppClerkLineLock[myLine]);
		}
	
  		else{
 			application_clerks[myLine].state = available; /*app clerk is available*/
 		}	 
 	
 		Acquire(AppClerkLock[myLine]); /*acquire interaction lock*/
		Wait(AppClerkLock[myLine], AppClerkCV[myLine]); /*wait for customer to give SSN*/

	   
	    for (a=0; a< 0; a++){
	    	if (customer_data[a].SSN == application_clerks[myLine].ssn){
	    		flag = true; 
	    	}
	    }
	    if (flag == false){
	    	/*create and add customer data for this particular customer*/
	
			customer_data[0].SSN = application_clerks[myLine].ssn;
		}
		
		/*loop each customer data*/
		for (a=0; a<10; a++){
			/*if customer's data is found*/
			if (customer_data[a].SSN == application_clerks[myLine].ssn){
				
				for (a=0; a<10; a++){
					Yield();		
				}
				customer_data[i].application = true;
				customers[customer_data[i].SSN].atAppClerk = true;
			}
		}
		Signal(AppClerkLock[myLine], AppClerkCV[myLine]);
		if (application_clerks[arg].bribe == true){ 
			application_clerks[arg].money = application_clerks[arg].money + 500;
			application_clerks[arg].bribe = false;
		}
		Signal(AppClerkLock[myLine], AppClerkCV[myLine]); /*signal the customer that record is completed*/
		Release(AppClerkLock[myLine]); /*release the lock*/

	}
}

void goToPassClerkLine(int arg){

	int myLine = -1; /*initially -1 */ 
	int lineSize = 1000; /*max line */
	int bribeLineSize = 1000; /*max line */
	bool bribed = false; /*initially false*/
	Acquire(PickPassClerkLineLock);
	if (customers[arg].money>100){ /* if customer has more than 100 dollars*/
		for (a=0; a<10; a++){
			if(passport_clerks[i].bribeLineCount<lineSize && passport_clerks[i].state != onBreak){
				myLine = i; /*my new clerk's line number*/
				bribeLineSize = passport_clerks[i].bribeLineCount; /*new bribelinesize*/
			}
		}
		bribed = true; /*i can bribe*/
	}
	else{ /*if i dont have enough money to bribe find the regular line*/
		for (a=0; a<10; a++){
			if(passport_clerks[a].lineCount<lineSize && passport_clerks[a].state != onBreak){
				myLine = i;
				lineSize = passport_clerks[a].lineCount;
			}
		}
		bribed = false; /*cant bribe */
	}
	Release(PickPassClerkLineLock);
	if (bribed == false){/*if won't bribe*/
		Acquire(PassClerkLineLock[myLine]);
 			if(passport_clerks[myLine].state == busy){
				passport_clerks[myLine].lineCount++;	/*increment line count, waiting*/
				Wait(PassClerkLineLock[myLine], PassClerkLineCV[myLine]); /*"go" to wait queue*/
				passport_clerks[myLine].lineCount--;	/*clerk woke me up*/
		}
			else{
		 		passport_clerks[myLine].state = busy; /*else  going straight to register*/
		}
		passport_clerks[myLine].bribe = false;
		Release(PassClerkLineLock[myLine]);
	}
	else{
		Acquire(PassClerkBribeLineLock[myLine]);
 			if(passport_clerks[myLine].state == busy){
				passport_clerks[myLine].bribeLineCount++;	/*increment line count, waiting*/
				Wait(PassClerkBribeLineLock[myLine], PassClerkBribeLineCV[myLine]);/*"go" to wait queue*/
				passport_clerks[myLine].bribeLineCount--;	/*clerk woke me up*/
				passport_clerks[myLine].bribe = true;/*bribed*/
				customers[arg].money = customers[arg].money - 500; /*deduct my money*/
		}
			else{
		 		passport_clerks[myLine].state = busy; /*going straight to register*/
		 		passport_clerks[myLine].bribe = false; /*not bribing*/
		}
		Release(PassClerkBribeLineLock[myLine]);
	}


	passport_clerks[myLine].ssn = arg; /* passing SSn to clerk*/
	Signal(PassClerkLock[myLine], PassClerkCV[myLine]); 
	Wait(PassClerkLock[myLine], PassClerkCV[myLine]);
	
	for(a=0; a<10; a++){
		if(customer_data[a].SSN == arg){
			if(customer_data[a].verified == false){ 
				
				for(a=0; a<100; a++){  
					Yield();
				}
				
				Release(PassClerkLock[myLine]); 
				customers[arg].pass_punished++;
				goToPassClerkLine(arg);		
				Acquire(PassClerkLock[myLine]);
				customers[arg].pass_punished--;
			}
			break;
		}
	}
	
	Release(PassClerkLock[myLine]);
	
	if(customers[arg].pass_punished == 0 && customers[arg].atPassClerk == true){ /*if verified and not punished */
		
	}
}





int main(){
PickAppClerkLineLock = CreateLock("PickAppClerkLineLock", sizeof("PickAppClerkLineLock"));
PickPicClerkLineLock = CreateLock("PickPicClerkLineLock", sizeof("PickPicClerkLineLock"));
PickPassClerkLineLock = CreateLock("PickPassClerkLineLock", sizeof("PickPassClerkLineLock")); 
PickCashierLineLock = CreateLock("PickCashierLineLock", sizeof("PickCashierLineLock")); 


for(i=0; i<2; i++){ 
  application_clerks[i].name = "AppClerk";
  application_clerks[i].lineCount = 0;
  application_clerks[i].bribeLineCount = 0;
  application_clerks[i].state = available;
  AppClerkLock[i] = CreateLock("AppClerkLock",sizeof("AppClerkLock"));
  AppClerkCV[i] = CreateCondition("AppClerkCV",sizeof("AppClerkCV"));
  AppClerkLineLock[i] = CreateLock("AppClerkLineLock", sizeof("AppClerkLineLock"));
  AppClerkLineCV[i] = CreateCondition("AppClerkLineCV", sizeof("AppClerkLineCV"));
  AppClerkBribeLineLock[i] = CreateLock("AppClerkBribeLineLock", sizeof("AppClerkBribeLineLock"));
  AppClerkBribeLineCV[i] = CreateCondition("AppClerkBribeLineCV", sizeof("AppClerkBribeLineCV"));
}

for(i=0; i<2; i++){ 
  picture_clerks[i].name = "PicClerk";
  picture_clerks[i].lineCount = 0;
  picture_clerks[i].bribeLineCount = 0;
  picture_clerks[i].state = available;
  PicClerkLock[i] = CreateLock("PicClerkLock",sizeof("PicClerkLock"));
  PicClerkCV[i] = CreateCondition("PicClerkCV",sizeof("PicClerkCV"));
  PicClerkLineLock[i] = CreateLock("PicClerkLineLock", sizeof("PicClerkLineLock"));
  PicClerkLineCV[i] = CreateCondition("PicClerkLineCV", sizeof("PicClerkLineCV"));
  PicClerkBribeLineLock[i] = CreateLock("PicClerkBribeLineLock", sizeof("PicClerkBribeLineLock"));
  PicClerkBribeLineCV[i] = CreateCondition("PicClerkBribeLineCV", sizeof("PicClerkBribeLineCV"));
}
for(i=0; i<2; i++){ 
  passport_clerks[i].name = "PassClerk";
  passport_clerks[i].lineCount = 0;
  passport_clerks[i].bribeLineCount = 0;
  passport_clerks[i].state = available;
  PassClerkLock[i] = CreateLock("PassClerkLock",sizeof("PassClerkLock"));
  PassClerkCV[i] = CreateCondition("PassClerkCV",sizeof("PassClerkCV"));
  PassClerkLineLock[i] = CreateLock("PassClerkLineLock", sizeof("PassClerkLineLock"));
  PassClerkLineCV[i] = CreateCondition("PassClerkLineCV", sizeof("PassClerkLineCV"));
  PassClerkBribeLineLock[i] = CreateLock("PassClerkBribeLineLock", sizeof("PassClerkBribeLineLock"));
  PassClerkBribeLineCV[i] = CreateCondition("PassClerkBribeLineCV", sizeof("PassClerkBribeLineCV"));
}
for(i=0; i<2; i++){ 
  passport_clerks[i].name = "PassClerk";
  passport_clerks[i].lineCount = 0;
  passport_clerks[i].bribeLineCount = 0;
  passport_clerks[i].state = available;
  PassClerkLock[i] = CreateLock("PassClerkLock",sizeof("PassClerkLock"));
  PassClerkCV[i] = CreateCondition("PassClerkCV",sizeof("PassClerkCV"));
  PassClerkLineLock[i] = CreateLock("PassClerkLineLock", sizeof("PassClerkLineLock"));
  PassClerkLineCV[i] = CreateCondition("PassClerkLineCV", sizeof("PassClerkLineCV"));
  PassClerkBribeLineLock[i] = CreateLock("PassClerkBribeLineLock", sizeof("PassClerkBribeLineLock"));
  PassClerkBribeLineCV[i] = CreateCondition("PassClerkBribeLineCV", sizeof("PassClerkBribeLineCV"));
}

for(i=0; i<2; i++){ 
  cashiers[i].name = "Cashier";
  cashiers[i].lineCount = 0;
  cashiers[i].state = available;
  CashierLock[i] = CreateLock("CashierLock",sizeof("CashierLock"));
  CashierCV[i] = CreateCondition("CashierCV",sizeof("CashierCV"));
  CashierLineLock[i] = CreateLock("CashierLineLock", sizeof("CashierLineLock"));
  CashierLineCV[i] = CreateCondition("CashierLineCV", sizeof("CashierLineCV"));
}


for (i=0; i<5; i++){
	customers[i].name = "customer";
	customers[i].money = 500;
	customers[i].social_security = i;	
}


}

