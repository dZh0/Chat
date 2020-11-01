//// SERVER ////

#include <iostream>
#include "Server.h"


constexpr int PORT = 1234;				// The port the server will open to listen for client connections;
constexpr int MAX_CLIENTS = 5;			// The maximum number of clients able to connect to the server;
constexpr Uint32 MAIN_LOOP_DELAY = 800;	// The delay after the server completes its main loop;


int main(int argc, char* argv[])
{
	std::cout << "SDLNet_Init()...\n";
	if (SDLNet_Init() < 0)
	{
		std::cerr << "SDLNet_Init: " << SDLNet_GetError() << "\n";
		return EXIT_FAILURE;
	}

	ChatServer Server;
	std::cout << "Opening listening port " << PORT << "...\n";
	if (!Server.Init(PORT, MAX_CLIENTS))
	{
		return EXIT_FAILURE;
	}

	// Main server loop
	bool isTerminated = false;
	while (!isTerminated)
	{
		if (!Server.Update())
		{
			isTerminated = true;
		}
		else
		{
			SDL_Delay(MAIN_LOOP_DELAY);
		}
	}
	SDLNet_Quit();
	return 0;
}