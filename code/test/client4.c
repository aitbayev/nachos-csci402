#include "syscall.h"


int main(){
	int lock4;
	lock4 = CreateLock("lock4", sizeof("lock4"));
	PrintNum(lock4);

return 0;
}