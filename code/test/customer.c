#include "syscall.h"
#include "setup.h"


int myLine = -1;
int lineSize = 1000;
int bribeLineSize = 1000;
bool bribed;
int a;
int myIndex;

void goToAppClerk(int arg){
	bribed = false;
	Acquire(PickAppClerkLineLock);
	if (customers[arg].money > 100){
		for (a = 0; a<2; a++){
				if(application_clerks[a].bribeLineCount<bribeLineSize && application_clerks[a].state != onBreak){
				/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
					
					myLine = a; /*this is my new line*/
					PrintNum(myLine);
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
				PrintNum(myLine);
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
			Write("--Got in regular line \n", sizeof("--Got in regular line \n"), ConsoleOutput);
			Wait(AppClerkLineLock[myLine], AppClerkLineCV[myLine]); /*wait on clerks waiting queue until he signal customer*/
			application_clerks[myLine].lineCount--;	/*decrement line count of the clerk, means he signalled customer*/
			
		}
		else{
			Write("--Application clerk is not busy \n", sizeof("--Application clerk is not busy \n"), ConsoleOutput);
		 	application_clerks[myLine].state = busy; /*else he is busy going right to register*/
		}
		application_clerks[myLine].bribe = false; /* not bribing*/
		Release(AppClerkLineLock[myLine]);
	}
	else{ /*customer bribed*/
		Acquire(AppClerkBribeLineLock[myLine]);
		if(application_clerks[myLine].state == busy){ /* if clerk is not busy */
			application_clerks[myLine].bribeLineCount++;	/*customer gets in line- increment line number*/
			PrintNum(application_clerks[myLine].bribeLineCount);	
			Write("--Got in bribe line, busy \n", sizeof("--Got in bribe line, busy \n"), ConsoleOutput);
			Wait(AppClerkBribeLineLock[myLine],AppClerkBribeLineCV[myLine]);/*wait on clerks waiting queue until he signal customer*/
			application_clerks[myLine].bribeLineCount--;	/* decrement line count of the clerk, means he signalled customer*/
			application_clerks[myLine].bribe = true; /*customer is bribing*/
			customers[arg].money = customers[arg].money - 500; /* reducing customer's money*/
		}
		else{
		 	application_clerks[myLine].state = busy;/* no one is in the line going straight to him*/
		 	Write("--Application clerk is not busy \n", sizeof("--Application clerk is not busy \n"), ConsoleOutput);
		 	application_clerks[myLine].bribe = false; /*not bribing*/
		}
		Release(AppClerkBribeLineLock[myLine]);	
	} 

    Acquire(AppClerkLock[myLine]);
	
	application_clerks[myLine].ssn = arg; 
	
	Signal(AppClerkLock[myLine], AppClerkCV[myLine]); /*wakeup the clerk*/
	
		
	/*Wait(AppClerkLock[myLine], AppClerkCV[myLine]); /*no need to wait*/

	/*Release(AppClerkLock[myLine]);*/
	
}

int main(){
	
	
	
	myIndex = GetMV(customer_index);
	SetMV(customer_index, myIndex+1);
	goToAppClerk(myIndex);
	
	

}