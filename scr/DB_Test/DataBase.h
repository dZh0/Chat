#pragma once
#include <optional>
#include <vector>
#include <sqlite3.h>

using clientId_t = int;
constexpr clientId_t MAX_CLIENT_ID = std::numeric_limits<clientId_t>::max();

using channelId_t = int;
constexpr clientId_t MAX_CHANNEL_ID = std::numeric_limits<channelId_t>::max();

using binaryData_t = std::vector<char>;

class DataBase
{
public:
	// Database functions
	DataBase() = default;
	DataBase(const std::string& fileName);
	bool Open(const std::string& fileName);
	void Close();
	bool CreateTables();

	// User functions
	std::optional<clientId_t> NewUser(const std::string &account, const binaryData_t &password, const std::string &name = "");
	std::optional<clientId_t> GetUserId(const std::string &account, const binaryData_t &password);
	bool ChangeUserName(clientId_t user, const std::string& newName);
	bool ChangeUserPassword(clientId_t user, const binaryData_t& oldPassword, const binaryData_t& newPassword);
	bool DeleteUser(clientId_t user);

	// Channel functions
	std::optional<channelId_t> NewChannel(const std::string& name = "");
	bool ChangeChannelName(channelId_t channel, const std::string& newName = "");
	bool AddUserToChannel(clientId_t user, channelId_t channel);
	bool RemoveUserFromChannel(clientId_t user, channelId_t channel);
	std::vector<channelId_t> GetChannelsOfUser(clientId_t user);
	std::vector<clientId_t> GetUsersOfChannel(channelId_t channel);
	bool DeleteChannel(channelId_t channel);

	// Message funtions
	bool RecordMessage(clientId_t user, channelId_t channel, const binaryData_t& message);
	std::vector<std::pair<clientId_t, binaryData_t>> GetMessagesToChannel(channelId_t channel);
	int DeleteMessages(float olderThanDays);

private:
	std::shared_ptr<sqlite3> _db;
};
