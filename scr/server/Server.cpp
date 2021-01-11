#include <iostream>
#include "Server.h"

ChatServer::~ChatServer()
{
	Disconnect();
}

bool ChatServer::Init(int port, int maxClients)
{
	Disconnect(); //@ METO: This shouldn't be neeed unless someone calls Init() a second time. Should I remove it?
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	serverDataBase.Open("mydb2.db");
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
		return false;
	}
	socketSet = SDLNet_AllocSocketSet(maxClients + 1);
	if (!socketSet)
	{
		std::cerr << "SDLNet_AllocSocketSet: " << SDLNet_GetError() << "\n";
		return false;
	}
	if (SDLNet_TCP_AddSocket(socketSet, listeningSocket) < 0)
	{
		std::cerr << "SDLNet_AddSocket: " << SDLNet_GetError() << "\n";
		return false;
	}
	maxConnections = maxClients;
	if (clientArr.size() > maxClients)
	{
		clientArr.resize(maxClients);
	}
	else
	{
		clientArr.reserve(maxClients);
	}
	clientIdCounter = 1;
	return true;
}

bool ChatServer::Update() //TODO: Check thoroughly
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
				std::cout << "Client CONNECTED!\n";
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
	client.lastMesageTime = SDL_GetTicks();
	client.onNotice = false;
	Uint32 nextClientCheckTime = client.lastMesageTime + NOTICE_TIME;
	if (nextActivityCheckSceduleTime > nextClientCheckTime)
	nextActivityCheckSceduleTime = (nextActivityCheckSceduleTime > nextClientCheckTime) ? nextClientCheckTime : nextActivityCheckSceduleTime;
}

void ChatServer::Disconnect()
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
	if (listeningSocket != nullptr)
	{
		SDLNet_TCP_Close(listeningSocket);
		listeningSocket = nullptr; //@ METO: Not sure if this is necesry but the state of listeningSocket is unclear in the documentation after calling SDLNet_FreeSocketSet();
		clientArr.clear();
	}
}

bool ChatServer::ReceiveMessage(ClientData& client)
{
	msg::type msgType = msg::Receive<msg::type>(client.socket);
	switch (msgType)
	{
		case msg::type::PING:
		{
			client.lastMesageTime = SDL_GetTicks();
			OnPing(client);
			break;
		}
		case msg::type::LOGIN_REQUEST:
		{
			client.lastMesageTime = SDL_GetTicks();
			LoginRequest msg = msg::Receive<LoginRequest>(client.socket);
			OnLoginRequest(client);
			LoginResponse result = LoginClient(client, msg.credentials());
			std::cout << "Login response to \"" << client.credentials << "\". . .\n";
			msg::Send(client.socket, msg::type::LOGIN_RESPONSE, result);
			break;
		}
		case msg::type::SEND_MESSAGE_REQUEST:
		{
			client.lastMesageTime = SDL_GetTicks();
			SendMessageRequest msg = msg::Receive<SendMessageRequest>(client.socket);
			std::cout << "Transfering message from " << client.name << " to " << msg.recipient_id() << " . . .\n";
			SendMessageResponse result = ForwardMessage(client, msg.recipient_id(), msg.data());
			std::cout << "Send message response to \"" << client.credentials << "\". . .\n";
			//SendProtoMessage(client.socket, message::type::SEND_MESSAGE_RESPONSE, result);
			break;
		}
		default:
		{
			std::cerr << "Unrecognised message type \n";
			DisconnectClient(client);
			return false;
		}
	}
	return true;
}

LoginResponse ChatServer::LoginClient(ClientData& client, const std::string& credentials)
{
	client.credentials = credentials;
	client.name = credentials;
	LoginResponse response = LoginResponse();
	if (true) //TODO: Credentials are not checked;
	{
		std::cout << "Log-in credentials: ACCEPPTED\n";
		AddClientToConversation(0, client);
		response.set_status(LoginResponse_Status_OK);
		//@ METO: Optimization oportunity - keep a generated LoginResponse to avoid building the conversation list on every login. Should I?
		for (auto cnv: targets) 
		{
			NewConversation& target = *response.add_conversations();
			target.set_id(cnv.first);
			target.set_name(cnv.second.name);
		}
		//Create Personal conversation;
		msg::targetId personalId = clientIdCounter++;
		client.id = personalId;
		response.set_id(personalId);
		Conversation& persoanlCnv = AddClientToConversation(personalId, client);
		for (ClientData &otherClient : clientArr)
		{
			if(&otherClient == &client)
			{
				continue;
			}
			std::cout << "Sending conversation to " << otherClient.name << "\n"; //TODO: Extend
			NewConversation newClient;
			newClient.set_id(personalId);
			newClient.set_name(persoanlCnv.name);
			msg::Send(otherClient.socket, msg::type::NEW_CONVERSATION, newClient);
		}
	}
	else
	{
		std::cout << "Log-in credentials: REJECTED\n";
		response.set_status(LoginResponse_Status_FAIL);
	}
	return response;
}

Conversation& ChatServer::AddClientToConversation(const msg::targetId id, ClientData& client)
{
	Conversation& conversation = targets.try_emplace(id, Conversation{ client.name, {}, true }).first->second;
	conversation.observers.emplace_back(&client);
	client.conversations.emplace_back(id);
	return conversation;
}

SendMessageResponse ChatServer::ForwardMessage(const ClientData& client, const msg::targetId& id, const std::string& data) const
{
	SendMessageResponse response;
	auto found = targets.find(id);
	if (found == targets.end())
	{
		response.set_status(SendMessageResponse_Status_FAIL);
		return response;
	}
	Message msg = Message();
	msg.set_sender_id(client.id);
	msg.set_recipient_id(id);
	msg.set_data(data);
	const Conversation& conv = found->second;
	for (ClientData* observer : conv.observers)
	{
		if (&client != observer)
		{
			std::cout << "   " << client.name << " -> " << observer->name << "\n";
			msg::Send(observer->socket, msg::type::MESSAGE, msg);
		}
	}
	response.set_status(SendMessageResponse_Status_OK);
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
		clientArrDirty = true;
		return;
	}
	if (SDL_GetTicks() > client.lastMesageTime + INACTIVITY_TIME)
	{
		if (client.onNotice)
		{
			std::cout << " DISCONNECTED!\n";
			DisconnectClient(client);
			return;
		}
		else
		{
			if (!msg::SendPing(client.socket))
			{
				std::cout << " DISCONNECTED!\n";
				DisconnectClient(client);
				return;
			}
			/*
			else
			{
				std::cout << "  Client_" << client.id <<" < Ping\n";
				client.onNotice = true;
			}
			*/
		}
	}
}

ClientData* ChatServer::AcceptConnection()
{
	TCPsocket clientSocket = SDLNet_TCP_Accept(listeningSocket);

	if (!clientSocket)
	{
		std::cerr << "SDLNet_TCP_Accept: " << SDLNet_GetError() << "\n";
		return nullptr;
	}
	if (SDLNet_TCP_AddSocket(socketSet, clientSocket) < 0)
	{
		std::cerr << "SDLNet_AddSocket: " << SDLNet_GetError() << "\n";
		return nullptr;
	}
	ClientData& client = clientArr.emplace_back(ClientData());
	client.socket = clientSocket;
	client.lastMesageTime = SDL_GetTicks();
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

void ChatServer::OnPing(const ClientData& client)
{
	std::cout << client.name << " 2 bytes of data: Ping\n";
}

void ChatServer::OnLoginRequest(const ClientData& client)
{
	std::cout << "Login request. . .\n";
}
