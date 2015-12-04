#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;	
			PrintNum(myLine);
			Write("That was me; pass clerk\n", sizeof("That was me; pass clerk\n"), ConsoleOutput);
			
			if (GetMV(passport_clerks[myLine].bribeLineCount)>0){ /*if someone on bribe line */
				Write("   Someone is in Bribe line\n", sizeof("   Someone is in Bribe line\n"), ConsoleOutput);
				Acquire(PassClerkBribeLineLock[myLine]);
				Signal(PassClerkBribeLineLock[myLine], PassClerkBribeLineCV[myLine]); /*wake them up, call to register*/
				SetMV(passport_clerks[myLine].state, 0); /*make myself busy */
				passport_clerks[myLine].money = passport_clerks[myLine].money + 500;
				SetMV(manager.passClerkMoney, GetMV(manager.passClerkMoney)+500);
				Write("-PassClerk-Received bribe from customer\n", sizeof("-PassClerk-Received bribe from customer\n"), ConsoleOutput);
				
				Release(PassClerkBribeLineLock[myLine]); 
			}
			else if(GetMV(passport_clerks[myLine].lineCount)>0){ /* if someone on regular line  */
				Write("   Someone is in Regular Line, no one in Bribe line\n", sizeof("   Someone is in Regular Line, no one in Bribe line\n"), ConsoleOutput);
				Acquire(PassClerkLineLock[myLine]);
				Signal(PassClerkLineLock[myLine], PassClerkLineCV[myLine]); /* wake up waiting customer*/
				SetMV(passport_clerks[myLine].state, 0); /*state is busy now*/
				Release(PassClerkLineLock[myLine]);
			}
	
			else{
				SetMV(passport_clerks[myLine].state, 1); /*app clerk is available*/
			}	 
	
			Acquire(PassClerkLock[myLine]); /*acquire interaction lock*/
			Write("-PassClerk-Acquired Lock\n", sizeof("-PassClerk-Acquired Lock\n"), ConsoleOutput);
			Wait(PassClerkLock[myLine], PassClerkCV[myLine]); /*wait for customer to give SSN*/
	
			Write("--Got ssn and app from customer- verified \n", sizeof("--Got ssn and app from customer- verified \n"), ConsoleOutput);
			SetMV(customer_data[customer_counter].verified, 1);
		
			Signal(PassClerkLock[myLine], PassClerkCV[myLine]);
			/*if (passport_clerks[myLine].bribe == true){ 
				passport_clerks[myLine].money = passport_clerks[myLine].money + 500;
				Write("-PassClerk-Received bribe from customer\n", sizeof("-PassClerk-Received bribe from customer\n"), ConsoleOutput);
				passport_clerks[myLine].bribe = false;
			}*/
			SetMV(passport_clerks[myLine].state, 1);
			/*Wait(PassClerkLock[myLine], PassClerkCV[myLine]);
			Signal(PassClerkLock[myLine], PassClerkCV[myLine]); /*signal the customer that record is completed*/
			Release(PassClerkLock[myLine]); /*release the lock*/
			/*break;*/
		}
}

int main(){
	setup();
	index = GetMV(MyLinePassClerk);
	SetMV(MyLinePassClerk, index+1);
	getCustomer(index);

	return 0;
}