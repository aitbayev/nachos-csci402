#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;	
			PrintNum(myLine);
			if (picture_clerks[myLine].bribeLineCount>0){ /*if someone on bribe line */
				Write("   Bribe line", sizeof("  Bribe line"), ConsoleOutput);
				Acquire(PicClerkBribeLineLock[myLine]);
				Signal(PicClerkBribeLineLock[myLine], PicClerkBribeLineCV[myLine]); /*wake them up, call to register*/
				picture_clerks[myLine].state = busy; /*make myself busy */
				Release(PicClerkBribeLineLock[myLine]); 
			}
			else if(picture_clerks[myLine].lineCount>0){ /* if someone on regular line  */
				Write("   Regular Line", sizeof("  Regular line"), ConsoleOutput);
				Acquire(PicClerkLineLock[myLine]);
				Signal(PicClerkLineLock[myLine], PicClerkLineCV[myLine]); /* wake up waiting customer*/
				picture_clerks[myLine].state = busy; /*state is busy now*/
				Release(PicClerkLineLock[myLine]);
			}
	
			else{
				picture_clerks[myLine].state = available; /*app clerk is available*/
			}	 
	
			Acquire(PicClerkLock[myLine]); /*acquire interaction lock*/
			Wait(PicClerkLock[myLine], PicClerkCV[myLine]); /*wait for customer to give SSN*/
	
			customer_data[customer_counter].picture= true;	
		
			Signal(PicClerkLock[myLine], PicClerkCV[myLine]);
			if (picture_clerks[myLine].bribe == true){ 
				picture_clerks[myLine].money = picture_clerks[myLine].money + 500;
				picture_clerks[myLine].bribe = false;
			}
			Signal(PicClerkLock[myLine], PicClerkCV[myLine]); /*signal the customer that record is completed*/
			Release(PicClerkLock[myLine]); /*release the lock*/

		}
}

int main(){

	index = GetMV(MyLinePicClerk);
	SetMV(MyLinePicClerk, index+1);

	getCustomer(index);


}