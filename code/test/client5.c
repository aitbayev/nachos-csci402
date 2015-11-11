#include "syscall.h"


int main(){
	int lock5;
	lock5 = CreateLock("lock5", sizeof("lock5"));
	PrintNum(lock5);

return 0;
}