#include "syscall.h"

	int mv1;
	int mv2;
	int mv3;
	int mv4;
	int mv5;
	int mv6;
	int set;
	int get1;
	int get2;
	int removeMV1;
	int set1;
	int set2;
	
	
	int lk1;
	int lk2;
	int lk3;
	int lk4;
	int lk5;
	int lk6;
	
	int cv1;
	int cv2;
	int cv3;
	int cv4;
	int cv5;



/* First 5 locks will be created by one server the last one should be created by another server, it will have index 0 */
void CreateMVMultipleLocks(){
	mv1 = CreateMV("Monitor1", sizeof("Monitor1"));
	mv2 = CreateMV("Monitor2", sizeof("Monitor2"));
	mv3 = CreateMV("Monitor3", sizeof("Monitor3"));
	mv4 = CreateMV("Monitor4", sizeof("Monitor4"));
	mv5 = CreateMV("Monitor5", sizeof("Monitor5"));
	mv6 = CreateMV("Monitor6", sizeof("Monitor6"));
	Write("created by SAME server \n", sizeof("created by SAME server \n"), ConsoleOutput);
	PrintNum(mv1);
	Write("created by SAME server \n", sizeof("created by SAME server \n"), ConsoleOutput);
	PrintNum(mv2);
	Write("created by SAME server \n", sizeof("created by SAME server \n"), ConsoleOutput);
	PrintNum(mv3);
	Write("created by SAME server \n", sizeof("created by SAME server \n"), ConsoleOutput);
	PrintNum(mv4);
	Write("created by SAME server \n", sizeof("created by SAME server \n"), ConsoleOutput);
	PrintNum(mv5);
	Write("created by DIFFERENT server \n", sizeof("created by DIFFERENT server \n"), ConsoleOutput);
	PrintNum(mv6);
}

void SetMvFromAnotherServer(){
	set1 = SetMV(mv1, 1);
	set2 = SetMV(mv6, 5);
	
	PrintNum(set1);
	PrintNum(set2);

}


/*The test should show that even though two MVs were created by different servers we can still get values of them */
void GetMVFromAnotherServer(){

	get1 = GetMV(mv1);
	get2 = GetMV(mv6);
	
	Write("should be 0 \n", sizeof("should be 1 \n"), ConsoleOutput);
	PrintNum(get1);
	Write("should be 0 \n", sizeof("should be 6 \n"), ConsoleOutput);
	PrintNum(get2);
	
	
}


int main(){

CreateMVMultipleLocks();	



GetMVFromAnotherServer();


}