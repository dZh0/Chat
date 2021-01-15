#pragma once
#include <functional>
#include <sqlite3.h>

class DataBase
{
	public:
		DataBase() = default;

		DataBase(const std::string& fileName): DataBase()
		{
			Open(fileName);
		}

		DataBase(const DataBase& other) = delete;

		DataBase& operator=(const DataBase& other) = delete;
		 
		DataBase(DataBase&& other) noexcept	= default;

		DataBase& operator=(DataBase&& other) = default;

		~DataBase() = default;

		bool Open(const std::string& fileName)
		{
			auto tmp = m_db.get();
			int result = sqlite3_open(fileName.c_str(), &tmp);
			if (result)
			{
				std::cerr << "Can't open data base: " << sqlite3_errmsg(m_db.get());
				return false;
			}
			return true;
		}

		void Close()
		{
			sqlite3_close(m_db.get());
		}
	protected:
		std::unique_ptr<sqlite3, decltype(&sqlite3_close)> m_db = std::unique_ptr<sqlite3, decltype(&sqlite3_close)>(nullptr, sqlite3_close);
};