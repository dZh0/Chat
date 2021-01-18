#include <iostream>
#include "DataBase.h"

int main()
{
	DataBase db;
	db.Open("Test.db");
	db.CreateTables();
	std::string pass1 = "123";
	std::string pass2 = "merry-95";
	std::string pass3 = "godlike";
	db.NewUser(1, "Bob", std::vector<char>(pass1.begin(), pass1.end()));
	db.NewUser(2, "Merry", std::vector<char>(pass2.begin(), pass2.end()));
	db.NewUser(3, "Paul", std::vector<char>(pass3.begin(), pass3.end()));
	std::string message1 = "Hi, Merry";
	std::string message2 = "Hello, Bob";
	std::string message3 = "Wasup, Bob?";
	db.RecordMessage(1, 2, std::vector<char>(message1.begin(), message1.end()));
	db.RecordMessage(2, 1, std::vector<char>(message2.begin(), message2.end()));
	db.RecordMessage(3, 1, std::vector<char>(message3.begin(), message3.end()));
	auto a = db.GetUserPassword("Merry");
	auto b = db.GetUserPassword("Bob");
	return 0;
}