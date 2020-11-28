#pragma once

#include <string>
#include <SDL_net.h>
#include "../ProtoBuffer/Message.h"

constexpr Uint32 ACTIVITY_CHECK_TIMEOUT = 1000;	// How long the client will wait for activity (in [ms]);

class ChatClient
{
public:
	virtual ~ChatClient() {};

	std::string credentials = "Bob";
	std::string serverIP = "localhost";
	Uint16 serverPort = 1234;
	char id[sizeof(Uint16)] = {};

	virtual void OnError(const std::string &errorMsg) const;
	virtual bool InitNetwork();
	bool ConnectToServer(const std::string& host, const Uint16 port, const std::string& credentials, unsigned attemptCount = 5, Uint32 attemptTime = 3000);
	bool RequestLogIn(const std::string& credentials);
	virtual bool Update();
	virtual void OnMessageReceived();
	void Disconnect();
	virtual void OnDisconnect();

	int ReceiveMessage();
	int ReceiveSint16(const TCPsocket socket) const;

	template<class T>
	const T ReceiveProtoMessage(const TCPsocket socket) const;

	virtual void OnPingMessageRecieved() {};
	virtual bool OnMessageReceived(const LoginResponse& message);
	virtual bool OnMessageReceived(const SendMessageResponse& message);
	virtual bool OnMessageReceived(const Message& message);

	template<class T>
	bool SendProtoMessage(const TCPsocket socket, const message::type msgType, const T& message) const;
	bool Ping() const;

	bool isConnected = false;
	bool isOnNotice = false;
private:
	TCPsocket socket;
	SDLNet_SocketSet socketSet;
};

