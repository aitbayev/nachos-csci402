#include "syscall.h"

/*typedef enum {busy, available, onBreak} ClerkState; /*0 is busy, 1 is available, 2 is onbreak*/
typedef enum {false, true} bool;

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
	int application;/* whether application clerk filed the application */
	int picture; /*whether picture clerk took a picture and filed it */
	int verified; /*whether passport clerk verified and filed it */
	int got_passport; /*whether customer received his/her passport from cashier */
	
};

/*application clerk struct*/
struct ApplicationClerk{
	char *name;	
	int lineCount;
	int bribeLineCount;
	int money;
	int ssn;
	int state;
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
	int state;
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
	int state;
};

/*cashier struct- we said that cashier cannot be bribed*/
struct Cashier{
	char *name;
	int lineCount;
	int money;
	int ssn;
	int state;
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
struct Manager manager;

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
	application_clerks[0].lineCount = CreateMV("App1LineCount", sizeof("App1LineCount"));
  	application_clerks[0].bribeLineCount = CreateMV("App1BribeLineCount", sizeof("App1BribeLineCount"));;
 	application_clerks[0].state = CreateMV("App1State", sizeof("App1State"));
 	
 	application_clerks[1].name = "AppClerk2";	
  	application_clerks[1].lineCount = CreateMV("App2LineCount", sizeof("App2LineCount"));
  	application_clerks[1].bribeLineCount = CreateMV("App2BribeLineCount", sizeof("App2BribeLineCount"));;
  	application_clerks[1].state = CreateMV("App2State", sizeof("App2State"));
	
	AppClerkLock[0] = CreateLock("AppClerkLock1", sizeof("AppClerkLock1"));
	AppClerkLock[1] = CreateLock("AppClerkLock2", sizeof("AppClerkLock2"));
	
	AppClerkCV[0] = CreateCondition("AppClerkCV1", sizeof("AppClerkCV1"));
	AppClerkCV[1] = CreateCondition("AppClerkCV2", sizeof("AppClerkCV2"));
	
	AppClerkLineLock[0] = CreateLock("AppClerkLineLock1", sizeof("AppClerkLineLock1"));
	AppClerkLineLock[1] = CreateLock("AppClerkLineLock2", sizeof("AppClerkLineLock2"));
	
	AppClerkLineCV[0] = CreateCondition("AppClerkLineCV1", sizeof("AppClerkLineCV1"));
	AppClerkLineCV[1] = CreateCondition("AppClerkLineCV2", sizeof("AppClerkLineCV2"));
	
	PickAppClerkLineLock = CreateLock("PickAppClerkLineLock", sizeof("PickAppClerkLineLock"));
	
	AppClerkBribeLineLock[0] = CreateLock("AppClerkBribeLineLock1", sizeof("AppClerkBribeLineLock1"));
	AppClerkBribeLineLock[1] = CreateLock("AppClerkBribeLineLock2", sizeof("AppClerkBribeLineLock2"));
	
	AppClerkBribeLineCV[0] = CreateCondition("AppClerkBribeLineCV1", sizeof("AppClerkBribeLineCV1"));
	AppClerkBribeLineCV[1] = CreateCondition("AppClerkBribeLineCV2", sizeof("AppClerkBribeLineCV2"));
	
	MyLineAppClerk = CreateMV("myLineAppClerk", sizeof("myLineAppClerk"));

/*******App Clerk Stuff********/

/*******Pic Clerk Stuff********/
	picture_clerks[0].name = "PicClerk1";
	picture_clerks[0].lineCount = CreateMV("Pic1LineCount", sizeof("Pic1LineCount"));
  	picture_clerks[0].bribeLineCount = CreateMV("Pic1BribeLineCount", sizeof("Pic1BribeLineCount"));
 	picture_clerks[0].state = CreateMV("Pic1State", sizeof("Pic1State"));
 	
 	picture_clerks[1].name = "PicClerk2";	
  	picture_clerks[1].lineCount = CreateMV("Pic2LineCount", sizeof("Pic2LineCount"));
  	picture_clerks[1].bribeLineCount = CreateMV("Pic2BribeLineCount", sizeof("Pic2BribeLineCount"));
  	picture_clerks[1].state = CreateMV("Pic2State", sizeof("Pic2State"));
	
	PicClerkLock[0] = CreateLock("PicClerkLock1", sizeof("PicClerkLock1"));
	PicClerkLock[1] = CreateLock("PicClerkLock2", sizeof("PicClerkLock2"));
	
	PicClerkCV[0] = CreateCondition("PicClerkCV1", sizeof("PicClerkCV1"));
	PicClerkCV[1] = CreateCondition("PicClerkCV2", sizeof("PicClerkCV2"));
	
	PicClerkLineLock[0] = CreateLock("PicClerkLineLock1", sizeof("PicClerkLineLock1"));
	PicClerkLineLock[1] = CreateLock("PicClerkLineLock2", sizeof("PicClerkLineLock2"));
	
	PicClerkLineCV[0] = CreateCondition("PicClerkLineCV1", sizeof("PicClerkLineCV1"));
	PicClerkLineCV[1] = CreateCondition("PicClerkLineCV2", sizeof("PicClerkLineCV2"));
	
	PickPicClerkLineLock = CreateLock("PickPicClerkLineLock", sizeof("PickPicClerkLineLock"));
	
	PicClerkBribeLineLock[0] = CreateLock("PicClerkBribeLineLock1", sizeof("PicClerkBribeLineLock1"));
	PicClerkBribeLineLock[1] = CreateLock("PicClerkBribeLineLock2", sizeof("PicClerkBribeLineLock2"));
	
	PicClerkBribeLineCV[0] = CreateCondition("PicClerkBribeLineCV1", sizeof("PicClerkBribeLineCV1"));
	PicClerkBribeLineCV[1] = CreateCondition("PicClerkBribeLineCV2", sizeof("PicClerkBribeLineCV2"));
	
	MyLinePicClerk = CreateMV("myLinePicClerk", sizeof("myLinePicClerk"));

/*******Pic Clerk Stuff********/

/*******Pass Clerk Stuff********/
	passport_clerks[0].name = "PassClerk1";
	passport_clerks[0].lineCount = CreateMV("Pass1LineCount", sizeof("Pass1LineCount"));
  	passport_clerks[0].bribeLineCount = CreateMV("Pass1BribeLineCount", sizeof("Pass1BribeLineCount"));
 	passport_clerks[0].state = CreateMV("Pass1State", sizeof("Pass1State"));
 	
 	passport_clerks[1].name = "PassClerk2";	
  	passport_clerks[1].lineCount = CreateMV("Pass2LineCount", sizeof("Pass2LineCount"));
  	passport_clerks[1].bribeLineCount = CreateMV("Pass2BribeLineCount", sizeof("Pass2BribeLineCount"));
  	passport_clerks[1].state = CreateMV("Pass2State", sizeof("Pass2State"));
	
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

/*******Cashier Stuff********/
	cashiers[0].name = "Cashier1";
	cashiers[0].lineCount = CreateMV("Cashier1LineCount", sizeof("Cashier1LineCount"));
 	cashiers[0].state = CreateMV("Cashier1State", sizeof("Cashier1State"));
 	
 	cashiers[1].name = "Cashier2";	
  	cashiers[1].lineCount = CreateMV("Cashier2LineCount", sizeof("Cashier2LineCount"));
  	cashiers[1].state = CreateMV("Cashier2State", sizeof("Cashier2State"));
	
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

/*******Cashier Stuff********/

/*******Manager Stuff********/
	manager.name = "Manager";
	manager.appClerkMoney = CreateMV("managerAppClerkMoney", sizeof("managerAppClerkMoney"));
	manager.picClerkMoney = CreateMV("managerPicClerkMoney", sizeof("managerPicClerkMoney"));
	manager.passClerkMoney = CreateMV("managerPassClerkMoney", sizeof("managerPassClerkMoney"));
	manager.cashierMoney = CreateMV("managerCashierMoney", sizeof("managerCashierMoney"));
	manager.totalMoney = CreateMV("managerTotalMoney", sizeof("managerTotalMoney"));

/*******Manager Stuff********/

/*******Customer Creation Stuf*********/
	customer_counter = CreateMV("customer_counter", sizeof("customer_counter"));
	
	customer_data[0].SSN = CreateMV("CustomerData1SSN", sizeof("CustomerData1SSN"));
	customer_data[0].application = CreateMV("CustomerData1App", sizeof("CustomerData1App"));
	customer_data[0].picture = CreateMV("CustomerData1Pic", sizeof("CustomerData1Pic"));
	customer_data[0].verified = CreateMV("CustomerData1Ver", sizeof("CustomerData1Ver"));
	customer_data[0].got_passport = CreateMV("CustomerData1GotPassport", sizeof("CustomerData1GotPassport"));
	
	customer_data[1].SSN = CreateMV("CustomerData2SSN", sizeof("CustomerData2SSN"));
	customer_data[1].application = CreateMV("CustomerData2App", sizeof("CustomerData2App"));
	customer_data[1].picture = CreateMV("CustomerData2Pic", sizeof("CustomerData2Pic"));
	customer_data[1].verified = CreateMV("CustomerData2Ver", sizeof("CustomerData2Ver"));
	customer_data[1].got_passport = CreateMV("CustomerData2GotPassport", sizeof("CustomerData2GotPassport"));
	
	customer_data[2].SSN = CreateMV("CustomerData3SSN", sizeof("CustomerData3SSN"));
	customer_data[2].application = CreateMV("CustomerData3App", sizeof("CustomerData3App"));
	customer_data[2].picture = CreateMV("CustomerData1Pic", sizeof("CustomerData3Pic"));
	customer_data[2].verified = CreateMV("CustomerData3Ver", sizeof("CustomerData3Ver"));
	customer_data[2].got_passport = CreateMV("CustomerData3GotPassport", sizeof("CustomerData3GotPassport"));
	
	customer_data[3].SSN = CreateMV("CustomerData4SSN", sizeof("CustomerData4SSN"));
	customer_data[3].application = CreateMV("CustomerData4App", sizeof("CustomerData4App"));
	customer_data[3].picture = CreateMV("CustomerData4Pic", sizeof("CustomerData4Pic"));
	customer_data[3].verified = CreateMV("CustomerData4Ver", sizeof("CustomerData4Ver"));
	customer_data[3].got_passport = CreateMV("CustomerData4GotPassport", sizeof("CustomerData4GotPassport"));
	
	customer_data[4].SSN = CreateMV("CustomerData5SSN", sizeof("CustomerData5SSN"));
	customer_data[4].application = CreateMV("CustomerData5App", sizeof("CustomerData5App"));
	customer_data[4].picture = CreateMV("CustomerData5Pic", sizeof("CustomerData5Pic"));
	customer_data[4].verified = CreateMV("CustomerData5Ver", sizeof("CustomerData5Ver"));
	customer_data[4].got_passport = CreateMV("CustomerData5GotPassport", sizeof("CustomerData5GotPassport"));
	
	customer_data[5].SSN = CreateMV("CustomerData6SSN", sizeof("CustomerData6SSN"));
	customer_data[5].application = CreateMV("CustomerData6App", sizeof("CustomerData6App"));
	customer_data[5].picture = CreateMV("CustomerData6Pic", sizeof("CustomerData6Pic"));
	customer_data[5].verified = CreateMV("CustomerData6Ver", sizeof("CustomerData6Ver"));
	customer_data[5].got_passport = CreateMV("CustomerData6GotPassport", sizeof("CustomerData6GotPassport"));
	
	customer_data[6].SSN = CreateMV("CustomerData7SSN", sizeof("CustomerData7SSN"));
	customer_data[6].application = CreateMV("CustomerData7App", sizeof("CustomerData7App"));
	customer_data[6].picture = CreateMV("CustomerData7Pic", sizeof("CustomerData7Pic"));
	customer_data[6].verified = CreateMV("CustomerData7Ver", sizeof("CustomerData7Ver"));
	customer_data[6].got_passport = CreateMV("CustomerData7GotPassport", sizeof("CustomerData7GotPassport"));
	
	customer_data[7].SSN = CreateMV("CustomerData8SSN", sizeof("CustomerData8SSN"));
	customer_data[7].application = CreateMV("CustomerData8App", sizeof("CustomerData8App"));
	customer_data[7].picture = CreateMV("CustomerData8Pic", sizeof("CustomerData8Pic"));
	customer_data[7].verified = CreateMV("CustomerData8Ver", sizeof("CustomerData8Ver"));
	customer_data[7].got_passport = CreateMV("CustomerData8GotPassport", sizeof("CustomerData8GotPassport"));
	
	customer_data[8].SSN = CreateMV("CustomerData9SSN", sizeof("CustomerData9SSN"));
	customer_data[8].application = CreateMV("CustomerData9App", sizeof("CustomerData9App"));
	customer_data[8].picture = CreateMV("CustomerData9Pic", sizeof("CustomerData9Pic"));
	customer_data[8].verified = CreateMV("CustomerData9Ver", sizeof("CustomerData9Ver"));
	customer_data[8].got_passport = CreateMV("CustomerData9GotPassport", sizeof("CustomerData9GotPassport"));

	customers[0].name = "customer1";
	customers[0].money = 1600;
	customers[0].ssn = 0;
	
	customers[1].name = "customer2";
	customers[1].money = 1600;
	customers[1].ssn = 1;
	
	customers[2].name = "customer3";
	customers[2].money = 1600;
	customers[2].ssn = 2;
	
	customers[3].name = "customer4";
	customers[3].money = 1600;
	customers[3].ssn = 3;
	
	customers[4].name = "customer5";
	customers[4].money = 1600;
	customers[4].ssn = 4;
	
	customers[5].name = "customer6";
	customers[5].money = 1600;
	customers[5].ssn = 5;
	
	customers[6].name = "customer7";
	customers[6].money = 1600;
	customers[6].ssn = 6;
	
	customers[7].name = "customer8";
	customers[7].money = 1600;
	customers[7].ssn = 7;
	
	customers[8].name = "customer9";
	customers[8].money = 1600;
	customers[8].ssn = 8;
	
	customers[9].name = "customer10";
	customers[9].money = 1600;
	customers[9].ssn = 9;

	customer_index = CreateMV("customer_index", sizeof("customer_index"));
}