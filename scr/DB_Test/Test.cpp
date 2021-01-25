#include <iostream>
#include <sqlite3.h>
#include "DataBase2.h"

int main()
{
/*
	DataBase db;
	if (db.Open("Test.db"))
	{
		db.CreateTables();
	}
	
//	db.NewUser(1, "Bob", std::vector<char>(pass1.begin(), pass1.end()));
//	db.NewUser(2, "Merry", std::vector<char>(pass2.begin(), pass2.end()));
//	db.NewUser(3, "Paul", std::vector<char>(pass3.begin(), pass3.end()));
	std::string message1 = "Hi, Merry";
	std::string message2 = "Hello, Bob";
	std::string message3 = "Wasup, Bob?";
//	db.RecordMessage(1, 2, std::vector<char>(message1.begin(), message1.end()));
//	db.RecordMessage(2, 1, std::vector<char>(message2.begin(), message2.end()));
//	db.RecordMessage(3, 1, std::vector<char>(message3.begin(), message3.end()));
//	auto a = db.GetUserPassword("Merry");
//	auto b = db.GetUserPassword("Bob");
*/
	DataBase db;
	if (!db.Open("Test.db"))
	{
		return EXIT_FAILURE;
	}
	db.CreateTables();
	//////////////////////////////
	// User tests
	//////////////////////////////
	std::string password1 = "123";
	std::string password2 = "kokiche";
	std::string password3 = "roza";

	auto write1 = db.RecordUser("bob@bob.bob", std::vector<char>(password1.begin(), password1.end()));
	auto write2 = db.RecordUser("merry@merry.mer", std::vector<char>(password2.begin(), password2.end()));
	auto write3 = db.RecordUser("paul@paul.pl", std::vector<char>(password3.begin(), password3.end()));

	auto read1 = db.GetUserId("bob@bob.bob", std::vector<char>(password2.begin(), password2.end()));
	auto read2 = db.GetUserId("x@x.x", std::vector<char>(password2.begin(), password2.end()));
	auto read3 = db.GetUserId("bob@bob.bob", std::vector<char>(password1.begin(), password1.end()));

	auto del = db.RemoveUser("bob@bob.bob");
	//////////////////////////////
	// Message test
	//////////////////////////////
	std::string message1 = "Hi, Merry";
	std::string message2 = "Hello, Bob";
	std::string message3 = "Wasup, Bob?";

	db.RecordMessage(1, 2, std::vector<char>(message1.begin(), message1.end()));
	db.RecordMessage(2, 1, std::vector<char>(message2.begin(), message2.end()));
	db.RecordMessage(3, 1, std::vector<char>(message3.begin(), message3.end()));

	return 0;
}