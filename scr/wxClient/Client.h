#pragma once

#include <string>
#include "SDL_net.h"
#include "../ProtoBuffer/Message.h"

constexpr Uint32 ACTIVITY_CHECK_TIMEOUT = 1000;	// How long the client will wait for activity (in [ms]);

class ChatClient
{
public:
	virtual ~ChatClient() {};

	std::string credentials = "Bob";
	char id[sizeof(Uint16)] = {};
	std::string errorMessage = "";	//Used to store error 

	virtual bool Init();
	virtual bool ConnectTo(std::string host, Uint16 port);
	virtual bool RequestLogIn(const std::string& credentials);
	virtual bool Update();
	virtual void OnMessageReceived();
	virtual void Disconnect();

	bool ReceiveMessage();
	int ReceiveSint16(const TCPsocket socket) const;

	template<class T>
	const T ReceiveProtoMessage(const TCPsocket socket) const;

	virtual bool HandleMessage(const LoginResponse& message);
	virtual bool HandleMessage(const SendMessageResponse& message) const;
	virtual bool HandleMessage(const Message& message) const;

	template<class T>
	bool SendProtoMessage(const TCPsocket socket, const message::type msgType, const T& message) const;
	bool Ping() const;

	bool isConnected = false;
	bool isOnNotice = false;
private:
	TCPsocket socket;
	SDLNet_SocketSet socketSet;
};