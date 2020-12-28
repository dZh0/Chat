#pragma once
#include <SDL_net.h>
#include "../ProtoBuffer/Messages.generated.h"

namespace msg
{
	enum class type
	{
		NONE = 0,
		PING = 1,
		LOGIN_REQUEST,
		LOGIN_RESPONSE,
		NEW_CONVERSATION,
		SEND_MESSAGE_REQUEST,
		SEND_MESSAGE_RESPONSE,
		MESSAGE
	};

	// Message id type - numeric value of msg::type enum class for internal serilization
	// Note that this type must be convertable to <int>
	typedef Uint16 id;
	void WriteType(void* buffer, msg::type type);
	const msg::type ReadType(void* buffer);

	// Message size type for serilization
	typedef Uint16 size;
	void WriteSize(void* buffer, msg::size size);
	const msg::size ReadSize(void* buffer);

	// Target id type sent in proto message as sender or reciever id and mapped in client and server
	// Note that this type must be convertable to <int>
	typedef uint32_t targetId;

	// Sending messages
	bool SendPing(const TCPsocket socket);

	//@ METO: Definition in .h to create template specializations
	template<class T>
	bool Send(const TCPsocket socket, msg::type type, const T& message)
	{
		const msg::size protoSize = message.ByteSizeLong(); //@ METO: How to best catch narrow numeric cast runtime given that msg::size is user defined type and the message.ByteSizeLong() might exceed it?
		int size = sizeof(msg::id) + sizeof(msg::size) + protoSize;
		char* buffer = new char[size];
		char* pos = buffer;

		WriteType(pos, type);
		pos += sizeof(msg::id);
		WriteSize(pos, protoSize);
		pos += sizeof(msg::size);
		message.SerializeToArray(pos, protoSize);

		int bytesSent = SDLNet_TCP_Send(socket, buffer, size);
		delete[] buffer;
		return bytesSent == size;
	}


	// Reciveing messages
	//@ METO: Definition in .h to create template specializations
	template<class T>
	const T Receive(const TCPsocket socket)
	{
		msg::size size = Receive<msg::size>(socket);
		T message;
		google::protobuf::Message* msgPtr = static_cast<google::protobuf::Message*>(&message);
		int bytesRead = 0;
		if (size > 0)
		{
			char* buffer = new char[size];
			bytesRead = SDLNet_TCP_Recv(socket, buffer, size);
			msgPtr->ParseFromArray(buffer, size);
			delete[] buffer;
		}
		return message;
	}
	template<>
	const msg::type Receive<msg::type>(const TCPsocket socket);
	template<>
	const msg::size Receive<msg::size>(const TCPsocket socket);
	template<>
	const void Receive<void>(const TCPsocket socket); //@ METO: This is my "discard" function. However, I'm unsure about the "const void" return type. 
}
