#include <iostream>
#include <sqlite3.h>
#include "DataBase.h"
namespace	// SQLite3 free helper functions
{
	using unique_stmt_ptr = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;
	// Wrapper of sqlite3_prepare_v2()
	sqlite3_stmt* prepare(sqlite3* db, const std::string& sql)
	{
		sqlite3_stmt* stmt = nullptr;
		auto result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr); //@METO: Not sure what "const char **pzTail" is for in sqlite3_prepare_v2().
		if (result != SQLITE_OK)
		{
			std::cerr << "DataBase::_prepare(): " << sqlite3_errmsg(db) << "\n" << sql << "\n";
			return nullptr;
		}
		return stmt;
	}
	// Wrapper of sqlite3_bind_text()
	void bind(sqlite3_stmt* stmt, const std::string& text, int idx = 1) 
	{
		int result;
		if (text == "")
		{
			result = sqlite3_bind_null(stmt, idx);
		}
		else
		{
			result = sqlite3_bind_text(stmt, idx, text.c_str(), text.size(), SQLITE_STATIC);
		}
		if (result != SQLITE_OK)
		{
			std::cerr << "SQLite binding failed" << sqlite3_errmsg(sqlite3_db_handle(stmt)) << "\n";
		}
	}
	// Wrapper of sqlite3_bind_blob()
	void bind(sqlite3_stmt* stmt, const binaryData_t& data, int idx = 1) 
	{
		if (sqlite3_bind_blob(stmt, idx, data.data(), data.size(), SQLITE_STATIC) != SQLITE_OK)
		{
			std::cerr << "SQLite binding failed" << sqlite3_errmsg(sqlite3_db_handle(stmt)) << "\n";;
		}
	}
	// Wrapper of sqlite3_bind_int()
	void bind(sqlite3_stmt* stmt, int number, int idx = 1) 
	{
		if (sqlite3_bind_int(stmt, idx, number) != SQLITE_OK)
		{
			std::cerr << "SQLite binding failed" << sqlite3_errmsg(sqlite3_db_handle(stmt)) << "\n";
		}
	}
	// Wrapper of sqlite3_bind_double()
	void bind(sqlite3_stmt* stmt, double number, int idx = 1) 
	{
		if (sqlite3_bind_double(stmt, idx, number) != SQLITE_OK)
		{
			std::cerr << "SQLite binding failed" << sqlite3_errmsg(sqlite3_db_handle(stmt)) << "\n";
		}
	}

	template<typename int... ints, typename... Ts>
	void bindEnum(sqlite3_stmt* stmt, std::integer_sequence<int, ints...> int_seq, const Ts&... args)
	{
		(bind(stmt, args, ints + 1), ...);
	}

	template <typename... Ts>
	void multyBind(sqlite3_stmt* stmt, const Ts&... args)
	{
		bindEnum(stmt, std::make_integer_sequence<int, sizeof...(args)>{}, args...);
	}

	template <typename... Ts>
	bool exec(sqlite3* db, const std::string  &sql, const Ts&... args)
	{
		unique_stmt_ptr stmt(prepare(db, sql), sqlite3_finalize);
		if constexpr (sizeof...(args) == 1)
		{
			bind(stmt.get(), args...);
		}
		if constexpr (sizeof...(args) > 1)
		{
			multyBind(stmt.get(), args...);
		}
		int result;
		do
		{
			result = sqlite3_step(stmt.get());
		}
		while(result == SQLITE_ROW);
		if (result != SQLITE_DONE)
		{
			std::cerr << sqlite3_errmsg(db) << "\n";
		}
		return result == SQLITE_DONE;
	}
}

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
	}
	else
	{
		_db = std::shared_ptr<sqlite3>(tmp, [=](sqlite3* ptr) { sqlite3_close_v2(ptr); });
	}
	return result == SQLITE_OK;;
}

void DataBase::Close()
{
	if (_db.use_count() < 2)
	{
		sqlite3_close(_db.get());
	}
}

bool DataBase::CreateTables()
{
	// Note:	"password" and "message" columns are set to "blob" and should NEVER be readable.
	constexpr auto sql =
		"PRAGMA foreign_keys = ON;"
		"CREATE TABLE users("
		"ID				INTEGER		PRIMARY KEY,"
		"account		TEXT		UNIQUE NOT NULL,"
		"name			TEXT		DEFAULT NULL,"
		"password		BLOB		NOT	NULL);"

		"CREATE TABLE channels("
		"ID				INTEGER		PRIMARY KEY,"
		"name			TEXT		DEFAULT NULL);"

		"CREATE TABLE connections("
		"user_ID		INTEGER		REFERENCES users ON DELETE CASCADE,"
		"channel_ID		INTEGER		REFERENCES channels ON DELETE CASCADE,"
		"CONSTRAINT unq UNIQUE (user_ID, channel_ID));"

		"CREATE TABLE messages("
		"msg_time		REAL		DEFAULT CURRENT_TIMESTAMP,"
		"user_ID		INTEGER		REFERENCES users ON DELETE SET NULL,"
		"channel_ID		INTEGER		REFERENCES channels ON DELETE CASCADE,"
		"message		BLOB		NOT NULL);";

	int result = sqlite3_exec(_db.get(), sql, nullptr, nullptr, nullptr);
	//auto result = exec(_db.get(), sql); //@METO: For some reason I can't create the tables with prepared statements
	if (result != SQLITE_OK)
	{
		std::cerr << "DataBase::CreateTables(): " << sqlite3_errmsg(_db.get()) << "\n";
	}
	return result == SQLITE_OK;
}

std::optional<clientId_t> DataBase::NewUser(const std::string& account, const binaryData_t& password, const std::string& name)
{
	constexpr auto sql = "INSERT INTO users (account, name, password) VALUES(?, ?, ?);";
	if(exec(_db.get(), sql, account, name, password) != true)
	{
		return std::nullopt;
	};
	auto id = sqlite3_last_insert_rowid(_db.get());
	if (id > MAX_CLIENT_ID)
	{
		std::cerr << "DataBase::RecordUser(): user ID is too large\n";
		return std::nullopt;
	}
	return static_cast<clientId_t>(id);
}

std::optional<clientId_t> DataBase::GetUserId(const std::string& account, const binaryData_t& password)
{
	if (account == "" || password.size() < 1)
	{
		return std::nullopt;
	}

	constexpr auto sql = "SELECT ID, password FROM users WHERE account = ? LIMIT 1;";
	unique_stmt_ptr stmt(prepare(_db.get(), sql), sqlite3_finalize);

	bind(stmt.get(), account);

	if (sqlite3_step(stmt.get()) != SQLITE_ROW)
	{
		std::cout << "DataBase::GetUserId(): Account " << account << " not found or password incorrect\n";
		return std::nullopt;
	}

	auto buffer = static_cast<const char*>(sqlite3_column_blob(stmt.get(), 1));
	auto buffer_size = sqlite3_column_bytes(stmt.get(), 1);
	if (password.size() != buffer_size)
	{
		std::cout << "DataBase::GetUserId(): Account " << account << " not found or password incorrect\n";
		return std::nullopt;
	}
	if (memcmp(buffer, password.data(), buffer_size) != 0)
	{
		std::cerr << "DataBase::GetUserId(): Account " << account << " not found or password incorrect\n";
		return std::nullopt;
	}

	auto id = sqlite3_column_int(stmt.get(), 0);
	if (id > MAX_CLIENT_ID)
	{
		std::cerr << "DataBase::GetUserId(): user ID is too large\n";
		return std::nullopt;
	}

	return static_cast<clientId_t>(id);
}

bool DataBase::ChangeUserName(clientId_t user, const std::string& newName)
{
	constexpr auto sql = "UPDATE users SET name = ? WHERE ID = ?";
	exec(_db.get(), sql, newName, static_cast<int>(user));

	auto result = sqlite3_changes(_db.get()) > 0;
	if (result == false)
	{
		std::cerr << "DataBase::RenameUser(): Account #" << user << " not found\n";
	}
	return result;
}

bool DataBase::ChangeUserPassword(clientId_t user, const binaryData_t& oldPassword, const binaryData_t& newPassword)
{
	constexpr auto sql = "UPDATE users SET password = ? WHERE password = ? AND ID = ?";
	exec(_db.get(), sql, newPassword, oldPassword, static_cast<int>(user));

	auto result = sqlite3_changes(_db.get()) > 0;
	if (result == false)
	{
		std::cerr << "DataBase::RenameUser(): Account #" << user << " not found or password is incorrect.\n";
	}
	return result;
}

bool DataBase::DeleteUser(clientId_t user)
{
	constexpr auto sql = "DELETE FROM users WHERE ID = ?";
	exec(_db.get(), sql, static_cast<int>(user));

	auto result = sqlite3_changes(_db.get()) > 0;
	if (result == false)
	{
		std::cerr << "DataBase::RemoveUser(): User #" << user << " not found\n";
	}
	return result;
}

std::optional<channelId_t> DataBase::NewChannel(const std::string& name)
{
	constexpr auto sql = "INSERT INTO channels (name) VALUES(?);";
	if (exec(_db.get(), sql, name) != true)
	{
		return std::nullopt;
	}

	auto id = sqlite3_last_insert_rowid(_db.get());
	if (id > MAX_CHANNEL_ID)
	{
		std::cerr << "DataBase::NewChannel(): Channel ID is too large\n";
		return std::nullopt;
	}
	return static_cast<channelId_t>(id);
}

bool DataBase::ChangeChannelName(channelId_t channel, const std::string& newName)
{
	constexpr auto sql = "UPDATE channels SET name = ? WHERE ID = ?";
	exec(_db.get(), sql, newName, channel);
	
	auto result = sqlite3_changes(_db.get()) > 0;
	if (result != true)
	{
		std::cerr << "DataBase::ChangeChannelName(): Channel #" << channel << " not found\n";
	}
	return result;
}

bool DataBase::AddUserToChannel(clientId_t user, channelId_t channel)
{
	constexpr auto sql = "INSERT INTO connections (user_ID, channel_ID) VALUES(?, ?);";
	auto result = exec(_db.get(), sql, static_cast<int>(user), static_cast<int>(channel));

	if (result != true)
	{
		std::cerr << "DataBase::AddUserToChannel(): " << sqlite3_errmsg(_db.get()) << "\n";
	}
	return result;
}

bool DataBase::RemoveUserFromChannel(clientId_t user, channelId_t channel)
{
	constexpr auto sql = "DELETE FROM connections WHERE user_ID = ? AND channel_ID = ?;";
	exec(_db.get(), sql, static_cast<int>(user), static_cast<int>(channel));

	auto result = sqlite3_changes(_db.get()) > 0;
	if (result != true)
	{
		std::cerr << "DataBase::RemoveUserFromChannel(): " << sqlite3_errmsg(_db.get()) << "\n";
	}
	return result;
}

std::vector<channelId_t> DataBase::GetChannelsOfUser(clientId_t user)
{
	constexpr auto sql = "SELECT channel_ID FROM connections WHERE user_ID = ?;";
	unique_stmt_ptr stmt(prepare(_db.get(), sql), sqlite3_finalize);
	bind(stmt.get(), static_cast<int>(user));

	std::vector<channelId_t> result;
	while (sqlite3_step(stmt.get()) == SQLITE_ROW)
	{
		result.push_back(static_cast<channelId_t>(sqlite3_column_int(stmt.get(), 0)));
	}
	return result;
}

std::vector<clientId_t> DataBase::GetUsersOfChannel(channelId_t channel)
{
	constexpr auto sql = "SELECT user_ID FROM connections WHERE channel_ID = ?;";
	unique_stmt_ptr stmt(prepare(_db.get(), sql), sqlite3_finalize);
	bind(stmt.get(), static_cast<int>(channel));

	std::vector<channelId_t> result;
	while (sqlite3_step(stmt.get()) == SQLITE_ROW)
	{
		result.push_back(static_cast<clientId_t>(sqlite3_column_int(stmt.get(), 0)));
	}
	return result;
}

bool DataBase::DeleteChannel(channelId_t channel)
{
	constexpr auto sql = "DELETE FROM channels WHERE ID = ?";
	exec(_db.get(), sql, static_cast<int>(channel));

	auto result = sqlite3_changes(_db.get()) > 0;
	if (result == false)
	{
		std::cerr << "DataBase::RemoveUser(): Account " << channel << " not found\n";
	}
	return result;
}

bool DataBase::RecordMessage(clientId_t user, channelId_t channel, const binaryData_t& message)
{
	constexpr auto sql = "INSERT INTO messages (user_ID, channel_ID, message) VALUES(?, ?, ?);";
	auto result = exec(_db.get(), sql, static_cast<int>(user), static_cast<int>(channel), message);

	if (result != true)
	{
		std::cerr << "DataBase::RecordMessage(): " << sqlite3_errmsg(_db.get()) << "\n";
	}
	return result;
}

std::vector<std::pair<clientId_t, binaryData_t>>  DataBase::GetMessagesToChannel(channelId_t channel)
{
	constexpr auto sql = "SELECT user_ID, message FROM messages WHERE channel_ID = ?;";
	unique_stmt_ptr stmt(prepare(_db.get(), sql), sqlite3_finalize);
	bind(stmt.get(), static_cast<int>(channel));

	std::vector<std::pair<clientId_t, binaryData_t>> result;
	while (sqlite3_step(stmt.get()) == SQLITE_ROW)
	{
		auto id = static_cast<clientId_t>(sqlite3_column_int(stmt.get(), 0));
		auto buffer_start = static_cast<const char*>(sqlite3_column_blob(stmt.get(), 1));
		auto buffer_size = sqlite3_column_bytes(stmt.get(), 1);
		result.emplace_back(std::make_pair(id, binaryData_t(buffer_start, buffer_start + buffer_size)));
	}
	return result;
}

int DataBase::DeleteMessages(float olderThanDays)
{
	constexpr auto sql = "DELETE FROM messages WHERE msg_time <  DATETIME(julianday('now')-?);";
	exec(_db.get(), sql, olderThanDays);
	return sqlite3_changes(_db.get());
}