#pragma once
#include <sqlite3.h>

// Wrapper class for data base linkage interface
class DataBase
{
	public:
		// Default constructor
		DataBase()
		{
			sqlite3* ptr = db.get();	//@ METO Seems silly but "sqlite3_open("database.db", &db.get())" fails to compile...
			if (sqlite3_open("database.db", &ptr))
			{
				throw std::runtime_error(sqlite3_errmsg(db.get()));
			}
		};
/*
		// Copy constuctor
		DataBase(const DataBase& other)	: db(other.db)
		{};

		// Copy assigment
		DataBase& operator=(const DataBase& other)
		{
			if (this == &other)
			{
				return *this;
			}
			db = other.db;
		}
		 
		// Move constuctor
		DataBase(DataBase&& other) noexcept	: db(std::exchange(other.db, nullptr))
		{};

		// Move assigment
		DataBase& operator=(const DataBase&& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}
			sqlite3_close(db.get());
			db = std::exchange(other.db, nullptr);
			return *this;
		}
*/
		// Destructor
		~DataBase()
		{
			if(db)
			{
				sqlite3_close(db.get());
			}
		};

	private:
		std::shared_ptr<sqlite3> db;
};