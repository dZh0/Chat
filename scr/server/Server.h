#pragma once

#include <vector>
#include <string>
#include "SDL_net.h"
#include "../ProtoBuffer/Message.h"

// SERVER SETTINGS
constexpr Uint32 ACTIVITY_CHECK_TIMEOUT = 1000;	// How long the server will wait for activity (in [ms]);
constexpr Uint32 NOTICE_TIME = 5000;				// How long before disconnecting, the server will send a chekc ping to the client (in [ms]) 
constexpr Uint32 INACTIVITY_TIME = 10000;		// How long a client must be inactive to be checked from the server (in [ms])

struct ClientData
{
	char id[sizeof(Uint16)] = "";
	std::string credentials;
	TCPsocket socket = nullptr;
	Uint32 checkTime = 0xFFFFFFFF;
	bool onNotice = false;
};

class ChatServer
{
public:
	virtual ~ChatServer();

	virtual bool Init(int port, int _maxConnections = 1);
	virtual bool Update();
	virtual void OnMessageReceived(ClientData& client);
	virtual void Disconnect();

	bool OpenPort(int port);
	bool SetMaxConnections(int _maxConnections = 1);
	void ClearSocketSet();
	int FillSocketSet();

	bool ReceiveMessage(ClientData& client);
	int ReceiveSint16(const TCPsocket socket) const;
	template<class T>
	const T ReceiveProtoMessage(const TCPsocket socket) const;

	LoginResponse LoginClient(ClientData& client, const std::string& credentials);
	virtual bool CheckCredentials(const std::string& credentials);
	SendMessageResponse ForwardMessage(const std::string& senderID, const std::string& targetID, const std::string& data) const;
	const TCPsocket FindClient(const std::string& id) const;
	
	void CheckForInactivity(ClientData& client);
	ClientData* AcceptConnection();
	void DisconnectClient(ClientData& client);
	void DeleteDisconnectdClients();

	template<class T>
	bool SendProtoMessage(const TCPsocket socket, message::type msgType, const T& message) const;
	bool Ping(ClientData& client) const;

protected:
	Uint16 clientIdCounter = 1;
	TCPsocket listeningSocket = nullptr;
	SDLNet_SocketSet socketSet = nullptr;
	std::vector<ClientData> clientArr;
	bool clientArrDirty = false;
	Uint32 nextActivityCheckSceduleTime = 0xFFFFFFFF;
	int maxConnections = 1;
};