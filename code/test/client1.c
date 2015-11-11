#include "syscall.h"


int main(){
	int lock1;
	lock1 = CreateLock("lock1", sizeof("lock1"));
	PrintNum(lock1);

return 0;
}