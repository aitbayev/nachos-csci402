#include "syscall.h"
#include "setup.h"

	int index;
	int a;


void getCustomer(int index){
		 while(true){
			int myLine = index;	
			PrintNum(myLine);
			Write("That was me; cashier\n", sizeof("That was me; cashier\n"), ConsoleOutput);
			
			if(GetMV(cashiers[myLine].lineCount)>0){ /* if someone on regular line  */
				Write("   Regular Line", sizeof("  Regular line"), ConsoleOutput);
				Acquire(CashierLineLock[myLine]);
				Signal(CashierLineLock[myLine], CashierLineCV[myLine]); /* wake up waiting customer*/
				SetMV(cashiers[myLine].state, 0); /*state is busy now*/
				cashiers[myLine].money = cashiers[myLine].money + 100;
				SetMV(manager.cashierMoney, GetMV(manager.cashierMoney)+100);
				Release(CashierLineLock[myLine]);
			}
			else{
				SetMV(cashiers[myLine].state, 1); /*app clerk is available*/
			}	 
	
	
			Acquire(CashierLock[myLine]); /*acquire interaction lock*/
			Write("-Cashier-Acquired Lock\n", sizeof("-Cashier-Acquired Lock\n"), ConsoleOutput);
			Wait(CashierLock[myLine], CashierCV[myLine]); /*wait for customer to give SSN*/
	
			if (GetMV(customer_data[customer_counter].verified) == 1){
				SetMV(customer_data[customer_counter].got_passport, 1);
				Write("--Verified; gave passport \n", sizeof("--Verified; gave passport \n"), ConsoleOutput);
			}
			
			Signal(CashierLock[myLine], CashierCV[myLine]);

			Write("-Cashier-Received money from customer\n", sizeof("-Cashier-Received money from customer\n"), ConsoleOutput);
			SetMV(cashiers[myLine].state, 1);
			/*Wait(CashierLock[myLine], CashierCV[myLine]);
			/*Signal(CashierLock[myLine], CashierCV[myLine]); /*signal the customer that record is completed*/
			Release(CashierLock[myLine]); /*release the lock*/
			/*break;*/
		}
}

int main(){
	setup();
	index = GetMV(MyLineCashier);
	SetMV(MyLineCashier, index+1);
	getCustomer(index);

	return 0;
}