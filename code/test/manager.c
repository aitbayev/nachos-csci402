#include "syscall.h"
#include "setup.h"

	int a;
	int i;

void moneyCount(){
		 while(true){
		 	if(a == 5){
		 		Write("WILL STOP OUTPUTTING MONEY\n", sizeof("WILL STOP OUTPUTTING MONEY\n"), ConsoleOutput);
		 		break;
		 	}
			for (i=0; i<300; i++){
				Yield();
			}
			SetMV(manager.totalMoney, GetMV(manager.appClerkMoney)+GetMV(manager.picClerkMoney)+GetMV(manager.passClerkMoney)+GetMV(manager.cashierMoney));
			PrintNum(GetMV(manager.totalMoney));
			Write("Manager: total money\n", sizeof("Manager: total money\n"), ConsoleOutput);
			a++;
		}
}

int main(){
	setup();
	moneyCount();

	return 0;
}