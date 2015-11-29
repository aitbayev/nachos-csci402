#include "syscall.h"

typedef enum {busy, available, onBreak} ClerkState;
typedef enum { false, true } bool;

/*customer struct*/
struct Customer{
	char *name;
	int money; 
	int ssn;
	bool application; /*application completed?*/
	bool atAppClerk; /*whether customer went to app clerk*/
	bool atPicClerk; /*whether customer went to pic clerk*/
	bool atPassClerk; /*whether customer went to passport clerk*/
	bool atCashier; /*whether customer went to cashier*/
	bool leftOffice;
	int liked; /*keep track of pic-recursive*/
	int pass_punished; /*keep track of punishment by pass clerk- recursive*/
	int cash_punished; /*keep track of punishment by cashier- recursive*/
	int pic_liking; /*probability of liking the picture*/
	int clerk_pick; /*which clerk to go first- app or pic clerk?*/
	bool senator; /*whether the customer is a senator*/
};

struct CustomerData{
	char name[20];
	int SSN; /* social security number */
	bool application;/* whether application clerk filed the application */
	bool picture; /*whether picture clerk took a picture and filed it */
	bool verified; /*whether passport clerk verified and filed it */
	bool got_passport; /*whether customer received his/her passport from cashier */
	
};

/*application clerk struct*/
struct ApplicationClerk{
	char *name;	
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	ClerkState state;
	bool bribe; /*bribed or not*/
};

/*picture clerk struct*/
struct PictureClerk{
	char *name;
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	bool pic;
	ClerkState state;
	bool bribe; /*bribed or not*/
};

/*passport clerk struct*/
struct PassportClerk{
	char *name;
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	bool bribe; /*bribed or not*/
	ClerkState state;
};

/*cashier struct- we said that cashier cannot be bribed*/
struct Cashier{
	char *name;
	int lineCount;
	int money;
	int ssn;
	ClerkState state;
};

/*manager struct*/
struct Manager{
	char *name;
	int appClerkMoney;
	int picClerkMoney;
	int passClerkMoney;
	int cashierMoney;
	int totalMoney;
};


struct CustomerData customer_data[10];
struct Customer customers[10];
struct ApplicationClerk application_clerks[10];
struct PictureClerk picture_clerks[10];
struct PassportClerk passport_clerks[10];
struct Cashier cashiers[10];

/*Lock stuff*/

int AppClerkLock[10]; /*lock used to interact- at the register*/
int AppClerkCV[10]; /*cv used to interact- at the register*/
int AppClerkLineLock[10]; /*lock used for line*/
int AppClerkLineCV[10]; /*cv used for line*/
int PickAppClerkLineLock;  
int AppClerkBribeLineCV[10]; /*cv used for bribe line*/
int AppClerkBribeLineLock[10];
int MyLineAppClerk;
int myLineAppClerk;

int PicClerkLock[10]; /*lock used to interact- at the register*/
int PicClerkCV[10];
int PicClerkLineLock[10];
int PicClerkLineCV[10];
int PickPicClerkLineLock;
int PicClerkBribeLineCV[10];
int PicClerkBribeLineLock[10];
int MyLinePicClerk;

int PassClerkLock[10]; /*lock used to interact- at the register*/
int PassClerkCV[10]; /*cv used to interact- at the register*/
int PassClerkLineLock[10]; /*lock used for line*/
int PassClerkLineCV[10]; /*cv used for line*/
int PickPassClerkLineLock;
int PassClerkBribeLineCV[10]; /*cv used for bribe line*/
int PassClerkBribeLineLock[10];
int MyLinePassClerk;

int CashierLock[10]; /*lock used to interact- at the register*/
int CashierCV[10]; /*cv used to interact- at the register*/
int CashierLineLock[10]; /*lock used for line*/
int CashierLineCV[10]; /*cv used for line*/
int PickCashierLineLock; 
int MyLineCashier;


int customer_counter; 
int customer_index; /*index of customer in customers array, also ssn*/
void setup(){

/*App Clerk Stuff*/
	
	application_clerks[0].name = "AppClerk1";
	application_clerks[0].lineCount = 0;
  	application_clerks[0].bribeLineCount = 0;
 	application_clerks[0].state = available;
 	
 	application_clerks[1].name = "AppClerk2";	
  	application_clerks[1].lineCount = 0;
  	application_clerks[1].bribeLineCount = 0;
  	application_clerks[1].state = available;
	
	AppClerkLock[0] = CreateLock("AppClerkLock1", sizeof("AppClerkLock1"));
	AppClerkLock[1] = CreateLock("AppClerkLock2", sizeof("AppClerkLock2"));
	
	AppClerkCV[0] = CreateCondition("AppClerkCV1", sizeof("AppClerkCV1"));
	AppClerkCV[1] = CreateCondition("AppClerkCV2", sizeof("AppClerkCV2"));
	
	AppClerkLineLock[0] = CreateLock("AppClerkLineLock1", sizeof("AppClerkLineLock1"));
	AppClerkLineLock[1] = CreateLock("AppClerkLineLock2", sizeof("AppClerkLineLock2"));
	
	AppClerkLineCV[0] = CreateCondition("AppClerkLineCV1", sizeof("AppClerkLineCV1"));
	AppClerkLineCV[1] = CreateCondition("AppClerkLineCV2", sizeof("AppClerkLineCV2"));
	
	PickAppClerkLineLock = CreateLock("PicAppClerkLineLock", sizeof("PicAppClerkLineLock"));
	
	AppClerkBribeLineLock[0] = CreateLock("AppClerkBribeLineLock1", sizeof("AppClerkBribeLineLock1"));
	AppClerkBribeLineLock[1] = CreateLock("AppClerkBribeLineLock2", sizeof("AppClerkBribeLineLock2"));
	
	AppClerkBribeLineCV[0] = CreateCondition("AppClerkBribeLineCV1", sizeof("AppClerkBribeLineCV1"));
	AppClerkBribeLineCV[1] = CreateCondition("AppClerkBribeLineCV2", sizeof("AppClerkBribeLineCV2"));
	
	MyLineAppClerk = CreateMV("myLineAppClerk", sizeof("myLineAppClerk"));

/*******App Clerk Stuff********/

/*******Pic Clerk Stuff********/
	picture_clerks[0].name = "PicClerk1";
	picture_clerks[0].lineCount = 0;
  	picture_clerks[0].bribeLineCount = 0;
 	picture_clerks[0].state = available;
 	
 	picture_clerks[1].name = "PicClerk2";	
  	picture_clerks[1].lineCount = 0;
  	picture_clerks[1].bribeLineCount = 0;
  	picture_clerks[1].state = available;
	
	PicClerkLock[0] = CreateLock("PicClerkLock1", sizeof("PicClerkLock1"));
	PicClerkLock[1] = CreateLock("PicClerkLock2", sizeof("PicClerkLock2"));
	
	PicClerkCV[0] = CreateCondition("PicClerkCV1", sizeof("PicClerkCV1"));
	PicClerkCV[1] = CreateCondition("PicClerkCV2", sizeof("PicClerkCV2"));
	
	PicClerkLineLock[0] = CreateLock("PicClerkLineLock1", sizeof("PicClerkLineLock1"));
	PicClerkLineLock[1] = CreateLock("PicClerkLineLock2", sizeof("PicClerkLineLock2"));
	
	PicClerkLineCV[0] = CreateCondition("PicClerkLineCV1", sizeof("PicClerkLineCV1"));
	PicClerkLineCV[1] = CreateCondition("PicClerkLineCV2", sizeof("PicClerkLineCV2"));
	
	PickPicClerkLineLock = CreateLock("PickPicClerkLineLock", sizeof("PiclPicClerkLineLock"));
	
	PicClerkBribeLineLock[0] = CreateLock("PicClerkBribeLineLock1", sizeof("PicClerkBribeLineLock1"));
	PicClerkBribeLineLock[1] = CreateLock("PicClerkBribeLineLock2", sizeof("PicClerkBribeLineLock2"));
	
	PicClerkBribeLineCV[0] = CreateCondition("PicClerkBribeLineCV1", sizeof("PicClerkBribeLineCV1"));
	PicClerkBribeLineCV[1] = CreateCondition("PicClerkBribeLineCV2", sizeof("PicClerkBribeLineCV2"));
	
	MyLinePicClerk = CreateMV("myLinePicClerk", sizeof("myLinePicClerk"));

/*******Pic Clerk Stuff********/

/*******Pass Clerk Stuff********/
	passport_clerks[0].name = "PassClerk1";
	passport_clerks[0].lineCount = 0;
  	passport_clerks[0].bribeLineCount = 0;
 	passport_clerks[0].state = available;
 	
 	passport_clerks[1].name = "PassClerk2";	
  	passport_clerks[1].lineCount = 0;
  	passport_clerks[1].bribeLineCount = 0;
  	passport_clerks[1].state = available;
	
	PassClerkLock[0] = CreateLock("PassClerkLock1", sizeof("PassClerkLock1"));
	PassClerkLock[1] = CreateLock("PassClerkLock2", sizeof("PassClerkLock2"));
	
	PassClerkCV[0] = CreateCondition("PassClerkCV1", sizeof("PassClerkCV1"));
	PassClerkCV[1] = CreateCondition("PassClerkCV2", sizeof("PassClerkCV2"));
	
	PassClerkLineLock[0] = CreateLock("PassClerkLineLock1", sizeof("PassClerkLineLock1"));
	PassClerkLineLock[1] = CreateLock("PassClerkLineLock2", sizeof("PassClerkLineLock2"));
	
	PassClerkLineCV[0] = CreateCondition("PassClerkLineCV1", sizeof("PassClerkLineCV1"));
	PassClerkLineCV[1] = CreateCondition("PassClerkLineCV2", sizeof("PassClerkLineCV2"));
	
	PickPassClerkLineLock = CreateLock("PickPassClerkLineLock", sizeof("PickPassClerkLineLock"));
	
	PassClerkBribeLineLock[0] = CreateLock("PassClerkBribeLineLock1", sizeof("PassClerkBribeLineLock1"));
	PassClerkBribeLineLock[1] = CreateLock("PassClerkBribeLineLock2", sizeof("PassClerkBribeLineLock2"));
	
	PassClerkBribeLineCV[0] = CreateCondition("PassClerkBribeLineCV1", sizeof("PassClerkBribeLineCV1"));
	PassClerkBribeLineCV[1] = CreateCondition("PassClerkBribeLineCV2", sizeof("PassClerkBribeLineCV2"));
	
	MyLinePassClerk = CreateMV("myLinePassClerk", sizeof("myLinePassClerk"));


/*******Pass Clerk Stuff********/

/*******Cashier Clerk Stuff********/
	cashiers[0].name = "Cashier1";
	cashiers[0].lineCount = 0;
 	cashiers[0].state = available;
 	
 	cashiers[1].name = "Cashier2";	
  	cashiers[1].lineCount = 0;
  	cashiers[1].state = available;
	
	CashierLock[0] = CreateLock("CashierLock1", sizeof("CashierLock1"));
	CashierLock[1] = CreateLock("CashierLock2", sizeof("CashierLock2"));
	
	CashierCV[0] = CreateCondition("CashierCV1", sizeof("CashierCV1"));
	CashierCV[1] = CreateCondition("CashierCV2", sizeof("CashierCV2"));
	
	CashierLineLock[0] = CreateLock("CashierLineLock1", sizeof("CashierLineLock1"));
	CashierLineLock[1] = CreateLock("CashierLineLock2", sizeof("CashierLineLock2"));
	
	CashierLineCV[0] = CreateCondition("CashierLineCV1", sizeof("CashierLineCV1"));
	CashierLineCV[1] = CreateCondition("CashierLineCV2", sizeof("CashierLineCV2"));
	
	PickCashierLineLock = CreateLock("PickCashierLineLock", sizeof("PickCashierLineLock"));
	
	MyLineCashier = CreateMV("myLineCashier", sizeof("myLineCashier"));

/*******Cashier Clerk Stuff********/

/*******Customer Creation Stuf*********/
	customer_counter = CreateMV("customer_counter", sizeof("customer_counter"));
	
	customers[0].name = "customer1";
	customers[0].money = 1000;
	customers[0].ssn = 0;
	
	customers[1].name = "customer2";
	customers[1].money = 1000;
	customers[1].ssn = 1;
	
	customers[2].name = "customer3";
	customers[2].money = 1000;
	customers[2].ssn = 2;
	
	customers[3].name = "customer4";
	customers[3].money = 1000;
	customers[3].ssn = 3;
	
	customers[4].name = "customer5";
	customers[4].money = 1000;
	customers[4].ssn = 4;

	customer_index = CreateMV("customer_index", sizeof("customer_index"));
}