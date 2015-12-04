#include "../userprog/syscall.h"

int main()
{

/*int a;*/
    Write("\nTesting Exec\n", sizeof("\nTesting Exec\n"), ConsoleOutput);

	/*a = CreateLock("", 0);*/
    Exec("../test/exectest",sizeof("../test/"));
    Write("\nExec called successfully\n", sizeof("\nExec called successfully\n"), ConsoleOutput);
    /* not reached */
}