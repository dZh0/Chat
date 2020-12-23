#pragma once

#include <vector>
#include <string>
#include "SDL_net.h"
#include "../ProtoBuffer/Message.h"

// SERVER SETTINGS
constexpr Uint32 ACTIVITY_CHECK_TIMEOUT = 1000;	// How long the server will wait for activity (in [ms]);
constexpr Uint32 NOTICE_TIME = 5000;			// How long before disconnecting, the server will send a check ping to the client (in [ms]) 
constexpr Uint32 INACTIVITY_TIME = 10000;		// How long a client must be inactive to be checked from the server (in [ms])

struct ClientData
{
	std::string credentials;
	TCPsocket socket = nullptr;
	Uint32 lastMesageTime = 0;
	bool onNotice = false;
	std::vector<message::idType> conversations = {};
};

// The idea behind "conversations" is to facilitate messaging offline clients by targeting their conversations. (that will not be removed)
struct Conversation
{
	std::string name = "Global";
	std::vector<ClientData*> participants;
	bool isPrivate = false;
};

class ChatServer
{
public:
	virtual ~ChatServer();

	virtual bool Init(int port, int maxClients = 1);
	virtual bool Update();
	virtual void OnMessageReceived(ClientData& client);
	virtual void Disconnect();

	bool ReceiveMessage(ClientData& client);
	int ReceiveSint16(const TCPsocket socket) const;
	template<class T>
	const T ReceiveProtoMessage(const TCPsocket socket) const;

	LoginResponse LoginClient(ClientData& client, const std::string& credentials);
	Conversation& AddClientToConversation(const message::idType id, ClientData& client);
	virtual bool CheckCredentials(const std::string& credentials);
	SendMessageResponse ForwardMessage(const message::idType senderId, const std::string& targetID, const std::string& data) const;
	const TCPsocket FindClient(const std::string& id) const;
	
	void CheckForInactivity(ClientData& client);
	ClientData* AcceptConnection();
	void DisconnectClient(ClientData& client);
	void DeleteDisconnectdClients();

	template<class T>
	bool SendProtoMessage(const TCPsocket socket, message::type msgType, const T& message) const;
	bool Ping(ClientData& client) const;
	
protected:
	message::idType clientIdCounter = 1;
	TCPsocket listeningSocket = nullptr;
	SDLNet_SocketSet socketSet = nullptr;
	std::vector<ClientData> clientArr;
	bool clientArrDirty = false;
	Uint32 nextActivityCheckSceduleTime = 0xFFFFFFFF;
	unsigned maxConnections = 1;

	//conversations
	std::map<const message::idType, Conversation> targets = { {0, {"Global",{},false}} };
	const Conversation& globalConversation = targets[0];

};
