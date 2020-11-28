#include <iostream>
#include "Client.h"


void ChatClient::OnError(const std::string& errorMsg) const
{
	std::cerr<<errorMsg;
}

bool ChatClient::InitNetwork()
{
	if (SDLNet_Init() < 0)
	{
		OnError("SDLNet_Init: " + std::string(SDLNet_GetError()));
		return false;
	}
    return true;
}

bool ChatClient::ConnectToServer(const std::string& host, const Uint16 port, const std::string& credentials, unsigned attemptCount, Uint32 attemptTime)
{
	std::string lastErrorMessage = "";
	bool isServerSocketResolved = false;
	bool isLoginSuccessful = false;
	for(unsigned i = 0; i < attemptCount; i++)
	{
		if (!isServerSocketResolved)
		{
			IPaddress hostIp;
			if (SDLNet_ResolveHost(&hostIp, host.c_str(), port) < 0)
			{
				lastErrorMessage = "SDLNet_ResolveHost: " + std::string(SDLNet_GetError());
				SDL_Delay(attemptTime);
				continue;
			}
			TCPsocket serverSocket = SDLNet_TCP_Open(&hostIp);
			if (!serverSocket)
			{
				lastErrorMessage = "SDLNet_TCP_Open: " + std::string(SDLNet_GetError());
				SDL_Delay(attemptTime);
				continue;
			}
			socketSet = SDLNet_AllocSocketSet(1);
			if (!socketSet)
			{
				lastErrorMessage = "SDLNet_AllocSocketSet: " + std::string(SDLNet_GetError());
				SDL_Delay(attemptTime);
				continue;
			}
			if (SDLNet_TCP_AddSocket(socketSet, serverSocket) < 1)
			{
				lastErrorMessage = "SDLNet_TCP_AddSocket: " + std::string(SDLNet_GetError());
				SDL_Delay(attemptTime);
				continue;
			}
			socket = serverSocket;
			GOOGLE_PROTOBUF_VERIFY_VERSION;
			isServerSocketResolved = true;
		}
		if (!RequestLogIn(credentials)) //METO: Should I request Log in on every attempt considering that TCP guarntees the server has recievet the message if this check fails
		{
			lastErrorMessage = "SDLNet_TCP_Send: " + std::string(SDLNet_GetError());
			SDL_Delay(attemptTime);
			continue;
		}
		int socketsToProcess = SDLNet_CheckSockets(socketSet, attemptTime);
		if (socketsToProcess < 0)
		{
			lastErrorMessage = "SDLNet_CheckSockets: " + std::string(SDLNet_GetError());
			continue;
		}
		if (socketsToProcess > 0)
		{
			if (SDLNet_SocketReady(socket))
			{
				if (!ReceiveMessage())
				{
					continue;
				}
				else
				{
					//TODO: Here I'm not sure that the message recieved was login request "ok"...
					isLoginSuccessful = true;
					break;
				}
			}
		}
	}
	isConnected = isServerSocketResolved && isLoginSuccessful;
	if (!isConnected)
	{
		OnError(lastErrorMessage);
		Disconnect();
	}
	return isConnected;
}

bool ChatClient::RequestLogIn(const std::string& credentials)
{
	LoginRequest LoginRequestMsg = LoginRequest();
	LoginRequestMsg.set_credentials((std::string)credentials);
	return SendProtoMessage(socket, message::type::LOGIN_REQUEST, LoginRequestMsg);
}


bool ChatClient::Update()
{
	if (SDLNet_Read16(id) == 0)
	{
		std::cout << "Requesting login for \"" << std::string(credentials) << "\". . .\n";
		RequestLogIn(credentials);
	}
	int socketsToProcess = SDLNet_CheckSockets(socketSet, ACTIVITY_CHECK_TIMEOUT);
	if (socketsToProcess < 0)
	{
		OnError("SDLNet_CheckSockets: " + std::string(SDLNet_GetError()));
		return false;
	}
	if (socketsToProcess > 0)
	{
		if (SDLNet_SocketReady(socket))
		{
			ReceiveMessage();
			OnMessageReceived();
		}
	}
	if (isOnNotice)
	{
		//TODO: Test message should be replaced with ping to keep the connection alive but sends a message instead for test purposes
		SendMessageRequest testMessage;
		char target[2] ="";
		SDLNet_Write16(2, target);
		testMessage.set_recipient_id(target, sizeof(Uint16));
		testMessage.set_data("This message should be sent to the second client.");
		SendProtoMessage(socket, message::type::SEND_MESSAGE_REQUEST, testMessage);
		isOnNotice = false;
	}
	return true;
}

void ChatClient::OnMessageReceived()
{
	//SendMessage("Test message - type: 1; size: 37+4 B");
}

void ChatClient::Disconnect()
{
	if (socket || isConnected)
	{
		SDLNet_TCP_DelSocket(socketSet, socket);
		SDLNet_FreeSocketSet(socketSet);
		socketSet = nullptr;
		SDLNet_TCP_Close(socket);
		socket = nullptr;
		isConnected = false;
	}
	OnDisconnect();
}

void ChatClient::OnDisconnect()
{
	std::cout << "DISCONNECTED\n"; 
}

int ChatClient::ReceiveMessage()
{
	int msgType = ReceiveSint16(socket);
	switch((message::type)msgType)
	{
		case message::type::PING:
		{
			isOnNotice = true; //TODO: Make Thread safe
			OnPingMessageRecieved();
			break;
		}
		case message::type::LOGIN_RESPONSE:
		{
			LoginResponse msg = ReceiveProtoMessage<LoginResponse>(socket);
			OnMessageReceived(msg);
		}
		case message::type::SEND_MESSAGE_RESPONSE:
		{
			SendMessageResponse msg = ReceiveProtoMessage<SendMessageResponse>(socket);
			OnMessageReceived(msg);
			break;
		}
		case message::type::MESSAGE:
		{
			Message msg = ReceiveProtoMessage<Message>(socket);
			OnMessageReceived(msg);
			break;
		}
		default:
		{
			return -1;
		}
	}
	return msgType;
}

int ChatClient::ReceiveSint16(const TCPsocket socket) const
{
	const int size = sizeof(Sint16);
	char buffer[size];
	int bytesRead = SDLNet_TCP_Recv(socket, buffer, size);
	if (bytesRead < size)
	{
		return -1;
	}
	return (Sint16)SDLNet_Read16(buffer);
}

template<class T>
const T ChatClient::ReceiveProtoMessage(const TCPsocket socket) const
{
	int msgSize = ReceiveSint16(socket);
	std::cout << msgSize << " bytes of data . . .\n";
	T message;
	google::protobuf::Message* msgPtr = dynamic_cast<google::protobuf::Message*>(&message);
	if (msgSize > 0)
	{
		char* buffer = new char[msgSize];
		int bytesRead = SDLNet_TCP_Recv(socket, buffer, msgSize);
		msgPtr->ParseFromArray(buffer, msgSize);
		delete[] buffer;
	}
	return message;
}

bool ChatClient::OnMessageReceived(const LoginResponse& message)
{
	if (message.status() == message.OK)
	{
		memcpy(id, message.id().c_str(), sizeof(Uint16));
		return true;
	}
	else
	{
		return false;
	}
}

bool ChatClient::OnMessageReceived(const SendMessageResponse& message)
{
	if (message.OK)
	{
		return true;
	}
	else
	{
		OnError("Sending message failed!");
		//TODO: Handle failed "send message" attempt...
		//Probably the response should include a way for the client to identigy the message and resend it...
		return false;
	}
}

bool ChatClient::OnMessageReceived(const Message& message)
{
	std::cout << message.sender_id() << ":\t" << message.data() << "\n";
	return true;
}

template<class T>
bool ChatClient::SendProtoMessage(const TCPsocket socket, message::type msgType, const T& message) const
{
	int protoSize = message.ByteSizeLong();
	int msgSize = sizeof(Sint16) * 2 + protoSize;
	char* buffer = new char[msgSize];

	SDLNet_Write16((Uint16)msgType, buffer);
	SDLNet_Write16(protoSize, buffer + sizeof(Sint16));
	message.SerializeToArray(buffer + sizeof(Sint16) * 2, protoSize);

	std::cout << "Sending " << msgSize << " bytes...\n";
	size_t send = SDLNet_TCP_Send(socket, buffer, msgSize);
	if (send < msgSize)
	{
		//OnError("SDLNet_TCP_Send: " + std::string(SDLNet_GetError()));
		return false;
	}
	delete[] buffer;
	return true;
}

bool ChatClient::Ping() const
{
	std::cout << "Sending 2 bytes...\n";
	const char PING_MSG[] = { '\0','\0' };
	size_t send = SDLNet_TCP_Send(socket, PING_MSG, 2);
	if (send < 2)
	{
		OnError("SDLNet_TCP_Send: " + std::string(SDLNet_GetError()));
		return false;
	}
	return true;
}