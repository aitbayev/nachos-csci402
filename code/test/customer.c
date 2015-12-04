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
	if (customers[arg].money >= 600){
		PrintNum(arg);
		Write("That was me; my money >= 600, bribe\n", sizeof("That was me; my money >= 600, bribe\n"), ConsoleOutput);
		for (a = 0; a<2; a++){
				if(GetMV(application_clerks[a].bribeLineCount)<bribeLineSize && GetMV(application_clerks[a].state) != 2){
				/*if(application_clerks[a].bribeLineCount<bribeLineSize && application_clerks[a].state != onBreak){
				/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
					PrintNum(GetMV(application_clerks[a].bribeLineCount));
					Write("# of people in bribe line\n", sizeof("# of people in bribe line\n"), ConsoleOutput);
					myLine = a; /*this is my new line*/
					bribeLineSize = GetMV(application_clerks[a].bribeLineCount);/* bribe line size is set to this line's size*/
					if(GetMV(application_clerks[a].bribeLineCount) == 0){
						break;
					}
				}
		}
		PrintNum(myLine);
		Write("I chose that appclerk\n", sizeof("I chose that appclerk\n"), ConsoleOutput);
		bribed = true; 
		
	}
	else{
		PrintNum(arg);
		Write("That was me; my money <= 100, can't bribe\n", sizeof("That was me; my money <= 100, can't bribe\n"), ConsoleOutput);

		for (a = 0; a<2; a++){
			if(GetMV(application_clerks[a].lineCount)<lineSize && GetMV(application_clerks[a].state) != 2){
			/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
				myLine = a; /*this is my new line*/
				/*PrintNum(myLine);*/
				lineSize = GetMV(application_clerks[a].lineCount); /*line size is set to this line's size*/
				if(GetMV(application_clerks[a].lineCount) == 0){
					break;
				}
			}
		}
		PrintNum(myLine);
		Write("I chose that appclerk\n", sizeof("I chose that appclerk\n"), ConsoleOutput);
		bribed = false; /*can't bribe*/
	}
	
	Release(PickAppClerkLineLock); /*release line picking lock */
	

		
	if (bribed == false){ /*if customer didn'`t bribe */
		Acquire(AppClerkLineLock[myLine]);
		if(GetMV(application_clerks[myLine].state) == 0){ /* if clerk is busy*/
			SetMV(application_clerks[myLine].lineCount, GetMV(application_clerks[myLine].lineCount)+1); /*customer gets in line- increment line number	*/
			Write("--Got in regular line \n", sizeof("--Got in regular line \n"), ConsoleOutput);
			Wait(AppClerkLineLock[myLine], AppClerkLineCV[myLine]); /*wait on clerks waiting queue until he signal customer*/
			SetMV(application_clerks[myLine].lineCount, GetMV(application_clerks[myLine].lineCount)-1);	/*decrement line count of the clerk, means he signalled customer*/
			
		}
		else{
			Write("--Application clerk is not busy \n", sizeof("--Application clerk is not busy \n"), ConsoleOutput);
		 	SetMV(application_clerks[myLine].state, 0); /*else he is busy going right to register*/
		}
		application_clerks[myLine].bribe = false; /* not bribing*/
		Release(AppClerkLineLock[myLine]);
	}
	else{ /*customer bribed*/
			

		Acquire(AppClerkBribeLineLock[myLine]);
		if(GetMV(application_clerks[myLine].state) == 0){ /* if clerk is busy */
			SetMV(application_clerks[myLine].bribeLineCount, GetMV(application_clerks[myLine].bribeLineCount)+1);	/*customer gets in line- increment line number*/
			/*PrintNum(application_clerks[myLine].bribeLineCount);	*/
			Write("--Got in bribe line, busy \n", sizeof("--Got in bribe line, busy \n"), ConsoleOutput);
			Wait(AppClerkBribeLineLock[myLine],AppClerkBribeLineCV[myLine]);/*wait on clerks waiting queue until he signal customer*/
			SetMV(application_clerks[myLine].bribeLineCount, GetMV(application_clerks[myLine].bribeLineCount)-1);	/* decrement line count of the clerk, means he signalled customer*/
			Write("  bribe line decremented \n", sizeof("  bribe line decremented \n"), ConsoleOutput);
			application_clerks[myLine].bribe = true; /*customer is bribing*/
			customers[arg].money = customers[arg].money - 500; /* reducing customer's money*/
			PrintNum(customers[arg].money);
			Write(":( Money I have left \n", sizeof(":( Money I have left \n"), ConsoleOutput);
		}
		else{
		 	SetMV(application_clerks[myLine].state, 0);/* no one is in the line going straight to him*/
		 	Write("--Application clerk is not busy, so I go straight to him- now he is busy \n", sizeof("--Application clerk is not busy, so I go straight to him- now he is busy \n"), ConsoleOutput);
		 	application_clerks[myLine].bribe = false; /*not bribing*/
		 	Write("THUS I didn't have to bribe \n", sizeof("THUS I didn't have to bribe \n"), ConsoleOutput);
			PrintNum(customers[arg].money);
			Write(":( Money I have left \n", sizeof(":( Money I have left \n"), ConsoleOutput);	
		}
		Release(AppClerkBribeLineLock[myLine]);	
		
	} 

    Acquire(AppClerkLock[myLine]);
    Write("-Client-Acquired Lock\n", sizeof("Client-Acquired Lock\n"), ConsoleOutput);
	application_clerks[myLine].ssn = arg; 
	Signal(AppClerkLock[myLine], AppClerkCV[myLine]); /*wakeup the clerk*/
	Wait(AppClerkLock[myLine], AppClerkCV[myLine]); /*no need to wait*/
	/*Signal(AppClerkLock[myLine], AppClerkCV[myLine]);*/
	Release(AppClerkLock[myLine]);
	PrintNum(arg);
	Write("\n***Leaving AppClerk***\n", sizeof("\n***Leaving AppClerk***\n"), ConsoleOutput);

}

void goToPicClerk(int arg){
	bribed = false;
	Acquire(PickPicClerkLineLock);
	if (customers[arg].money >= 600){
		PrintNum(arg);
		Write("That was me; my money >= 600, bribe\n", sizeof("That was me; my money >= 600, bribe\n"), ConsoleOutput);
		for (a = 0; a<2; a++){
				if(GetMV(picture_clerks[a].bribeLineCount)<bribeLineSize && GetMV(picture_clerks[a].state) != 2){
				/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
					PrintNum(GetMV(picture_clerks[a].bribeLineCount));
					Write("# of people in bribe line\n", sizeof("# of people in bribe line\n"), ConsoleOutput);
					
					myLine = a; /*this is my new line*/
					/*PrintNum(myLine);*/
					bribeLineSize = GetMV(picture_clerks[a].bribeLineCount);/* bribe line size is set to this line's size*/
					if(GetMV(picture_clerks[a].bribeLineCount) == 0){
						break;
					}
				}
		}
		PrintNum(myLine);
		Write("I chose that picclerk\n", sizeof("I chose that picclerk\n"), ConsoleOutput);
		bribed = true; 
	}
	else{
		PrintNum(arg);
		Write("That was me; my money <= 100, can't bribe\n", sizeof("That was me; my money <= 100, can't bribe\n"), ConsoleOutput);

		for (a = 0; a<2; a++){
			if(GetMV(picture_clerks[a].lineCount)<lineSize && GetMV(picture_clerks[a].state) != 2){
			/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
				myLine = a; /*this is my new line*/
				/*PrintNum(myLine);*/
				lineSize = GetMV(picture_clerks[a].lineCount); /*line size is set to this line's size*/
				if(GetMV(picture_clerks[a].lineCount) == 0){
					break;
				}
			}
		}
		PrintNum(myLine);
		Write("I chose that picclerk\n", sizeof("I chose that picclerk\n"), ConsoleOutput);
		bribed = false; /*can't bribe*/
	}
	Release(PickPicClerkLineLock); /*release line picking lock */
	
	if (bribed == false){ /*if customer didn'`t bribe */
		Acquire(PicClerkLineLock[myLine]);
		if(GetMV(picture_clerks[myLine].state) == 0){ /* if clerk is not busy*/
			SetMV(picture_clerks[myLine].lineCount, GetMV(picture_clerks[myLine].lineCount)+1); /*customer gets in line- increment line number	*/
			Write("--Got in regular line \n", sizeof("--Got in regular line \n"), ConsoleOutput);
			Wait(PicClerkLineLock[myLine], PicClerkLineCV[myLine]); /*wait on clerks waiting queue until he signal customer*/
			SetMV(picture_clerks[myLine].lineCount, GetMV(picture_clerks[myLine].lineCount)-1);	/*decrement line count of the clerk, means he signalled customer*/
			
		}
		else{
			Write("--Picture clerk is not busy \n", sizeof("--Picture clerk is not busy \n"), ConsoleOutput);
		 	SetMV(picture_clerks[myLine].state, 0); /*else he is busy going right to register*/
		}
		picture_clerks[myLine].bribe = false; /* not bribing*/
		Release(PicClerkLineLock[myLine]);
	}
	else{ /*customer bribed*/
		Acquire(PicClerkBribeLineLock[myLine]);
		if(GetMV(picture_clerks[myLine].state) == 0){ /* if clerk is not busy */
			SetMV(picture_clerks[myLine].bribeLineCount, GetMV(picture_clerks[myLine].bribeLineCount)+1);	/*customer gets in line- increment line number*/
			/*PrintNum(picture_clerks[myLine].bribeLineCount);	*/
			Write("--Got in bribe line, busy \n", sizeof("--Got in bribe line, busy \n"), ConsoleOutput);
			Wait(PicClerkBribeLineLock[myLine],PicClerkBribeLineCV[myLine]);/*wait on clerks waiting queue until he signal customer*/
			SetMV(picture_clerks[myLine].bribeLineCount, GetMV(picture_clerks[myLine].bribeLineCount)-1);	/* decrement line count of the clerk, means he signalled customer*/
			Write("  bribe line decremented \n", sizeof("  bribe line decremented \n"), ConsoleOutput);
			picture_clerks[myLine].bribe = true; /*customer is bribing*/
			customers[arg].money = customers[arg].money - 500; /* reducing customer's money*/
			PrintNum(customers[arg].money);
			Write(":( Money I have left \n", sizeof(":( Money I have left \n"), ConsoleOutput);
		
		}
		else{
		 	SetMV(picture_clerks[myLine].state, 0);/* no one is in the line going straight to him*/
		 	Write("--Picture clerk is not busy, so I go straight to him- now he is busy \n", sizeof("--Picture clerk is not busy, so I go straight to him- now he is busy \n"), ConsoleOutput);
		 	picture_clerks[myLine].bribe = false; /*not bribing*/
		 	Write("THUS I didn't have to bribe \n", sizeof("THUS I didn't have to bribe \n"), ConsoleOutput);
			PrintNum(customers[arg].money);
			Write(":( Money I have left \n", sizeof(":( Money I have left \n"), ConsoleOutput);	
		
		}
		Release(PicClerkBribeLineLock[myLine]);	
	} 

    Acquire(PicClerkLock[myLine]);
    Write("-Client-Acquired Lock\n", sizeof("Client-Acquired Lock\n"), ConsoleOutput);
	Signal(PicClerkLock[myLine], PicClerkCV[myLine]); /*wakeup the clerk*/
	Wait(PicClerkLock[myLine], PicClerkCV[myLine]); /*no need to wait*/
	/*Signal(PicClerkLock[myLine], PicClerkCV[myLine]);*/
	Release(PicClerkLock[myLine]);
	
	PrintNum(arg);
	Write("\n***Leaving PicClerk***\n", sizeof("\n***Leaving PicClerk***\n"), ConsoleOutput);
}

void goToPassClerk(int arg){
	bribed = false;
	Acquire(PickPassClerkLineLock);
	if (customers[arg].money >= 600){
		PrintNum(arg);
		Write("That was me; my money >= 600, bribe\n", sizeof("That was me; my money >= 600, bribe\n"), ConsoleOutput);
		
		for (a = 0; a<2; a++){
				if(GetMV(passport_clerks[a].bribeLineCount)<bribeLineSize && GetMV(passport_clerks[a].state) != 2){
				/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
					PrintNum(GetMV(passport_clerks[a].bribeLineCount));
					Write("# of people in bribe line\n", sizeof("# of people in bribe line\n"), ConsoleOutput);
					
					myLine = a; /*this is my new line*/
					/*PrintNum(myLine);*/
					bribeLineSize = GetMV(passport_clerks[a].bribeLineCount);/* bribe line size is set to this line's size*/
					if(GetMV(passport_clerks[a].bribeLineCount) == 0){
						break;
					}
				}
					
		}
		PrintNum(myLine);
		Write("I chose that passclerk\n", sizeof("I chose that passclerk\n"), ConsoleOutput);
		
		bribed = true; 
	}
	else{
		PrintNum(arg);
		Write("That was me; my money <= 100, can't bribe\n", sizeof("That was me; my money <= 100, can't bribe\n"), ConsoleOutput);

		for (a = 0; a<2; a++){
			if(GetMV(passport_clerks[a].lineCount)<lineSize && GetMV(passport_clerks[a].state) != 2){
			/*if the clerk is not on break and bribe line is shorter than the previous bribe line I checked (if first time, it is satisfied)*/
				myLine = a; /*this is my new line*/
				/*PrintNum(myLine);*/
				lineSize = GetMV(passport_clerks[a].lineCount); /*line size is set to this line's size*/
				if(GetMV(passport_clerks[a].lineCount) == 0){
					break;
				}
			}
		}
		PrintNum(myLine);
		Write("I chose that passclerk\n", sizeof("I chose that passclerk\n"), ConsoleOutput);
		
		bribed = false; /*can't bribe*/
	}
	Release(PickPassClerkLineLock); /*release line picking lock */
	
	if (bribed == false){ /*if customer didn'`t bribe */
		Acquire(PassClerkLineLock[myLine]);
		if(GetMV(passport_clerks[myLine].state) == 0){ /* if clerk is not busy*/
			SetMV(passport_clerks[myLine].lineCount, GetMV(passport_clerks[myLine].lineCount)+1); /*customer gets in line- increment line number	*/
			Write("--Got in regular line \n", sizeof("--Got in regular line \n"), ConsoleOutput);
			Wait(PassClerkLineLock[myLine], PassClerkLineCV[myLine]); /*wait on clerks waiting queue until he signal customer*/
			SetMV(passport_clerks[myLine].lineCount, GetMV(passport_clerks[myLine].lineCount)-1); /*decrement line count of the clerk, means he signalled customer*/
			
		}
		else{
			Write("--Passport clerk is not busy \n", sizeof("--Passport clerk is not busy \n"), ConsoleOutput);
		 	SetMV(passport_clerks[myLine].state, 0); /*else he is busy going right to register*/
		}
		passport_clerks[myLine].bribe = false; /* not bribing*/
		Release(PassClerkLineLock[myLine]);
	}
	else{ /*customer bribed*/
		Acquire(PassClerkBribeLineLock[myLine]);
		if(GetMV(passport_clerks[myLine].state) == 0){ /* if clerk is not busy */
			SetMV(passport_clerks[myLine].bribeLineCount, GetMV(passport_clerks[myLine].bribeLineCount)+1);	/*customer gets in line- increment line number*/
			/*PrintNum(passport_clerks[myLine].bribeLineCount);	*/
			Write("--Got in bribe line, busy \n", sizeof("--Got in bribe line, busy \n"), ConsoleOutput);
			Wait(PassClerkBribeLineLock[myLine],PassClerkBribeLineCV[myLine]);/*wait on clerks waiting queue until he signal customer*/
			SetMV(passport_clerks[myLine].bribeLineCount, GetMV(passport_clerks[myLine].bribeLineCount)-1);	/* decrement line count of the clerk, means he signalled customer*/
			Write("  bribe line decremented \n", sizeof("  bribe line decremented \n"), ConsoleOutput);
			passport_clerks[myLine].bribe = true; /*customer is bribing*/
			customers[arg].money = customers[arg].money - 500; /* reducing customer's money*/
			PrintNum(customers[arg].money);
			Write(":( Money I have left \n", sizeof(":( Money I have left \n"), ConsoleOutput);
		
		}
		else{
		 	SetMV(passport_clerks[myLine].state, 0);/* no one is in the line going straight to him*/
		 	Write("--Passport clerk is not busy \n", sizeof("--Passport clerk is not busy \n"), ConsoleOutput);
		 	passport_clerks[myLine].bribe = false; /*not bribing*/
		 	Write("THUS I didn't have to bribe \n", sizeof("THUS I didn't have to bribe \n"), ConsoleOutput);
			PrintNum(customers[arg].money);
			Write(":( Money I have left \n", sizeof(":( Money I have left \n"), ConsoleOutput);	
		
		}
		Release(PassClerkBribeLineLock[myLine]);	
	} 

    Acquire(PassClerkLock[myLine]);
    Write("-Client-Acquired Lock\n", sizeof("Client-Acquired Lock\n"), ConsoleOutput);
	passport_clerks[myLine].ssn = arg; 
	Signal(PassClerkLock[myLine], PassClerkCV[myLine]); /*wakeup the clerk*/
	Wait(PassClerkLock[myLine], PassClerkCV[myLine]); /*no need to wait*/
	/*Signal(PassClerkLock[myLine], PassClerkCV[myLine]);*/
	Release(PassClerkLock[myLine]);
	PrintNum(arg);
	Write("\n***Leaving PassClerk***\n", sizeof("\n***Leaving PassClerk***\n"), ConsoleOutput);
}

void goToCashier(int arg){
	bribed = false;
	Acquire(PickCashierLineLock);
	for (a = 0; a<2; a++){
		if(GetMV(cashiers[a].lineCount)<lineSize && GetMV(cashiers[a].state) != 2){
			myLine = a; /*this is my new line*/
			/*PrintNum(myLine);*/
			lineSize = GetMV(cashiers[a].lineCount); /*line size is set to this line's size*/
			if(GetMV(cashiers[a].lineCount) == 0){
				break;
			}
		}
	}
	PrintNum(myLine);
	Write("I chose that cashier\n", sizeof("I chose that cashier\n"), ConsoleOutput);

	Release(PickCashierLineLock); /*release line picking lock */
	
	if (bribed == false){ /*if customer didn'`t bribe */
		Acquire(CashierLineLock[myLine]);
		if(GetMV(cashiers[myLine].state) == 0){ /* if clerk is not busy*/
			SetMV(cashiers[myLine].lineCount, GetMV(cashiers[myLine].lineCount)+1); /*customer gets in line- increment line number	*/
			Write("--Got in regular line \n", sizeof("--Got in regular line \n"), ConsoleOutput);
			Wait(CashierLineLock[myLine], CashierLineCV[myLine]); /*wait on clerks waiting queue until he signal customer*/
			SetMV(cashiers[myLine].lineCount, GetMV(cashiers[myLine].lineCount)-1);	/*decrement line count of the clerk, means he signalled customer*/
			
		}
		else{
			Write("--Cashier is not busy \n", sizeof("--Cashier is not busy \n"), ConsoleOutput);
		 	SetMV(cashiers[myLine].state, 0); /*else he is busy going right to register*/
		}
		Release(CashierLineLock[myLine]);
	}

    Acquire(CashierLock[myLine]);
    Write("-Client-Acquired Lock\n", sizeof("Client-Acquired Lock\n"), ConsoleOutput);
	customer_data[arg].verified = true;
	Signal(CashierLock[myLine], CashierCV[myLine]); /*wakeup the clerk*/
	Wait(CashierLock[myLine], CashierCV[myLine]); /*no need to wait*/
	/*Signal(CashierLock[myLine], CashierCV[myLine]);*/
	Release(CashierLock[myLine]);
	PrintNum(arg);
	Write("\n***Leaving Cashier***\n", sizeof("\n***Leaving Cashier***\n"), ConsoleOutput);

}

int main(){
	setup();
	myIndex = GetMV(customer_index);
	SetMV(customer_index, myIndex+1);
	goToAppClerk(myIndex);
	Write("\n***Going to PicClerk***\n\n", sizeof("\n***Going to PicClerk***\n\n"), ConsoleOutput);
	goToPicClerk(myIndex);
	Write("\n***Going to PassClerk***\n\n", sizeof("\n***Going to PassClerk***\n\n"), ConsoleOutput);
	goToPassClerk(myIndex);
	Write("\n***Going to Cashier***\n\n", sizeof("\n***Going to Cashier***\n\n"), ConsoleOutput);
	goToCashier(myIndex);
	Write("\n----LEAVING PASSPORT OFFICE----\n", sizeof("\n----LEAVING PASSPORT OFFICE----\n"), ConsoleOutput);

}