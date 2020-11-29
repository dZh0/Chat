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

	virtual void OnError(const std::string &errorMsg); //@METO: Not const since the wxApp needs to queue an event here; Consequently no method calling OnError() can be const :(
	virtual void OnPingMessageRecieved() {};
	virtual void OnLoginSuccessful() {};
	virtual void OnLoginFailed() {};
	virtual void OnSendMessageSuccess() {};
	virtual void OnSendMessageFail() {};
	virtual void OnMessageReceived(const std::string& senderId, const std::string& message);
	virtual void OnDisconnect();

	bool IsConnected() const;
	bool InitNetwork();
	bool ConnectToServer(const std::string& host, const Uint16 port, const std::string& credentials, int attemptCount = 5, Uint32 attemptTime = 3000);
	bool ListenForMessage(const Uint32 timeout = 1000, const message::type filter = message::type::ALL);
	void Disconnect();

	// Sending Message
	template<class T>
	bool SendProtoMessage(const TCPsocket socket, const message::type msgType, const T& message);
	bool Ping();

private:
	mutable std::shared_mutex mtx; //@METO: mutable, in order to enable a lock from const functions?
	TCPsocket serverSocket;
	SDLNet_SocketSet socketSet;
	std::atomic_bool isConnected = false;

	bool ConnectTo(const std::string& host, const Uint16 port);
	bool RequestLogIn(const std::string& credentials);
	void LoginSuccessful();

	template<class T>
	const T Receive() const;
	template<>
	const Sint16 Receive() const;
	void DiscardMessage() const;
};

