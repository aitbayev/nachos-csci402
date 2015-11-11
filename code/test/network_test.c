#include "syscall.h"
	int mv1;
	int mv2;
	int set;
	int get;
	int removeMV1;
	
	int lk1;
	int lk2;
	int lk3;
	int lk4;
	int lk5;
	
	int cv1;
	int cv2;
	int cv3;
	int cv4;


/******** MV********/	
/* Returns -1 if fails */

/* Successfully creating MV should return index of MV*/
void CreateMVSuccessfull(){
	mv1 = CreateMV("MV", sizeof("MV"));
	PrintNum(mv1);
}
/*Create MV with the same name, should return -1*/
void CreateMVSameName(){
	mv2 = CreateMV("MV", sizeof("MV"));
	PrintNum(mv2);
}

/*Create VM with too long name, should return -1*/

void CreateMVLongName(){
	mv2 = CreateMV("asdfghjklqwertyuiopzxcvbnmwzexrctfvgybuhnjert", sizeof("asdfghjklqwertyuiopzxcvbnmwzexrctfvgybuhnjert"));
	PrintNum(mv2);
}

/*Successfully Destroy MV*/

void DestroyMVSuccessfull(){
	mv2 = DestroyMV(mv1);
	PrintNum(mv2);
}

/*Destroy MV that doesn't exist, wasn't able to destroy return -1*/
void DestroyMVDoesntExist(){
	mv2 = DestroyMV(mv1);
	PrintNum(mv2);
}

void DestroyMVInvalidArgument(){
	mv2 = DestroyMV(-2);
	PrintNum(mv2);
}

/*Successfully set Value of MV */

void SetMVSuccessfull(){
	mv1 = CreateMV("MV", sizeof("MV"));
	set = SetMV(mv1, 5);
	get = GetMV(mv1);
}

/*Pass wrong argument to SetMV, should return -1*/
void SetMVWrongArguement(){
	set = SetMV(-2, 5);
	PrintNum(set);
}

/*Set MV that doesn't exist should return -1*/
void SetMVDoesntExist(){
	set = SetMV(2, 5);
	PrintNum(set);
}

/* GetMV Successful */
void GetMVSuccessful(){
	get = GetMV(mv1);
	PrintNum(get);
}

/* GetMV wrong argument */
void GetMVWrongArgument(){
	get = GetMV(-2);
	PrintNum(get);
}

/* GetMV MV doesn't exists*/
void GetMVDoesntExist(){
	get = GetMV(2);
	PrintNum(get);
}

/******** LOCK ********/	
/* Successfully creating Lock */
void CreateLockSuccessfull(){
	lk1 = CreateLock("Lock", sizeof("Lock"));
	PrintNum(lk1);
}

void CreateLockSameName(){
	lk2 = CreateLock("Lock", sizeof("Lock"));
	PrintNum(lk2);
}

void CreateLockLongName(){
	lk2 = CreateLock("asdfghjklqwertyuiopzxcvbnmwzexrctfvgybuhnjert", sizeof("asdfghjklqwertyuiopzxcvbnmwzexrctfvgybuhnjert"));
	PrintNum(lk2);
}

/*Successfully destroying lock*/
void DestroyLockSuccessfull(){
	DestroyLock(lk1);
}

/*Destroy Lock that doesn't exist*/
void DestroyLockDoesntExist(){
	DestroyLock(3);
	
}
/*Destroy Lock invalid arg */
void DestroyLockInvalidArgument(){
	 DestroyLock(-2);
}

/*Successfully Acquire and Release*/
void AcquireReleaseSuccessfull(){
	lk1 = CreateLock("Lk1", sizeof("Lk1"));
	Acquire(lk1);
	Release(lk1);
}
/*Acquire and Release locks passing wrong argument*/
void AcquireReleaseWrongArgument(){
	Acquire(-2);
	Release(-2);
}
/*Acquire and release lock that doesn't exist*/
void AcquireReleaseDoesntExist(){
	DestroyLock(1);
	Acquire(1);
	Release(1);
}
/*Release lock without acquiring it*/
void ReleaseWithoutAcquire(){
	lk3 = CreateLock("lk3", sizeof("lk3"));
	Release(lk3);
}

/*Acquire lock twice, should give an error*/
void AcquireLockTwice(){
	Acquire(lk3);
	Acquire(lk3);	
}
/*Try to destroy lock that is being used, then release should delete it*/
void AcquireDestroyRelease(){
	lk4 = CreateLock("lk4", sizeof("lk4"));
	Acquire(lk4);
	DestroyLock(lk4);
	Release(lk4);
}

/******** CV ********/	
/* Successfully creating CV */
void CreateCVSuccessfull(){
	cv1 = CreateCondition("CV", sizeof("CV"));
	PrintNum(cv1);
}

void CreateCVSameName(){
	cv2 = CreateCondition("CV", sizeof("CV"));
	PrintNum(cv2);
}

void CreateCVLongName(){
	cv2 = CreateCondition("asdfghjklqwertyuiopzxcvbnmwzexrctfvgybuhnjert", sizeof("asdfghjklqwertyuiopzxcvbnmwzexrctfvgybuhnjert"));
	PrintNum(cv2);
}

/*Successfully destroying CV*/
void DestroyCVSuccessfull(){
	DestroyCondition(cv1);
}

/*Destroy CV that doesn't exist*/
void DestroyCVDoesntExist(){
	DestroyCondition(3);
	
}
/*Destroy CV invalid arg */
void DestroyCVInvalidArgument(){
	 DestroyCondition(-2);
}

/*Testing Wait and Signal syscall*/

void WaitSignalSuccessfull(){
	lk5 = CreateLock("LK5", sizeof("LK5"));
	cv2 = CreateCondition("CV2", sizeof("CV2"));
	Acquire(lk5);
	Wait(lk5, cv2);
	Signal(cv2, lk5);
	Release(lk5);
}

/* Passing bad arguments to Wait and Signal*/

void WaitSignalBadArguments(){
	Wait(10, 10);
	Signal(10, 10);
}

/*Pass wrong lock to Wait*/
void WaitWrongLock(){
	Wait(lk4,cv2);
}
/*Signalling wrong lock */
void SignalWrongLock(){
	Wait(lk4, cv2);
	Signal(lk3, cv2);
}

/*Broadcast Successfully*/

void BroadcastSuccessfully(){
	Acquire(lk5);
	Wait(lk5, cv2);
	Broadcast(lk5, cv2);
}

/*Broadcast bad arguments*/
void BroadcastBadArguments(){
	Broadcast(10, 10);
}
int main(){	
/********** MV *************/	

CreateMVSuccessfull();	
CreateMVSameName();
CreateMVLongName();

DestroyMVSuccessfull();
DestroyMVDoesntExist();
DestroyMVInvalidArgument();

SetMVSuccessfull();
SetMVWrongArguement();
SetMVDoesntExist();

GetMVSuccessful();
GetMVDoesntExist();
GetMVWrongArgument();

/******** MV TEST ENDS ********/	
	
/*************LOCK*****************/	
CreateLockSuccessfull();
CreateLockSameName();
CreateLockLongName();

DestroyLockSuccessfull();
DestroyLockDoesntExist();
DestroyLockInvalidArgument();

AcquireReleaseSuccessfull();
AcquireReleaseWrongArgument();	
AcquireReleaseDoesntExist(); 

ReleaseWithoutAcquire();
AcquireLockTwice();
AcquireDestroyRelease();

/******** LOCK TEST ENDS ********/

	
/*************CV*****************/	
CreateCVSuccessfull();
CreateCVSameName();
CreateCVLongName();

DestroyCVSuccessfull();
DestroyCVDoesntExist();
DestroyCVInvalidArgument();

WaitSignalSuccessfull();
WaitSignalBadArguments();
WaitWrongLock();
SignalWrongLock();

BroadcastSuccessfully();
BroadcastBadArguments();


/******** CV TEST ENDS ********/	

Exit(0);
	
return 0;

}