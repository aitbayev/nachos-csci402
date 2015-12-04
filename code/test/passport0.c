/*
	TEST 1: 1 customer
*/

#include "syscall.h"
#include "setup.h"

int main(){

int i;

/*setup();*/

Exec("../test/app_clerk", sizeof("../test/app_clerk"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/app_clerk", sizeof("../test/app_clerk"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/pic_clerk", sizeof("../test/pic_clerk"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/pic_clerk", sizeof("../test/pic_clerk"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/pass_clerk", sizeof("../test/pass_clerk"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/pass_clerk", sizeof("../test/pass_clerk"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/cashier", sizeof("../test/cashier"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/cashier", sizeof("../test/cashier"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/customer", sizeof("../test/customer"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/customer", sizeof("../test/customer"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/customer", sizeof("../test/customer"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/customer", sizeof("../test/customer"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/customer", sizeof("../test/customer"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/customer", sizeof("../test/customer"));
for (i=0; i<40; i++){
	Yield();
}
Exec("../test/manager", sizeof("../test/manager"));

/* NOTE: The rest of clerks' Execs are in customer.c */

return 0;
}