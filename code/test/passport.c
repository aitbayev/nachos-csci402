/*
	TEST 2: 4 customers
*/

#include "syscall.h"
#include "setup.h"

int main(){

Exec("../test/network_test", sizeof("../test/network_test"));
Exec("../test/network_test2", sizeof("../test/network_test2"));

/*setup();*/
/*Exec("../test/app_clerk", sizeof("../test/app_clerk"));
Exec("../test/app_clerk", sizeof("../test/app_clerk"));

Exec("../test/customer", sizeof("../test/customer"));
Exec("../test/customer", sizeof("../test/customer"));
Exec("../test/customer", sizeof("../test/customer"));
Exec("../test/customer", sizeof("../test/customer"));

Exec("../test/pic_clerk", sizeof("../test/pic_clerk"));
Exec("../test/pass_clerk", sizeof("../test/pass_clerk"));
Exec("../test/cashier", sizeof("../test/cashier"));

/* NOTE: The rest of clerks' Execs are in customer.c */

return 0;
}