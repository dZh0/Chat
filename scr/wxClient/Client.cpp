#include <iostream>
#include "Client.h"


void ChatClient::OnError(const std::string& errorMsg) 
{
	std::cerr<<errorMsg;
}

bool ChatClient::IsConnected() const
{
	return isConnected;
}

bool ChatClient::InitNetwork()
{ 
	return  (SDLNet_Init() >= 0);
}

bool ChatClient::ConnectToServer(const std::string& host, const Uint16 port, const std::string& credentials, int attemptCount, Uint32 timeout)
{
	bool isServerResolved = false;
	bool isLoginRequestSent = false;
	std::string lastErrorMessage = "";
	int attempt = -1;
	while(attempt < attemptCount && !isConnected)
	{
		attempt++;
		if (!isServerResolved)		//@METO: Will "try-catch" be more graceful in this case?
		{
			if (!ConnectTo(host, port))
			{
				SDL_Delay(timeout);
				continue;
			}
			isServerResolved = true;
		}
		GOOGLE_PROTOBUF_VERIFY_VERSION;
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
		ListenForMessage(timeout, message::type::LOGIN_RESPONSE); // Will set isConnected=true of successful response is recieved
	}
	if (!isConnected)
	{
		OnError(lastErrorMessage);
		Disconnect();
	}
	return isConnected;
}
bool ChatClient::ConnectTo(const std::string& host, const Uint16 port)
{
	Disconnect();
	IPaddress hostIp;
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
	LoginRequestMsg.set_credentials((std::string)credentials);
	return SendProtoMessage(serverSocket, message::type::LOGIN_REQUEST, LoginRequestMsg);
}

bool ChatClient::ListenForMessage(const Uint32 timeout, const message::type filter)
{
	std::lock_guard lock(mtx);
	int socketsToProcess = SDLNet_CheckSockets(socketSet, timeout);
	if (socketsToProcess < 0)
	{
		OnError("SDLNet_CheckSockets: " + std::string(SDLNet_GetError()));
		return false;
	}
	if (socketsToProcess > 0)
	{
		if (SDLNet_SocketReady(serverSocket))
		{
			message::type msgType = (message::type)Receive<Sint16>();
			if (msgType != filter && filter != message::type::ALL)
			{
				DiscardMessage();
				return false;
			}
			switch ((message::type)msgType)
			{
				case message::type::PING:
				{
					Ping();
					OnPingMessageRecieved();
					break;
				}
				case message::type::LOGIN_RESPONSE:
				{
					LoginResponse msg = Receive<LoginResponse>();
					if (msg.status() == msg.OK)
					{
						isConnected = true;
						OnLoginSuccessful();
					}
					else
					{
						OnLoginFailed();
					}
					break;
				}
				case message::type::SEND_MESSAGE_RESPONSE:
				{
					SendMessageResponse msg = Receive<SendMessageResponse>();
					if (msg.OK)
					{
						OnSendMessageSuccess();
					}
					else
					{
						// TODO: Maybe resend message?
						OnSendMessageFail();
					}
					break;
				}
				case message::type::MESSAGE:
				{
					Message msg = Receive<Message>();
					OnMessageReceived(msg.sender_id(), msg.data());
					break;
				}
				default:
				{
					DiscardMessage();
					return false;
				}
			}
			return true; // if (msgType == filter || filter == message::type::ALL)
		}
	}
	return false; //if (socketsToProcess == 0)
}


void ChatClient::Disconnect()
{
	isConnected = false;
	OnDisconnect();
	std::lock_guard lock(mtx);
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

template<class T>
const T ChatClient::Receive() const
{
	int msgSize = Receive<Sint16>();
	std::cout << msgSize << " bytes of data . . .\n";
	T message;
	google::protobuf::Message* msgPtr = static_cast<google::protobuf::Message*>(&message);
	int bytesRead = 0;
	if (msgSize > 0)
	{
		char* buffer = new char[msgSize];
		{
			std::lock_guard lock(mtx);
			bytesRead = SDLNet_TCP_Recv(serverSocket, buffer, msgSize);
		}
		msgPtr->ParseFromArray(buffer, msgSize);
		delete[] buffer;
	}
	return message;
}

// Specialization
template<>
const Sint16 ChatClient::Receive<Sint16>() const
{
	int msgSize = sizeof(Uint16);
	char* buffer = new char[msgSize];
	int bytesRead = 0;
	{
		std::lock_guard lock(mtx);
		bytesRead = SDLNet_TCP_Recv(serverSocket, buffer, msgSize);
	}
	Sint16 result = -1;
	if (bytesRead == msgSize)
	{
		result = (Sint16)SDLNet_Read16(buffer);
	}
	delete[] buffer;
	return result;
}

void ChatClient::DiscardMessage() const
{
	int msgSize = Receive<Sint16>();
	std::cout << msgSize << " discarding . . .\n";
	if (msgSize > 0)
	{
		char* buffer = new char[msgSize]; //@METO: Can I go with void* buffer and would the delete[] work on that type?
		{
			std::lock_guard lock(mtx);
			SDLNet_TCP_Recv(serverSocket, buffer, msgSize);
		}
		delete[] buffer;
	}
}


void ChatClient::OnMessageReceived(const std::string& senderId, const std::string& message)
{
	std::cout << senderId << ":\t" << message << "\n";
}

template<class T>
bool ChatClient::SendProtoMessage(const TCPsocket socket, message::type msgType, const T& message)
{
	int protoSize = message.ByteSizeLong();
	int msgSize = sizeof(Sint16) * 2 + protoSize;
	char* buffer = new char[msgSize];

	SDLNet_Write16((Uint16)msgType, buffer);
	SDLNet_Write16(protoSize, buffer + sizeof(Sint16));
	message.SerializeToArray(buffer + sizeof(Sint16) * 2, protoSize);

	std::cout << "Sending " << msgSize << " bytes...\n";
	int bytesSent = 0;
	{
		std::lock_guard lock(mtx);
		bytesSent = SDLNet_TCP_Send(socket, buffer, msgSize);
	}
	delete[] buffer;
	if (bytesSent < msgSize)
	{
		OnError("SDLNet_TCP_Send: " + std::string(SDLNet_GetError()));
		return false;
	}
	return true;
}

bool ChatClient::Ping()
{
	const char PING_MSG[] = { '\0','\0' };
	const int PING_SIZE = 2;
	std::cout << "Sending 2 bytes...\n";
	int bytesSent = 0;
	{
		std::lock_guard lock(mtx);
		bytesSent = SDLNet_TCP_Send(serverSocket, PING_MSG, PING_SIZE);
	}
	if (bytesSent < PING_SIZE)
	{
		OnError("SDLNet_TCP_Send: " + std::string(SDLNet_GetError()));
		return false;
	}
	return true;
}
