#pragma once

#include <vector>
#include <string>
#include <memory>
#include <chrono>
//#include "DataBase.h"
#include "Message.h"

// SERVER SETTINGS
constexpr Uint32 ACTIVITY_CHECK_TIMEOUT = 1000;	// How long the server will wait for activity (in [ms]);
constexpr Uint32 NOTICE_TIME = 5000;			// How long before disconnecting, the server will send a check ping to the client (in [ms]) 
constexpr Uint32 INACTIVITY_TIME = 10000;		// How long a client must be inactive to be checked from the server (in [ms])

using binary_t = std::vector<std::byte>;


namespace network
{
	class ISocket
	{
	public:
		virtual bool IsReady() = 0;
	};
	bool Init();
	void Quit();
	std::unique_ptr<ISocket> Open(int port, int maxClients);
}

struct ClientData
{
	msg::targetId id = 0;
	TCPsocket socket = nullptr;
	std::string credentials;
	std::string name;
	Uint32 lastMesageTime = 0;
	bool onNotice = false;
	std::vector<msg::targetId> conversations = {};
};

struct Conversation
{
	std::string name = "Global";
	std::vector<ClientData*> observers;
	bool isPrivate = false;
};

class ChatServer
{
public:
	virtual ~ChatServer();

	virtual bool Init2(int port, int maxClients);
	virtual bool Init(int port, int maxClients = 1);
	virtual bool Update();
	virtual bool Update(float delaySeconds);
	virtual void OnMessageReceived(ClientData& client);
	virtual void Disconnect();

	bool ReceiveMessage(ClientData& client);

	LoginResponse LoginClient(ClientData& client, const std::string& credentials);
	Conversation& AddClientToConversation(const msg::targetId id, ClientData& client);
	
	const TCPsocket FindClient(const std::string& id) const;
	
	void CheckForInactivity(ClientData& client);
	ClientData* AcceptConnection();
	void DisconnectClient(ClientData& client);
	void DeleteDisconnectdClients();
	

	virtual void OnPing(const ClientData& client);
	virtual void OnLoginRequest(const ClientData& client);

protected:
	msg::targetId clientIdCounter = 1;
	//TCPsocket listeningSocket = nullptr;
	
	std::unique_ptr<network::ISocket> socket{nullptr};

	//SDLNet_SocketSet socketSet = nullptr;
	std::vector<ClientData> clientArr;
	bool clientArrDirty = false;
	Uint32 nextActivityCheckSceduleTime = 0xFFFFFFFF;
	unsigned maxConnections{0};
	std::map<const msg::targetId, Conversation> targets = { {0, {"Global",{},false}} };
private:
	SendMessageResponse ForwardMessage(const ClientData& client, const msg::targetId& id, const std::string& data) const;
//	DataBase serverDataBase;
};
