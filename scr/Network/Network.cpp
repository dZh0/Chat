#include "Network.h"

bool network::Init()
{
	bool result = SDLNet_Init() >= 0;
	if (!result)
	{
		std::cerr << "SDLNet_Init: " << SDLNet_GetError() << "\n";
	}
	return result;
}

void network::Quit()
{
	SDLNet_Quit();
}

std::unique_ptr<network::ISocket> network::Open(int port, int setSize)
{
	IPaddress hostIp;
	if (SDLNet_ResolveHost(&hostIp, nullptr, port) < 0)
	{
		std::cerr << "SDLNet_ResolveHost(): " << SDLNet_GetError() << "\n";
		return nullptr;
	}
	auto socket = SDLNet_TCP_Open(&hostIp);
	if (!socket)
	{
		std::cerr << "SDLNet_TCP_Open(): " << SDLNet_GetError() << "\n";
		return nullptr;
	}
	auto socketSet = SDLNet_AllocSocketSet(setSize);
	if (!socketSet)
	{
		std::cerr << "SDLNet_AllocSocketSet(): " << SDLNet_GetError() << "\n";
		return nullptr;
	}
	if (SDLNet_TCP_AddSocket(socketSet, socket) < 0)
	{
		std::cerr << "SDLNet_AddSocket(): " << SDLNet_GetError() << "\n";
		return nullptr;
	}
	return std::make_unique<SDLsocket>(socket, socketSet);
}

SDLsocket::~SDLsocket()
{
	SDLNet_TCP_DelSocket(*_socketSet.get(), _socket);
}

bool SDLsocket::IsReady()
{
	int socketsToProcess = SDLNet_CheckSockets(*_socketSet.get(), 0); //@METO: Troublesome - it is called every time on each socket.
	if (socketsToProcess < 0)
	{
		std::cerr << "SDLNet_CheckSockets(): SYSTEM ERROR...\n";
	}
	if (socketsToProcess > 0)
	{
		return SDLNet_SocketReady(_socket);
	}
	return false;
}
