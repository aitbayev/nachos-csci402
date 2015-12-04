#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;
			PrintNum(myLine);
			Write("That was me; pic clerk\n", sizeof("That was me; pic clerk\n"), ConsoleOutput);
				
			/*PrintNum(myLine);*/
			if (GetMV(picture_clerks[myLine].bribeLineCount)>0){ /*if someone on bribe line */
				Write("   Someone is in Bribe line\n", sizeof("   Someone is in Bribe line\n"), ConsoleOutput);
				Acquire(PicClerkBribeLineLock[myLine]);
				Signal(PicClerkBribeLineLock[myLine], PicClerkBribeLineCV[myLine]); /*wake them up, call to register*/
				SetMV(picture_clerks[myLine].state, 0); /*make myself busy */
				picture_clerks[myLine].money = picture_clerks[myLine].money + 500;
				SetMV(manager.picClerkMoney, GetMV(manager.picClerkMoney)+500);
				Write("-PicClerk-Received bribe from customer\n", sizeof("-PicClerk-Received bribe from customer\n"), ConsoleOutput);
				
				Release(PicClerkBribeLineLock[myLine]); 
			}
			else if(GetMV(picture_clerks[myLine].lineCount)>0){ /* if someone on regular line  */
				Write("   Someone is in Regular Line, no one in Bribe line\n", sizeof("   Someone is in Regular Line, no one in Bribe line\n"), ConsoleOutput);
				Acquire(PicClerkLineLock[myLine]);
				Signal(PicClerkLineLock[myLine], PicClerkLineCV[myLine]); /* wake up waiting customer*/
				SetMV(picture_clerks[myLine].state, 0); /*state is busy now*/
				Release(PicClerkLineLock[myLine]);
			}
	
			else{
				SetMV(picture_clerks[myLine].state, 1); /*app clerk is available*/
			}	 
		
			/*if(picture_clerks[myLine].state != available){*/
				Acquire(PicClerkLock[myLine]); /*acquire interaction lock*/
				/*PrintNum(PicClerkLock[0]);*/
				Write("-PicClerk-Acquired Lock\n", sizeof("-PicClerk-Acquired Lock\n"), ConsoleOutput);
				Wait(PicClerkLock[myLine], PicClerkCV[myLine]); /*wait for customer to give SSN*/
				SetMV(customer_data[customer_counter].picture, 1);	
				Write("--Picture is taken \n", sizeof("--Picture is taken \n"), ConsoleOutput);
				/*SetMV(customer_data[customer_counter].picture, 1);*/
				Signal(PicClerkLock[myLine], PicClerkCV[myLine]);
				/*if (picture_clerks[myLine].bribe == true){ 
					picture_clerks[myLine].money = picture_clerks[myLine].money + 500;
					Write("-PicClerk-Received bribe from customer\n", sizeof("-PicClerk-Received bribe from customer\n"), ConsoleOutput);
					picture_clerks[myLine].bribe = false;
				}*/
				/*Wait(PicClerkLock[myLine], PicClerkCV[myLine]);
				/*Signal(PicClerkLock[myLine], PicClerkCV[myLine]); /*signal the customer that record is completed*/
				SetMV(picture_clerks[myLine].state, 1);
				Release(PicClerkLock[myLine]); /*release the lock*/
				/*break;*/
			}
		/*}*/
}

int main(){
	setup();
	index = GetMV(MyLinePicClerk);
	SetMV(MyLinePicClerk, index+1);
	getCustomer(index);

	return 0;
}