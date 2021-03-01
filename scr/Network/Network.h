#pragma once
#include <memory>
#include <SDL_net.h>
#include "..\Server\Server.h"

class SDLsocket final : public network::ISocket
{
public:
	SDLsocket(TCPsocket &socket, SDLNet_SocketSet &socketSet) :
		_socket(socket),
		_socketSet(&socketSet, [](SDLNet_SocketSet *set) { SDLNet_FreeSocketSet(*set); })
	{};
	~SDLsocket();

	bool IsReady() override;
private:
	friend std::unique_ptr<network::ISocket> network::Open(int port, int maxClients);

	const TCPsocket _socket;
	std::shared_ptr<SDLNet_SocketSet> _socketSet;
};

