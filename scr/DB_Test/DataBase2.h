#pragma once
#include <iostream>
#include <optional>
#include <vector>
#include <sqlite3.h>

using clientId = int;

struct msg
{
	float time = 0.f;
	clientId from = 0;
	clientId to = 0;
	std::vector<char> message = {};
};

class DataBase
{
public:
	DataBase() = default;
	DataBase(const std::string& fileName);
	bool Open(const std::string& fileName);
	void Close();
	bool CreateTables();

	std::optional<clientId> RecordUser(const std::string &account, const std::vector<char> &password, const std::string &name = "");
	bool RecordMessage(clientId user, clientId target, const std::vector<char> &message);
	std::optional<clientId> GetUserId(const std::string &account, const std::vector<char> &password);
	bool GetMessagesToTarget(clientId user, float daysAgo = 0.f);
	bool RemoveUser(const std::string& account);
	bool RemoveMessage();
private:
	std::shared_ptr<sqlite3> _db;
};

DataBase::DataBase(const std::string& fileName)
{
	Open(fileName);
}

bool DataBase::Open(const std::string& fileName)
{
	sqlite3* tmp = nullptr;
	auto result = sqlite3_open_v2(fileName.c_str(), &tmp, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::Opens(): Error opening/creating " << fileName << ". Check if the file is write protected.\n";
		return false;
	}
	_db = std::shared_ptr<sqlite3>(tmp, [=](sqlite3* ptr) { sqlite3_close_v2(ptr); });
	return true;
}

void DataBase::Close()
{
	if(_db.use_count() < 2)
	{
		sqlite3_close(_db.get());
	}
}

bool DataBase::CreateTables()
{
	// Note:	"password" and "message" columns are set to "blob" and should NEVER be readable.
	constexpr auto sql =
		"CREATE TABLE users("\
		"ID				INTEGER		PRIMARY KEY,"\
		"account		TEXT		UNIQUE						NOT NULL,"\
		"name			TEXT									NOT NULL,"\
		"password		BLOB									NOT	NULL);"\

		"CREATE TABLE messages("\
		"ID				INTEGER		PRIMARY KEY,"\
		"msg_time		REAL		DEFAULT CURRENT_TIMESTAMP	NOT NULL,"\
		"from_user		INTEGER		REFERENCES users ON DELETE SET NULL,"\
		"target			INTEGER,"\
		"message		BLOB);";
	int result = sqlite3_exec(_db.get(), sql, nullptr, nullptr, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::CreateTables(): " << sqlite3_errmsg(_db.get()) << "\n";
	}
	return result == SQLITE_OK;
}

// Records a new user in the data base. Returns the ID of the user or std::nullopt if the recording fails;
std::optional<clientId> DataBase::RecordUser(const std::string &account, const std::vector<char> &password, const std::string &name)
{
	constexpr auto sql = "INSERT INTO users (account, name, password) VALUES(?, ?, ?);";
	sqlite3_stmt* stmt = nullptr;

	auto result = sqlite3_prepare_v2(_db.get(), sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::RecordUser(): " << sqlite3_errmsg(_db.get()) << ")\n";
		return std::nullopt;
	}

	const std::string &nameRef = (name == "")? account: name;

	sqlite3_bind_text(stmt, 1, account.c_str(), account.size(), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, nameRef.c_str(), nameRef.size(), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 3, password.data(), password.size(), SQLITE_STATIC);
	sqlite3_step(stmt);
	
	auto id = sqlite3_last_insert_rowid(_db.get());
	if (id > std::numeric_limits<clientId>::max())
	{
		std::cerr << "DataBase::RecordUser(): user ID is too large\n";
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	sqlite3_finalize(stmt);
	return static_cast<clientId>(id);
}

bool DataBase::RecordMessage(clientId user, clientId target, const std::vector<char>& message)
{
	constexpr auto sql = "INSERT INTO messages (from_user, target, message) VALUES(?, ?, ?);";
	sqlite3_stmt* stmt = nullptr;

	auto result = sqlite3_prepare_v2(_db.get(), sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::RecordMessage(): " << sqlite3_errmsg(_db.get()) << ")\n";
		return false;
	}

	sqlite3_bind_int(stmt, 1, user);
	sqlite3_bind_int(stmt, 2, target);
	sqlite3_bind_blob(stmt, 3, message.data(), message.size(), SQLITE_STATIC);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return true;
}

// Retrieves the user ID from the data base. Returns std::nullopt if the retreaval fails ot the provided password is incorrect;
std::optional<clientId> DataBase::GetUserId(const std::string& account, const std::vector<char>& password)
{
	constexpr auto sql = "SELECT ID, password FROM users WHERE account = ? LIMIT 1;";
	sqlite3_stmt* stmt = nullptr;

	auto result = sqlite3_prepare_v2(_db.get(), sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::GetUserId(): "<< sqlite3_errmsg(_db.get()) << ")\n";
		return std::nullopt;
	}

	sqlite3_bind_text(stmt, 1, account.c_str(), static_cast<int>(account.size()), SQLITE_STATIC);

	result = sqlite3_step(stmt);
	if (result != SQLITE_ROW)
	{
		std::cout << "DataBase::GetUserId(): Account " << account << " not found\n";
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	auto buffer = static_cast<const char*>(sqlite3_column_blob(stmt, 1));
	auto buffer_size = sqlite3_column_bytes(stmt, 1);
	std::vector<char> retrivedPassword(buffer, buffer + buffer_size); //@METO: Does this make unnecesary copy of the buffer and how do I avoid it?
	if (password != retrivedPassword) 
	{
		std::cerr << "DataBase::GetUserId(): Account " << account << " password incorrect\n";
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	auto id = sqlite3_column_int(stmt, 0);
	if (id > std::numeric_limits<clientId>::max())
	{
		std::cerr << "DataBase::GetUserId(): user ID is too large\n";
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	sqlite3_finalize(stmt);
	return static_cast<clientId>(id);
}

// Retrieves all messages to the specidied target. If daysAgo > 0 it limits to messages daysAgo from now.
bool DataBase::GetMessagesToTarget(clientId user, float daysAgo)
{
	constexpr auto sql = "SELECT * FROM messages WHERE account = ? ;";
	sqlite3_stmt* stmt = nullptr;
	/*
	auto result = sqlite3_prepare_v2(_db.get(), sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::GetUserId(): " << sqlite3_errmsg(_db.get()) << ")\n";
		return std::nullopt;
	}

	sqlite3_bind_text(stmt, 1, account.c_str(), static_cast<int>(account.size()), SQLITE_STATIC);

	result = sqlite3_step(stmt);
	if (result != SQLITE_ROW)
	{
		std::cout << "DataBase::GetUserId(): Account " << account << " not found\n";
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	auto buffer = static_cast<const char*>(sqlite3_column_blob(stmt, 1));
	auto buffer_size = sqlite3_column_bytes(stmt, 1);
	std::vector<char> retrivedPassword(buffer, buffer + buffer_size); //@METO: Does this make unnecesary copy of the buffer and how do I avoid it?
	if (password != retrivedPassword)
	{
		std::cerr << "DataBase::GetUserId(): Account " << account << " password incorrect\n";
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	auto id = sqlite3_column_int(stmt, 0);
	if (id > std::numeric_limits<clientId>::max())
	{
		std::cerr << "DataBase::GetUserId(): user ID is too large\n";
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	sqlite3_finalize(stmt);*/
	return true;
}

// Removes user information from a given account. Returns false if the removal fails or the account is not found.
bool DataBase::RemoveUser(const std::string& account)
{
	constexpr auto sql = "DELETE FROM users WHERE account = ?";
	sqlite3_stmt* stmt = nullptr;

	auto result = sqlite3_prepare_v2(_db.get(), sql, -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::GetUserId(): " << sqlite3_errmsg(_db.get()) << ")\n";
		return false;
	}

	sqlite3_bind_text(stmt, 1, account.c_str(), static_cast<int>(account.size()), SQLITE_STATIC);
	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE)
	{
		std::cerr << "DataBase::RemoveUser(): Account " << account << " not found\n";
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}
bool DataBase::RemoveMessage()
{
	return true;
}