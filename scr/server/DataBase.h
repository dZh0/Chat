#pragma once
#include <functional>
#include <sqlite3.h>

class DataBase
{
	public:
		DataBase()
		{
			m_db = std::unique_ptr<sqlite3, std::function<int(sqlite3*)>> (nullptr, sqlite3_close);			//	LEGAL with decl 1
			//m_db = std::unique_ptr<sqlite3, decltype(&sqlite3_close)>(nullptr, &sqlite3_close);			//	no appropriate default constructor available
			//m_db = std::make_unique<sqlite3, std::function<int(sqlite3*)>>(nullptr, sqlite3_close);		//	binary '=': no operator found which takes a right-hand operand of type 'std::unique_ptr<sqlite3,std::default_delete<sqlite3>>' (or there is no acceptable conversion)
			//m_db = std::make_unique<sqlite3, decltype(&sqlite3_close)>(nullptr, &sqlite3_close);			//	no operator "=" matches these operands; operand types are : std::unique_ptr<sqlite3, int (*)(sqlite3*)> = std::unique_ptr<sqlite3, std::default_delete<sqlite3>>
			//m_db = std::make_unique<sqlite3>(nullptr, [=](sqlite3* ptr) { sqlite3_close(ptr); });			//	can't delete an incomplete type
			sqlite3* tmp = nullptr;
			//m_db = std::make_unique<sqlite3>(sqlite3_open("demo.db", &tmp), &sqlite3_close);				//	can't delete an incomplete type
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
		std::unique_ptr<sqlite3, std::function<int(sqlite3*)>> m_db;	// decl 1
		//std::unique_ptr<sqlite3, decltype(&sqlite3_close)> m_db;		// decl 2
		//std::unique_ptr<sqlite3> m_db;								// decl 3
};