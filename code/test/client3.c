#include "syscall.h"


int main(){
	int lock3;
	lock3 = CreateLock("lock3", sizeof("lock3"));
	PrintNum(lock3);

return 0;
}