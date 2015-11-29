#include "syscall.h"
#include "setup.h";

int main(){


setup();
Exec("../test/app_clerk", sizeof("../test/app_clerk"));
Exec("../test/app_clerk", sizeof("../test/app_clerk"));

Exec("../test/customer", sizeof("../test/customer"));
Exec("../test/customer", sizeof("../test/customer"));
Exec("../test/customer", sizeof("../test/customer"));
Exec("../test/customer", sizeof("../test/customer"));


}