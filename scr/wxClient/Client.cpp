#include <iostream>
#include "Client.h"


bool ChatClient::Init()
{
	if (SDLNet_Init() < 0)
	{
		errorMessage = "SDLNet_Init: " + (std::string)SDLNet_GetError();
		return false;
	}
    return true;
}

bool ChatClient::ConnectTo(std::string host, Uint16 port)
{
	IPaddress hostIp;
	if (SDLNet_ResolveHost(&hostIp, host.c_str(), port) < 0)
	{
		std::cerr << "SDLNet_ResolveHost: " << SDLNet_GetError() << "\n";
		return false;
	}
	TCPsocket clientSocket = SDLNet_TCP_Open(&hostIp);
	if (!clientSocket)
	{
		std::cerr << "SDLNet_TCP_Open: " << SDLNet_GetError() << "\n";
		return false;
	}
	socketSet = SDLNet_AllocSocketSet(1);
	if (!socketSet)
	{
		std::cerr << "SDLNet_AllocSocketSet: " << SDLNet_GetError() << "\n";
		return false;
	}
	if (SDLNet_TCP_AddSocket(socketSet, clientSocket) < 1)
	{
		std::cerr << "SDLNet_AddSocket: " << SDLNet_GetError() << "\n";
		return false;
	}
	socket = clientSocket;
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	isConnected = true;
	return true;
}

bool ChatClient::RequestLogIn(const std::string& credentials)
{
	bool result = false;
	LoginRequest LoginRequestMsg = LoginRequest();
	LoginRequestMsg.set_credentials((std::string)credentials);
	if (SendProtoMessage(socket, message::type::LOGIN_REQUEST, LoginRequestMsg))
	{
		result = true;
	}
	return result;
}


bool ChatClient::Update()
{
	if (SDLNet_Read16(id) == 0)
	{
		std::cout << "Requesting login for \"" << (std::string)credentials << "\". . .\n";
		RequestLogIn(credentials);
	}
	int socketsToProcess = SDLNet_CheckSockets(socketSet, ACTIVITY_CHECK_TIMEOUT);
	if (socketsToProcess < 0)
	{
		std::cerr << "SDLNet_CheckSockets: " << SDLNet_GetError() << "\n";
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
	if (socket != nullptr)
	{
		SDLNet_TCP_DelSocket(socketSet, socket);
		SDLNet_FreeSocketSet(socketSet);
		socketSet = nullptr;
		SDLNet_TCP_Close(socket);
		socket = nullptr;
		isConnected = false;
	}
}

bool ChatClient::ReceiveMessage()
{
	int msgType = ReceiveSint16(socket);
	if (msgType < 0)
	{
		Disconnect();
		return false;
	}
	std::cout << "Receiveing message type [" << msgType << "] ";
	switch((message::type)msgType)
	{
		case message::type::PING:
		{
			std::cout << "2 bytes of data...\n";
			isOnNotice = true;
			break;
		}
		case message::type::LOGIN_RESPONSE:
		{
			LoginResponse msg = ReceiveProtoMessage<LoginResponse>(socket);
			HandleMessage(msg);
			break;
		}
		case message::type::SEND_MESSAGE_RESPONSE:
		{
			SendMessageResponse msg = ReceiveProtoMessage<SendMessageResponse>(socket);
			HandleMessage(msg);
			break;
		}
		case message::type::MESSAGE:
		{
			Message msg = ReceiveProtoMessage<Message>(socket);
			HandleMessage(msg);
			break;
		}
		default:
		{
			std::cerr << "Unrecognised message type!\n";
			return false;
		}
	}

	return true;
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

bool ChatClient::HandleMessage(const LoginResponse& message)
{
	if (message.status() == message.OK)
	{
		memcpy(id, message.id().c_str(), sizeof(Uint16));
		std::cerr << "Log-in successful. ID "<< SDLNet_Read16(id) <<"\n";
		return true;
	}
	else
	{
		std::cerr << "Log-in request failed!\n";
		//TODO: Handle failed log-in attempt...
		return false;
	}
}

bool ChatClient::HandleMessage(const SendMessageResponse& message) const
{
	if (message.OK)
	{
		return true;
	}
	else
	{
		std::cerr << "Sending message failed!\n";
		//TODO: Handle failed "send message" attempt...
		return false;
	}
}

bool ChatClient::HandleMessage(const Message& message) const
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
		std::cerr << "SDLNet_TCP_Send: " << SDLNet_GetError() << "\n";
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
		std::cerr << "SDLNet_TCP_Send: " << SDLNet_GetError() << "\n";
		return false;
	}
	return true;
}