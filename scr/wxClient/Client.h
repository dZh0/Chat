#pragma once
#include <string>
#include <mutex>
#include <SDL_net.h>
#include "Message.h"

constexpr Uint32 ACTIVITY_CHECK_TIMEOUT = 1000;	// How long the client will wait for activity (in [ms]);

class ChatClient
{
public:
	virtual ~ChatClient() {};

	virtual void OnError(const std::string& errorMsg);
	virtual void OnPing() {};
	virtual void OnLoginSuccessful(const std::vector<std::pair<const msg::targetId, std::string>> &conversationList) {};
	virtual void OnLoginFailed() {};
	virtual void OnNewConversation(const msg::targetId id, const std::string& name) {};
	virtual void OnSendMessageSuccess() {};
	virtual void OnSendMessageFail() {};
	virtual void OnMessageReceived(const msg::targetId senderId, const std::string& message){};
	virtual void OnInvalidMessage() {};
	virtual void OnDisconnect();

	bool InitNetwork();
	bool IsConnected() const;
	bool ConnectToServer(const std::string& host, const Uint16 port, const std::string& credentials, int attemptCount = 5, Uint32 attemptTime = 3000);
	bool SendTextMessage(const msg::targetId target, const std::string& message);
	bool ListenForMessage(const Uint32 timeout = 1000);
	void Disconnect();

private:
	mutable std::mutex mtx;
	TCPsocket serverSocket = nullptr;
	SDLNet_SocketSet socketSet = nullptr;
	std::atomic_bool isConnected = false;

	bool ConnectTo(const std::string& host, const Uint16 port);
	bool RequestLogIn(const std::string& credentials);
};

