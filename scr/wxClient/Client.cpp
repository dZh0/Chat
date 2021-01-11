#include "Client.h"


bool ChatClient::InitNetwork()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	return !SDLNet_Init();
}

void ChatClient::OnError(const std::string &errorMsg)
{
	std::cerr<<errorMsg<<"\n";
}

bool ChatClient::IsConnected() const
{
	return isConnected;
}

bool ChatClient::IsMe(const msg::targetId id)
{
	return id == ChatClient::id;
}

bool ChatClient::ConnectToServer(const std::string& host, const Uint16 port, const std::string& credentials, int attemptCount, Uint32 timeout)
{
	bool isServerResolved = false;
	bool isLoginRequestSent = false;
	std::string lastErrorMessage;
	int attempt = -1;
	while(attempt < attemptCount && !isConnected)
	{
		attempt++;
		if (!isServerResolved)
		{
			if (!ConnectTo(host, port))
			{
				SDL_Delay(timeout);
				continue;
			}
			isServerResolved = true;
		}
		if (!isLoginRequestSent)
		{
			if (!RequestLogIn(credentials))
			{
				lastErrorMessage = "SDLNet_TCP_Send: " + std::string(SDLNet_GetError());
				SDL_Delay(timeout);
				continue;
			}
			isLoginRequestSent = true;
		}
		ListenForMessage(timeout);
	}
	if (!isConnected)
	{
		lastErrorMessage = (lastErrorMessage.empty())? "Login failed..." : lastErrorMessage;
		OnError(lastErrorMessage);
		Disconnect();
	}
	return isConnected;
}

bool ChatClient::SendTextMessage(const msg::targetId target, const std::string& message) const
{
	SendMessageRequest msg = SendMessageRequest();
	msg.set_recipient_id(target);
	msg.set_data(message);
	bool result;
	{
		std::lock_guard lock(mtx); // Locking acess to [serverSocket]
		result = msg::Send(serverSocket, msg::type::SEND_MESSAGE_REQUEST, msg);
	}
	return result;
}

bool ChatClient::ConnectTo(const std::string& host, const Uint16 port)
{
	IPaddress hostIp;
	std::lock_guard lock(mtx); // Locking acess to [serverSocket], [socketSet] and virtual OnError()
	if (SDLNet_ResolveHost(&hostIp, host.c_str(), port) < 0)
	{
		OnError("SDLNet_ResolveHost: " + std::string(SDLNet_GetError()));
		return false;
	}
	TCPsocket socket = SDLNet_TCP_Open(&hostIp);
	if (!socket)
	{
		OnError("SDLNet_TCP_Open: " + std::string(SDLNet_GetError()));
		return false;
	}
	socketSet = SDLNet_AllocSocketSet(1);
	if (!socketSet)
	{
		OnError("SDLNet_AllocSocketSet: " + std::string(SDLNet_GetError()));
		return false;
	}
	if (SDLNet_TCP_AddSocket(socketSet, socket) < 1)
	{
		OnError("SDLNet_AddSocket: " + std::string(SDLNet_GetError()));
		return false;
	}
	serverSocket = socket;
	return true;
}

bool ChatClient::RequestLogIn(const std::string& credentials)
{
	LoginRequest LoginRequestMsg = LoginRequest();
	LoginRequestMsg.set_credentials(credentials);
	std::lock_guard lock(mtx); // Locking acess to [serverSocket]
	return msg::Send(serverSocket, msg::type::LOGIN_REQUEST, LoginRequestMsg);
}

bool ChatClient::ListenForMessage(const Uint32 timeout)
{
	int socketsToProcess = 0;
	{
		std::lock_guard lock(mtx); // Locking acess to [socketSet]
		socketsToProcess = SDLNet_CheckSockets(socketSet, timeout);
		if (socketsToProcess < 0)
		{
			OnError("SDLNet_CheckSockets: " + std::string(SDLNet_GetError()));
		}
	}
	if (socketsToProcess <= 0)
	{
		return false;
	}
	std::lock_guard lock(mtx); // Locking acess to [serverSocket] and all virtual function calls
	if (!SDLNet_SocketReady(serverSocket))
	{
		return false;
	}
	msg::type msgType = msg::Receive<msg::type>(serverSocket);
	switch (msgType)
	{
		case msg::type::PING:
		{
			OnPing();
			msg::SendPing(serverSocket);
			break;
		}
		case msg::type::LOGIN_RESPONSE:
		{
			LoginResponse msg = msg::Receive<LoginResponse>(serverSocket);
			if (msg.status() == msg.OK)
			{
				std::vector<std::pair<const msg::targetId, std::string>> conversationList;
				for (auto &conv : msg.conversations())
				{
					conversationList.push_back(std::make_pair(conv.id(), conv.name()));
				}
				isConnected = true;
				id = static_cast<msg::targetId>(msg.id());
				OnLoginSuccessful(conversationList);
			}
			else
			{
				OnLoginFailed();
			}
			break;
		}
		case msg::type::NEW_CONVERSATION:
		{
			NewConversation msg = msg::Receive<NewConversation>(serverSocket);
			OnNewConversation(msg.id(), msg.name());
			break;
		}
		case msg::type::SEND_MESSAGE_RESPONSE:
		{
			SendMessageResponse msg = msg::Receive<SendMessageResponse>(serverSocket);
			if (msg.OK)
			{
				OnSendMessageSuccess();
			}
			else
			{
				//TODO: Maybe resend message?
				OnSendMessageFail();
			}
			break;
		}
		case msg::type::MESSAGE:
		{
			Message msg = msg::Receive<Message>(serverSocket);
			OnMessageReceived(msg.recipient_id(), msg.sender_id(), msg.data());
			break;
		}
		default:
		{
			//Skipping message
			OnInvalidMessage();
			msg::Receive<void>(serverSocket);
			return false;
		}
	}
	return true;
}

void ChatClient::Disconnect()
{
	isConnected = false;
	std::lock_guard lock(mtx); // Locking acess to [serverSocket], [socketSet] and virtual OnDisconnect()
	OnDisconnect();
	if (serverSocket)
	{
		SDLNet_TCP_DelSocket(socketSet, serverSocket);
		SDLNet_FreeSocketSet(socketSet);
		socketSet = nullptr;
		SDLNet_TCP_Close(serverSocket);
		serverSocket = nullptr;
	}
}

void ChatClient::OnDisconnect()
{
	std::cout << "DISCONNECTED\n"; 
}

