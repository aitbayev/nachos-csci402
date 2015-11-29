#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;	
			PrintNum(myLine);
			if (application_clerks[myLine].bribeLineCount>0){ /*if someone on bribe line */
				Write("   Bribe line", sizeof("  Bribe line"), ConsoleOutput);
				Acquire(AppClerkBribeLineLock[myLine]);
				Signal(AppClerkBribeLineLock[myLine], AppClerkBribeLineCV[myLine]); /*wake them up, call to register*/
				application_clerks[myLine].state = busy; /*make myself busy */
				Release(AppClerkBribeLineLock[myLine]); 
			}
			else if(application_clerks[myLine].lineCount>0){ /* if someone on regular line  */
				Write("   Regular Line", sizeof("  Regular line"), ConsoleOutput);
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
	
			customer_data[customer_counter].SSN = application_clerks[myLine].ssn;
			Write("--Got ssn and app from customer \n", sizeof("--Got ssn and app from customer \n"), ConsoleOutput);
			customer_data[customer_counter].application = true;
			
			/*customers[customer_data[i].SSN].atAppClerk = true; */
				
		
			/*Signal(AppClerkLock[myLine], AppClerkCV[myLine]);
			if (application_clerks[myLine].bribe == true){ 
				application_clerks[myLine].money = application_clerks[myLine].money + 500;
				application_clerks[myLine].bribe = false;
			}
			Signal(AppClerkLock[myLine], AppClerkCV[myLine]); /*signal the customer that record is completed*/
			/*Release(AppClerkLock[myLine]); /*release the lock*/

		}
}

int main(){

	setup();
	
	index = GetMV(MyLineAppClerk);
	SetMV(MyLineAppClerk, index+1);

	getCustomer(index);


}