#include <iostream>
#include <assert.h>
#include "DataBase.h"

int main()
{
	remove("Test.db");
	DataBase db;
	if (!db.Open("Test.db"))
	{
		return EXIT_FAILURE;
	}
	assert(db.CreateTables()==true);

	//////////////////////////////
	// User tests
	//////////////////////////////
	std::string str1 = "123";
	std::string str2 = "padpaduk";
	std::string str3 = "1RozovaRoza";
	binaryData_t password1(str1.begin(), str1.end());
	binaryData_t password2(str2.begin(), str2.end());
	binaryData_t password3(str3.begin(), str3.end());

	auto u1 = db.NewUser("bob@bob.bob", password1);
	assert(u1 != std::nullopt);
	auto u2 = db.NewUser("merry@merry.mer", password1);						// same password (OK)
	assert(u2 != std::nullopt);
	auto u3 = db.NewUser("paul@paul.pl", password3);
	assert(u3 != std::nullopt);
	assert(db.NewUser("bob@bob.bob", password2) == std::nullopt);			//non-unique account

	auto user1 = u1.value();
	auto user2 = u2.value();
	auto user3 = u3.value();

	assert(db.GetUserId("merry@merry.mer", password2) == std::nullopt);		//wrong password
	assert(db.GetUserId("x@x.x", password2) == std::nullopt);				//non-existent account
	assert(db.GetUserId("merry@merry.mer", password1) == user2);

	assert(db.ChangeUserName(user1, "Bobislav") == true);
	assert(db.ChangeUserName(1000, "Bobislav") == false);					//non-existent account ID

	assert(db.ChangeUserPassword(1000, password1, password2) == false);		//non-existent account ID
	assert(db.ChangeUserPassword(user2, password2, password1) == false);	//wrong old password
	assert(db.ChangeUserPassword(user2, password1, password2) == true);
	assert(db.GetUserId("merry@merry.mer", password1) == std::nullopt);		// old password now wrong
	assert(db.GetUserId("merry@merry.mer", password2) == user2);			// new password now correct

	//////////////////////////////
	// Channel tests
	//////////////////////////////
	auto ch1 = db.NewChannel("Global");
	assert(ch1 != std::nullopt);
	auto ch2 = db.NewChannel();
	assert(ch2 != std::nullopt);
	
	auto channel1 = ch1.value();
	auto channel2 = ch2.value();

	assert(db.ChangeChannelName(channel1, "Local") == true);
	assert(db.ChangeChannelName(channel2) == true);
	assert(db.ChangeChannelName(1000, "Local") == false);	// non-existent channel ID

	assert(db.AddUserToChannel(user1, channel1) == true);
	assert(db.AddUserToChannel(user1, channel2) == true);
	assert(db.AddUserToChannel(user2, channel1) == true);
	assert(db.AddUserToChannel(user2, channel2) == true);
	assert(db.AddUserToChannel(user3, channel1) == true);
	assert(db.AddUserToChannel(user3, channel1) == false); // added twice

	// before removal
	auto userList = db.GetUsersOfChannel(channel1);
	assert(userList.size() == 3);
	auto channelList = db.GetChannelsOfUser(user1);
	assert(channelList.size() == 2);

	assert(db.RemoveUserFromChannel(user1, channel1) == true);
	assert(db.RemoveUserFromChannel(user3, channel2) == false); // user3 was never added to the channel2

	// after removal
	userList = db.GetUsersOfChannel(channel1);
	assert(userList.size() == 2);
	channelList = db.GetChannelsOfUser(user1);
	assert(channelList.size() == 1);

	//////////////////////////////
	// Message test
	//////////////////////////////
	std::string msg1 = "Message 1";
	std::string msg2 = "Message 2";
	std::string msg3 = "Message 3";

	binaryData_t message1(msg1.begin(), msg1.end());
	binaryData_t message2(msg2.begin(), msg2.end());
	binaryData_t message3(msg3.begin(), msg3.end());
	binaryData_t message4;

	assert(db.RecordMessage(user1, channel2, message1)==true);
	assert(db.RecordMessage(user2, channel1, message2)==true);
	assert(db.RecordMessage(user3, channel1, message3)==true);
	assert(db.RecordMessage(1000, channel1, message3) == false);	// non-existing user
	assert(db.RecordMessage(user3, 1000, message3) == false);		// non-existing channel
	assert(db.RecordMessage(user3, channel1, message4) == false);	// empty message
	
	assert(db.GetMessagesToChannel(channel1).size() == 2);
	assert(db.DeleteMessages(1.f) == 0);
	assert(db.DeleteMessages(-1.f) == 3);
	assert(db.GetMessagesToChannel(channel1).size() == 0);

	//////////////////////////////
	// User linkage test
	//////////////////////////////

	assert(db.AddUserToChannel(user1, channel1) == true);

	// before removal
	assert(db.GetUsersOfChannel(channel1).size() == 3);
	assert(db.RecordMessage(user1, channel1, message1) == true);
	assert(db.GetMessagesToChannel(channel1)[0].first == user1);

	assert(db.DeleteUser(user1) == true);

	// after removal
	assert(db.GetUsersOfChannel(channel1).size() == 2);
	assert(db.RecordMessage(user1, channel2, message1) == false);
	assert(db.GetMessagesToChannel(channel1)[0].first == NULL);

	//////////////////////////////
	// Channel linkage test
	//////////////////////////////

	// before removal
	assert(db.GetUsersOfChannel(channel2).size() == 1);
	assert(db.RecordMessage(user2, channel2, message1) == true);
	assert(db.GetMessagesToChannel(channel2).size() == 1);
	
	assert(db.DeleteChannel(channel2) == true);

	// after removal
	assert(db.GetUsersOfChannel(channel2).size() == 0);
	assert(db.RecordMessage(user2, channel2, message1) == false);
	assert(db.GetMessagesToChannel(channel2).size() == 0);

	return 0;
}