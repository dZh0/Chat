#pragma once

#include <string>
#include <shared_mutex>
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

	virtual void OnError(const std::string &errorMsg);
	virtual void OnPingMessageRecieved() {};
	virtual void OnLoginSuccessful(const std::vector<std::pair<const message::idType, std::string>> &conversationList) {};
	virtual void OnLoginFailed() {};
	virtual void OnSendMessageSuccess() {};
	virtual void OnSendMessageFail() {};
	virtual void OnMessageReceived(const message::idType senderId, const std::string& message);
	virtual void OnDisconnect();

	bool IsConnected() const;
	bool InitNetwork();
	
	bool ConnectToServer(const std::string& host, const Uint16 port, const std::string& credentials, int attemptCount = 5, Uint32 attemptTime = 3000);
	bool SendMessageToTarget(const uint32_t target, const std::string& message, Uint32 timeout);
	bool ListenForMessage(const Uint32 timeout = 1000, const message::type filter = message::type::ALL);
	void Disconnect();

	bool Ping();

private:
	mutable std::recursive_mutex mtx;
	TCPsocket serverSocket;
	SDLNet_SocketSet socketSet;
	std::atomic_bool isConnected = false;

	bool ConnectTo(const std::string& host, const Uint16 port);
	bool RequestLogIn(const std::string& credentials);

	template<class T>
	bool SendProtoMessage(const TCPsocket socket, const message::type msgType, const T& message);

	template<class T>
	const T Receive() const;
	template<>
	const Sint16 Receive<Sint16>() const;
	template<>
	const void Receive<void>() const;
	void DiscardMessage() const;
};

