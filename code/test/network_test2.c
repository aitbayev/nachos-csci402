#include "syscall.h"

int lk1;
int cv1;

int main(){
	lk1 = CreateLock("lk1", sizeof("lk1"));
	cv1 = CreateCondition("cv1", sizeof("cv1"));
	
	Acquire(lk1);
	
	Write("Test 2:Acquired lk1 \n", sizeof("Test 2:Acquired lk1 \n"), ConsoleOutput);

	Signal(lk1, cv1);
	Write("Test 2:Signalled lk1 \n", sizeof("Test 2:Signalled lk1 \n"), ConsoleOutput);

	Wait(lk1, cv1);
	Write("Test 2:I was signalled lk1 \n", sizeof("Test 2:I was signalled lk1 \n"), ConsoleOutput);

	Release(lk1);
	Write("Test 2:Released lk1 \n", sizeof("Test 2:Released lk1 \n"), ConsoleOutput);
}