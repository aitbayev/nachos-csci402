/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "../userprog/syscall.h"




int main() {

/*
Creating locks tests
*/
  int a;
  int b;
  int i;
  int d;
  int e;
/*for CV tests*/  
int a1;
int b1;
int i1;
int d1;
int e1;
 
  a = CreateLock("", 0); /*should return error saying that invalid name was passed*/
  
  PrintNum(a);
  
  b = CreateLock("abcqwertyuiopasdfghjklzxcvbnmqwertyuiopl", 40); /*Should return error: name is too long*/
  
  PrintNum(b);
  
  for(i=0; i<100; i++){ /*this should compile fine*/
  	d = CreateLock("lock", 4);
  	PrintNum(d); 
  } 
  
  e = CreateLock("lock", 4); /*This should say that lock table is full*/
  PrintNum(e);
  
/*Destroying locks tests*/  
	
	DestroyLock(-1); /* invalid argument, out of scope */
	DestroyLock(101); /* invalid argument, index greater than size of maxLockTableSize*/
  	DestroyLock(0); /* destroys lock at index location 0*/
  	DestroyLock(0); /* should say that no lock exist at given location*/
  
  	Acquire(1); /* successfully acquire lock 1*/
  	DestroyLock(1);/* try to destroy, but it doesn't destroy because it is busy */
  	Release(1); /* release lock 1  */
  	/* no addrspace check*/
  
/*Acquire lock tests*/

Acquire(-1);/* wrong argument*/
Acquire(101); /* wrong argument*/
Acquire(0); /* acquiring null lock */
Acquire(2); /* successful acquire*/

/* no addrspace check*/
  
/*Release lock tests*/  
  Release(-1); /* wrong argument*/
  Release(101); /* wrong argument*/
  Release(0); /* releasing null lock */
  Release(2);  /* successful release*/

/*Creating CVs tests*/  
a1 = CreateCondition("", 0); /*should return error saying that invalid name was passed*/

PrintNum(a1);
 
b1 = CreateCondition("abcqwertyuiopasdfghjklzxcvbnmqwertyuiopl", 40); /*Should return error: name is too long*/
 
PrintNum(b1);

  for(i1=0; i1<100; i1++){ /*this should run fine, will create 100 condition vars*/
  	d1 = CreateCondition("Condition", 9);
  	PrintNum(d1); 
  } 
  
  e1 = CreateCondition("CV", 2); /*This should say that condition table is full*/
  PrintNum(e);
  
/*Destroying condition vars system calls*/  
    DestroyCondition(-1); /* invalid argument, out of scope */
	DestroyCondition(101); /* invalid argument, index greater than size of maxLockTableSize*/
  	DestroyCondition(0); /* destroys CV at index location 0*/
  	DestroyCondition(0); /* should say that no CV exist at given location*/
  
 
  	
 return 0;
  
}
