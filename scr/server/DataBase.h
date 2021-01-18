#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>

class DataBase
{
	public:
		DataBase()
		{
			std::cout << "Create DataBase handle . . .\n";
		}

		DataBase(const std::string& fileName): DataBase()
		{
			Open(fileName);
		}

		DataBase(const DataBase& other) = delete;

		DataBase& operator=(const DataBase& other) = delete;
		 
		DataBase(DataBase&& other) noexcept	= default;

		DataBase& operator=(DataBase&& other) = default;

		~DataBase()
		{
			Close();
			std::cout << "Delete DataBase handle . . .\n";
		};
		
		bool Open(const std::string& fileName)
		{
			sqlite3* tmp = nullptr;
			int result = sqlite3_open_v2(fileName.c_str(), &tmp, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
			if (result != SQLITE_OK)
			{
				std::cerr << "Can't open data base: " << sqlite3_errmsg(tmp);
				sqlite3_close_v2(tmp);
				return false;
			}
			m_db.reset(tmp);
			return true;
		}

		void Close()
		{
			m_db.release();
		}

		void CreateTables()
		{
			// Note:	"password" and "message" columns are set to "blob" and should NEVER be readable.
			constexpr auto sql =
				"CREATE TABLE users("\
				"ID				integer		PRIMARY KEY					NOT NULL,"\
				"name			text		UNIQUE						NOT NULL,"\
				"password		blob									NOT	NULL);"\

				"CREATE TABLE messages("\
				"ID				integer		PRIMARY KEY					NOT NULL,"\
				"time			integer		DEFAULT CURRENT_TIMESTAMP	NOT NULL,"\
				"from_user		integer		REFERENCES users ON DELETE SET NULL,"\
				"target			integer,"\
				"message		blob);";

			char* errMsg;
			int result = sqlite3_exec(m_db.get(), sql, nullptr, 0, &errMsg);
			if (result != SQLITE_OK)
			{
				std::cerr << "SQL error: " << errMsg << "\n";
			}
		}

		bool NewUser(int id, const std::string &name, const std::vector<char> &password)
		{
			const std::string sql = "INSERT INTO users (ID, name, password) VALUES(" + std::to_string(id) + ", '" + name + "' , ' " + "dummy password" + "');"; //TODO: figgure out how to record the password
			char* errMsg;
			int result = sqlite3_exec(m_db.get(), sql.c_str(), nullptr, 0, &errMsg);
			if (result != SQLITE_OK)
			{
				if (result == SQLITE_CONSTRAINT)
				{
					std::cout << "User " << name << " alredy exists!\n";
				}
				else
				{
					std::cerr << "SQL error: " << errMsg << "\n";
				}
				return false;
			}
			return true;
		}

		bool RecordMessage(int userId, int targetId, const std::vector<char>& password)
		{
			const std::string sql =
			"INSERT INTO messages (from_user, target, message) VALUES(" +
				std::to_string(userId) + ", '" +
				std::to_string(targetId) + "' , ' " +
				"dummy message" +			//TODO: figgure out how to record the message
			"');";
			char* errMsg;
			int result = sqlite3_exec(m_db.get(), sql.c_str(), nullptr, 0, &errMsg);
			if (result != SQLITE_OK)
			{
				std::cerr << "SQL error: " << errMsg <<"\n";
				return false;
			}
			return true;
		}

		std::vector<char> GetUserPassword(const std::string& name)
		{
			std::vector<char> password;
			constexpr auto query = "SELECT password FROM users WHERE name = ? LIMIT 1;";
			sqlite3_stmt* stmt = nullptr;
			auto sqlResult = sqlite3_prepare_v3(m_db.get(), query, -1, 0, &stmt, 0);
			if(sqlResult != SQLITE_OK)
			{
				std::cerr << "SQL prepare error: " << sqlite3_errstr(sqlResult) << "\n";
				return password;
			}
			sqlResult = sqlite3_bind_text(stmt, 1, name.c_str(), name.size(), SQLITE_STATIC);
			if(sqlResult != SQLITE_OK)
			{
				std::cerr << "SQL binding error: " << sqlite3_errstr(sqlResult) << "\n";
				return password;
			}
			if(sqlite3_step(stmt) == SQLITE_ROW)
			{
				auto size = sqlite3_column_bytes(stmt, 0);
				if(size > 1)
				{
					auto buffer = (char*)sqlite3_column_blob(stmt, 0);
					password = std::vector<char>(buffer+1, buffer + size);
				}
			};
			sqlResult = sqlite3_finalize(stmt);
			if (sqlResult != SQLITE_OK)
			{
				std::cerr << "SQL inalization error: " << sqlite3_errstr(sqlResult) << "\n";
			}
			return password;
		}
/*
		std::vector<char> GetMessages(int target, unsigned daysAgo = 0)
		{
			int times
			if (daysAgo == 0)
			{

			}
			constexpr auto query = "SELECT * FROM messages WHERE target = ?;";
			sqlite3_stmt* stmt = nullptr;
			sqlite3_prepare_v3(m_db.get(), query, -1, 0, &stmt, 0);
			sqlite3_bind_int(stmt, 1, target);
			std::vector<std::tuple<>> password(0);
			if (sqlite3_step(stmt) == SQLITE_ROW)
			{
				auto size = sqlite3_column_bytes(stmt, 0);
				char* buffer = new char[size];
				buffer = (char*)sqlite3_column_blob(stmt, 0);
				password = std::vector<char>(buffer + 1, buffer + size);
				delete[] buffer;
			};
			sqlite3_finalize(stmt);
			return password;
		}
*/
	protected:
		std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> m_db = std::unique_ptr < sqlite3, decltype(&sqlite3_close_v2)> (nullptr, &sqlite3_close);
};