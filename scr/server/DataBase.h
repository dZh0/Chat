#pragma once
#include <functional>
#include <sqlite3.h>

class DataBase
{
	public:
		DataBase()
		{
			sqlite3* tmp = nullptr;
			m_db = std::unique_ptr<sqlite3, std::function<void(sqlite3*)>> (tmp, sqlite3_close);
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
		};
		
		bool Open(const std::string& fileName)
		{
			sqlite3* tmp = nullptr;
			int result = sqlite3_open(fileName.c_str(), &tmp);
			if (result)
			{
				std::cerr << "Can't open data base: " << sqlite3_errmsg(tmp);
				return false;
			}
			m_db.reset(tmp);
			return true;
		}

		void Close()
		{
			m_db.release();
		}

	protected:
		std::unique_ptr<sqlite3, std::function<void(sqlite3*)>> m_db;
};