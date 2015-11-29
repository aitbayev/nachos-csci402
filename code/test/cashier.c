#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;	
			PrintNum(myLine);
			if(cashiers[myLine].lineCount>0){ /* if someone on regular line  */
				Write("   Regular Line", sizeof("  Regular line"), ConsoleOutput);
				Acquire(CashierLineLock[myLine]);
				Signal(CashierLineLock[myLine], CashierLineCV[myLine]); /* wake up waiting customer*/
				cashiers[myLine].state = busy; /*state is busy now*/
				Release(CashierLineLock[myLine]);
			}
			else{
				cashiers[myLine].state = available; /*app clerk is available*/
			}	 
	
			Acquire(CashierLock[myLine]); /*acquire interaction lock*/
			Wait(CashierLock[myLine], CashierCV[myLine]); /*wait for customer to give SSN*/
	
			if (customer_data[customer_counter].verified == true){
				customer_data[customer_counter].got_passport = true;
			}
			
			Signal(CashierLock[myLine], CashierCV[myLine]);
			
			cashiers[myLine].money = cashiers[myLine].money + 1500;
				
			Signal(CashierLock[myLine], CashierCV[myLine]); /*signal the customer that record is completed*/
			Release(CashierLock[myLine]); /*release the lock*/

		}
}

int main(){

	index = GetMV(MyLineCashier);
	SetMV(MyLineCashier, index+1);

	getCustomer(index);


}