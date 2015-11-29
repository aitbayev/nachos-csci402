#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;	
			PrintNum(myLine);
			if (passport_clerks[myLine].bribeLineCount>0){ /*if someone on bribe line */
				Write("   Bribe line", sizeof("  Bribe line"), ConsoleOutput);
				Acquire(PassClerkBribeLineLock[myLine]);
				Signal(PassClerkBribeLineLock[myLine], PassClerkBribeLineCV[myLine]); /*wake them up, call to register*/
				passport_clerks[myLine].state = busy; /*make myself busy */
				Release(PassClerkBribeLineLock[myLine]); 
			}
			else if(passport_clerks[myLine].lineCount>0){ /* if someone on regular line  */
				Write("   Regular Line", sizeof("  Regular line"), ConsoleOutput);
				Acquire(PassClerkLineLock[myLine]);
				Signal(PassClerkLineLock[myLine], PassClerkLineCV[myLine]); /* wake up waiting customer*/
				passport_clerks[myLine].state = busy; /*state is busy now*/
				Release(PassClerkLineLock[myLine]);
			}
	
			else{
				passport_clerks[myLine].state = available; /*app clerk is available*/
			}	 
	
			Acquire(PassClerkLock[myLine]); /*acquire interaction lock*/
			Wait(PassClerkLock[myLine], PassClerkCV[myLine]); /*wait for customer to give SSN*/
	
			customer_data[customer_counter].verified = true;	
		
			Signal(PassClerkLock[myLine], PassClerkCV[myLine]);
			if (passport_clerks[myLine].bribe == true){ 
				passport_clerks[myLine].money = passport_clerks[myLine].money + 500;
				passport_clerks[myLine].bribe = false;
			}
			Signal(PassClerkLock[myLine], PassClerkCV[myLine]); /*signal the customer that record is completed*/
			Release(PassClerkLock[myLine]); /*release the lock*/
		}
}

int main(){

	index = GetMV(MyLinePassClerk);
	SetMV(MyLinePassClerk, index+1);

	getCustomer(index);


}