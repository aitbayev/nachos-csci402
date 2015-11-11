#include "syscall.h"


int main(){
	int lock2;
	lock2 = CreateLock("lock2", sizeof("lock2"));
	PrintNum(lock2);

return 0;
}