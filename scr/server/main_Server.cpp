//// SERVER ////

#include <iostream>
#include "Server.h"


constexpr auto PORT = 1234;				// The port the server will open to listen for client connections;
constexpr auto MAX_CLIENTS = 5;			// The maximum number of clients able to connect to the server;
constexpr auto MAIN_LOOP_DELAY = 1.f;	// The delay after server completes its main loop;


int main(int argc, char* argv[])
{
	std::cout << "SDLNet_Init()...\n";
	if (!network::Init())
	{
		return EXIT_FAILURE;
	}

	ChatServer Server;
	std::cout << "Opening listening port " << PORT << "...\n";
	if (!Server.Init2(PORT, MAX_CLIENTS))
	{
		return EXIT_FAILURE;
	}

	bool isTerminated = false;
	while (!isTerminated)
	{
		if (!Server.Update(MAIN_LOOP_DELAY))
		{
			isTerminated = true;
		}
	}

	network::Quit();
	return 0;
}