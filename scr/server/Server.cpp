#include <iostream>
#include "Server.h"


ChatServer::~ChatServer()
{
	Disconnect();
}

bool ChatServer::Init(int port, int _maxConnections)
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if (!OpenPort(port))
	{
		return false;
	}
	if (!SetMaxConnections(_maxConnections))
	{
		return false;
	}
	clientIdCounter = 1;
	return true;
}


bool ChatServer::Update()
{
	// Check sockets
	int socketsToProcess = SDLNet_CheckSockets(socketSet, ACTIVITY_CHECK_TIMEOUT);
	if (socketsToProcess < 0)
	{
		std::cerr << "SDLNet_CheckSockets: " << SDLNet_GetError() << "\n";
		return false;
	}
	// Handle incoming connections
	if (socketsToProcess > 0 && clientArr.size() < maxConnections)
	{
		if (SDLNet_SocketReady(listeningSocket))
		{
			std::cout << "Incoming connection...\n";
			ClientData* client = AcceptConnection();
			if (client)
			{
				std::cout << "Client_" << client->id << " CONNECTED!\n";
			}
		}
	}
	// Scedule next activity check
	bool pendingActivityCheck = nextActivityCheckSceduleTime < SDL_GetTicks();
	if(pendingActivityCheck)
	{
		nextActivityCheckSceduleTime = 0xFFFFFFFF;
	}
	// Handle client messages
	for (ClientData& client : clientArr)
	{
		if (client.socket == nullptr)
		{
			continue;
		}
		if (socketsToProcess > 0)
		{
			if (SDLNet_SocketReady(client.socket))
			{
				ReceiveMessage(client);
				OnMessageReceived(client);
				--socketsToProcess;
			}
		}
		if (socketsToProcess <= 0 && !pendingActivityCheck)
		{
			break;
		}
		// Activity check
		if (pendingActivityCheck)
		{
			CheckForInactivity(client);
		}
	}
	// Delete disconnected clients
	if (clientArrDirty)
	{
		std::cout << "Cleaning disconnected clients...\n";
		DeleteDisconnectdClients();
	}
	return true;
}

void ChatServer::OnMessageReceived(ClientData& client)
{
	client.checkTime = SDL_GetTicks() + INACTIVITY_TIME - NOTICE_TIME;;
	client.onNotice = false;
	nextActivityCheckSceduleTime = (nextActivityCheckSceduleTime > client.checkTime) ? client.checkTime : nextActivityCheckSceduleTime;
}
void ChatServer::Disconnect()
{
	if (listeningSocket != nullptr)
	{
		ClearSocketSet();
		SDLNet_TCP_Close(listeningSocket);
		listeningSocket = nullptr;
		clientArr.clear();
	}
}

bool ChatServer::OpenPort(int port)
{
	Disconnect();
	IPaddress hostIp;
	if (SDLNet_ResolveHost(&hostIp, nullptr, port) < 0)
	{
		std::cerr << "SDLNet_ResolveHost: " << SDLNet_GetError() << "\n";
		return false;
	}
	listeningSocket = SDLNet_TCP_Open(&hostIp);
	if (!listeningSocket)
	{
		std::cerr << "SDLNet_TCP_Open: " << SDLNet_GetError() << "\n";
		listeningSocket = nullptr;
		return false;
	}
	return true;
}

bool ChatServer::SetMaxConnections(int _maxConnections)
{
	if (maxConnections == _maxConnections)
	{
		return true;
	}
	maxConnections = _maxConnections;
	ClearSocketSet();
	socketSet = SDLNet_AllocSocketSet(maxConnections + 1);
	if (!socketSet)
	{
		std::cerr << "SDLNet_AllocSocketSet: " << SDLNet_GetError() << "\n";
		return false;
	}
	if (clientArr.size() > maxConnections)
	{
		clientArr.resize(maxConnections);
	}
	else
	{
		clientArr.reserve(maxConnections);
	}
	if(FillSocketSet() < 0)
	{
		return false;
	}
	return true;
}

void ChatServer::ClearSocketSet()
{
	if (socketSet != nullptr)
	{
		SDLNet_TCP_DelSocket(socketSet, listeningSocket);
		for (ClientData& client : clientArr)
		{
			SDLNet_TCP_DelSocket(socketSet, client.socket);
		}
		SDLNet_FreeSocketSet(socketSet);
		socketSet = nullptr;
	}
}

int ChatServer::FillSocketSet()
{
	int usedSockets = SDLNet_TCP_AddSocket(socketSet, listeningSocket);
	if (usedSockets < 0)
	{
		std::cerr << "SDLNet_AddSocket: " << SDLNet_GetError() << "\n";
		return usedSockets;
	}
	for (ClientData& client : clientArr)
	{
		if (usedSockets < maxConnections + 1)
		{
			usedSockets = SDLNet_TCP_AddSocket(socketSet, client.socket);
			if ( usedSockets < 0)
			{
				std::cerr << "SDLNet_AddSocket: " << SDLNet_GetError() << "\n";
				return usedSockets;
			}
		}
		else
		{
			return usedSockets;
		}
	}
	return usedSockets;
}

bool ChatServer::ReceiveMessage(ClientData& client)
{
	int msgType = ReceiveSint16(client.socket);
	if (msgType < 0)
	{
		std::cout << client.credentials << " DISCONNECTED!\n";
		DisconnectClient(client);
		return false;
	}
	std::cout << "Receiveing message type [" << msgType << "] ";
	switch ((message::type)msgType)
	{
		case message::type::PING:
		{
			std::cout << "2 bytes of data: Ping\n";
			bool isOnNotice = true;
			break;
		}
		case message::type::LOGIN_REQUEST:
		{
			LoginRequest msg = ReceiveProtoMessage<LoginRequest>(client.socket);
			std::cout << "Login request from \"" << msg.credentials() << "\". . .\n";
			LoginResponse result = LoginClient(client, msg.credentials());
			std::cout << "Login response to \"" << client.credentials << "\". . .\n";
			SendProtoMessage(client.socket, message::type::LOGIN_RESPONSE, result);
			break;
		}
		case message::type::SEND_MESSAGE_REQUEST:
		{
			SendMessageRequest msg = ReceiveProtoMessage<SendMessageRequest>(client.socket);
			std::cout << "Transfering message from " << client.credentials << " to " << msg.recipient_id() << " . . .\n";
			SendMessageResponse result = ForwardMessage(client.id, msg.recipient_id(), msg.data());
			std::cout << "Send message response to \"" << client.credentials << "\". . .\n";
			SendProtoMessage(client.socket, message::type::SEND_MESSAGE_RESPONSE, result);
			break;
		}
		default:
		{
			std::cerr << "Unrecognised message type \n";
			return false;
		}
	}
	return true;
}

int ChatServer::ReceiveSint16(const TCPsocket socket) const
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
const T ChatServer::ReceiveProtoMessage(const TCPsocket socket) const
{
	int msgSize = ReceiveSint16(socket);
	std::cout << msgSize << " bytes of data:\n";
	T message;
	if (msgSize > 0)
	{
		char* buffer = new char[msgSize];
		int bytesRead = SDLNet_TCP_Recv(socket, buffer, msgSize);
		message.ParseFromArray(buffer, msgSize); // METO: This seems wrong as message might not have ParseFromArray() method.
		delete[] buffer;
	}
	return message;
}

template<class T>
bool ChatServer::SendProtoMessage(const TCPsocket socket, message::type msgType, const T& message) const
{
	int protoSize = message.ByteSizeLong();	 //TO METO: This seems wrong as message might not have ByteSizeLong() method.
	int msgSize = sizeof(Uint16) * 2 + protoSize;
	char* buffer = new char[msgSize];

	SDLNet_Write16((Uint16)msgType, buffer);
	SDLNet_Write16(protoSize, buffer + sizeof(Uint16));
	message.SerializeToArray(buffer + sizeof(Uint16) * 2, protoSize);  // METO: This seems wrong as message might not have ParseFromArray() method.

	std::cout << "Sending " << msgSize << " bytes of data...\n";
	size_t send = SDLNet_TCP_Send(socket, buffer, msgSize);
	if (send < msgSize)
	{
		std::cerr << "SDLNet_TCP_Send: " << SDLNet_GetError() << "\n";
		return false;
	}
	delete[] buffer;
	return true;
}

LoginResponse ChatServer::LoginClient(ClientData& client, const std::string& credentials)
{
	LoginResponse response = LoginResponse();
	if (CheckCredentials(credentials))
	{
		client.credentials = credentials;
		std::cout << "Log-in credentials: ACCEPPTED\n";
		response.set_status(LoginResponse_Status_OK);
		SDLNet_Write16(clientIdCounter, client.id);
		std::cout << "ID assigned: " << clientIdCounter << "\n";
		++clientIdCounter;
		response.set_id(client.id, sizeof(Uint16));
	}
	else
	{
		std::cout << "Log-in credentials: REJECTED\n";
		response.set_status(LoginResponse_Status_FAIL);
	}
	return response;
}

// This should be overriden to contain meaningful functionality. (For now it returns "true")
bool ChatServer::CheckCredentials(const std::string& credentials)
{
    return true;
}

SendMessageResponse ChatServer::ForwardMessage(const std::string& senderID, const std::string& targetID, const std::string& data) const
{
	TCPsocket target = FindClient(targetID);
	SendMessageResponse response;
	if (target == nullptr)
	{
		response.set_status(SendMessageResponse_Status_FAIL);
		return response;
	}
	Message msg = Message();
	msg.set_sender_id(senderID);
	msg.set_data(data);
	if(SendProtoMessage(target, message::type::MESSAGE, msg))
	{
		response.set_status(SendMessageResponse_Status_OK);
	}
	else
	{
		response.set_status(SendMessageResponse_Status_FAIL);
	}
	return response;
}

// TODO: Fix to contain meaningful functionality. (For now it returns socket of the second connected client)
const TCPsocket ChatServer::FindClient(const std::string& id) const
{
	if (clientArr.size() > 1)
	{
		return clientArr[1].socket;
	}
	else
	{
		return nullptr;
	}
}

void ChatServer::CheckForInactivity(ClientData& client)
{
	if (client.socket == nullptr)
	{
		return;
	}
	if (SDL_GetTicks() > client.checkTime)
	{
		if (client.onNotice)
		{
			std::cout << " DISCONNECTED!\n";
				DisconnectClient(client);
				return;
		}
		else
		{
			if (!Ping(client))
			{
				std::cout << " DISCONNECTED!\n";
				DisconnectClient(client);
				return;
			}
			else
			{
				std::cout << "  Client_" << client.id <<" < Ping\n";
				client.onNotice = true;
				client.checkTime = SDL_GetTicks() + NOTICE_TIME;
			}
		}
	}
	nextActivityCheckSceduleTime = (nextActivityCheckSceduleTime > client.checkTime) ? client.checkTime : nextActivityCheckSceduleTime;
}

ClientData* ChatServer::AcceptConnection()
{
	TCPsocket clientSocket = SDLNet_TCP_Accept(listeningSocket);

	if (!clientSocket)
	{
		std::cerr << "SDLNet_TCP_Accept: " << SDLNet_GetError() << "\n";
		return nullptr;
	}

	int usedSockets = SDLNet_TCP_AddSocket(socketSet, clientSocket);
	if (usedSockets < 0)
	{
		std::cerr << "SDLNet_AddSocket: " << SDLNet_GetError() << "\n";
		return nullptr;
	}

	ClientData& client = clientArr.emplace_back(ClientData());
	client.socket = clientSocket;
	client.checkTime = SDL_GetTicks() + INACTIVITY_TIME - NOTICE_TIME;
	nextActivityCheckSceduleTime = (nextActivityCheckSceduleTime > client.checkTime) ? client.checkTime : nextActivityCheckSceduleTime;
	return &client;
}

void ChatServer::DisconnectClient(ClientData& client)
{
	SDLNet_TCP_DelSocket(socketSet, client.socket);
	SDLNet_TCP_Close(client.socket);
	client.socket = nullptr;
	clientArrDirty = true;
}

void ChatServer::DeleteDisconnectdClients()
{
	clientArr.erase
	(
		std::remove_if
		(
			clientArr.begin(),
			clientArr.end(),
			[](const ClientData& client)
			{
				return client.socket == nullptr;
			}
		),
		clientArr.end()
	);
	clientArrDirty = false;
}

bool ChatServer::Ping(ClientData& client) const
{
	std::cout << "Sending 2 bytes...\n";
	const char PING_MSG[] = { '\0','\0' };
	size_t send = SDLNet_TCP_Send(client.socket, PING_MSG, 2);
	if (send < 2)
	{
		std::cerr << "SDLNet_TCP_Send: " << SDLNet_GetError() << "\n";
		return false;
	}
	return true;
}