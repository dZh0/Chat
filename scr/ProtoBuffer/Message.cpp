#include "Message.h"

void msg::WriteType(void* buffer, msg::type type)
{
	msg::id id = static_cast<msg::id>(type);
	SDLNet_Write16(id, buffer);
}
const msg::type msg::ReadType(void* buffer)
{
	msg::id id = (msg::id)SDLNet_Read16(buffer);
	return (msg::type)id;
}
void msg::WriteSize(void* buffer, msg::size size)
{
	SDLNet_Write16(size, buffer);
}
const msg::size msg::ReadSize(void* buffer)
{
	return (msg::size)SDLNet_Read16(buffer);
}

bool msg::SendPing(const TCPsocket socket)
{
	const char pingMsg[] = { '\0','\0' };
	const int pingSize = 2;
	int bytesSent = 0;
	bytesSent = SDLNet_TCP_Send(socket, pingMsg, pingSize);
	return !(SDLNet_TCP_Send(socket, pingMsg, pingSize) < pingSize);
}

template<>
const msg::type msg::Receive<msg::type>(const TCPsocket socket)
{
	char* buffer = new char[sizeof(msg::id)];
	msg::type result = msg::type::NONE;
	if (SDLNet_TCP_Recv(socket, buffer, sizeof(msg::id)) == sizeof(msg::id))
	{
		result = ReadType(buffer);
	}
	delete[] buffer;
	return result;
}

template<>
const msg::size msg::Receive<msg::size>(const TCPsocket socket)
{
	char* buffer = new char[sizeof(msg::size)];
	msg::size result = 0;
	if (SDLNet_TCP_Recv(socket, buffer, sizeof(msg::size)) == sizeof(msg::size))
	{
		result = ReadSize(buffer);
	}
	delete[] buffer;
	return result;
}

template<>
const void msg::Receive<void>(const TCPsocket socket)
{
	msg::size size = Receive<msg::size>(socket);
	if (size > 0)
	{
		char* buffer = new char[size];
		SDLNet_TCP_Recv(socket, buffer, size);
		delete[] buffer;
	}
}

