#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;	
			PrintNum(myLine);
			Write("That was me; app clerk\n", sizeof("That was me; app clerk\n"), ConsoleOutput);
			
			if(GetMV(application_clerks[myLine].bribeLineCount)>0){
			/*if (application_clerks[myLine].bribeLineCount>0){ /*if someone on bribe line */
				Write("   Someone is in Bribe line\n", sizeof("   Someone is in Bribe line\n"), ConsoleOutput);
				Acquire(AppClerkBribeLineLock[myLine]);
				Signal(AppClerkBribeLineLock[myLine], AppClerkBribeLineCV[myLine]); /*wake them up, call to register*/
				SetMV(application_clerks[myLine].state, 0);
				application_clerks[myLine].money = application_clerks[myLine].money + 500;
				SetMV(manager.appClerkMoney, GetMV(manager.appClerkMoney)+500);
				Write("-AppClerk-Received bribe from customer\n", sizeof("-AppClerk-Received bribe from customer\n"), ConsoleOutput);
				/*application_clerks[myLine].state = busy; /*make myself busy */
				Release(AppClerkBribeLineLock[myLine]); 
			}
			else if(GetMV(application_clerks[myLine].lineCount)>0){
			/*else if(application_clerks[myLine].lineCount>0){ /* if someone on regular line  */
				Write("   Someone is in Regular Line, no one in Bribe line\n", sizeof("   Someone is in Regular Line, no one in Bribe line\n"), ConsoleOutput);
				Acquire(AppClerkLineLock[myLine]);
				Signal(AppClerkLineLock[myLine], AppClerkLineCV[myLine]); /* wake up waiting customer*/
				SetMV(application_clerks[myLine].state, 0);
				/*application_clerks[myLine].state = busy; /*state is busy now*/
				Release(AppClerkLineLock[myLine]);
			}
	
			else{
				SetMV(application_clerks[myLine].state, 1);
				/*application_clerks[myLine].state = available; /*app clerk is available*/
			}	 
			
			
			/*if(GetMV(application_clerks[myLine].state) != 1){*/
				Acquire(AppClerkLock[myLine]); /*acquire interaction lock*/
				Write("-AppClerk-Acquired Lock\n", sizeof("-AppClerk-Acquired Lock\n"), ConsoleOutput);
				Wait(AppClerkLock[myLine], AppClerkCV[myLine]); /*wait for customer to give SSN*/
	
				SetMV(customer_data[customer_counter].SSN, application_clerks[myLine].ssn);
				/*          Should I check if it is the right SSN?         */
				Write("--Got ssn and app from customer \n", sizeof("--Got ssn and app from customer \n"), ConsoleOutput);
				SetMV(customer_data[customer_counter].application, 1);
			
				/*customers[customer_data[i].SSN].atAppClerk = true; */
				
				Signal(AppClerkLock[myLine], AppClerkCV[myLine]);
				/*if (application_clerks[myLine].bribe == true){ 
					application_clerks[myLine].money = application_clerks[myLine].money + 500;
					Write("-AppClerk-Received bribe from customer\n", sizeof("-AppClerk-Received bribe from customer\n"), ConsoleOutput);
					application_clerks[myLine].bribe = false;
				}*/
				/*Wait(AppClerkLock[myLine], AppClerkCV[myLine]);
				Signal(AppClerkLock[myLine], AppClerkCV[myLine]); /*signal the customer that record is completed*/
				SetMV(application_clerks[myLine].state, 1);
				Release(AppClerkLock[myLine]); /*release the lock*/
				/*break;*/
			/*}*/
			
		}
}

int main(){
	setup(); /*MV systemcall, retrieve MV- server looks for the name, return index; don't create new*/
	index = GetMV(MyLineAppClerk);
	SetMV(MyLineAppClerk, index+1);
	getCustomer(index);

	return 0;
}