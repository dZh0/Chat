#pragma once
#include "../ProtoBuffer/Messages.generated.h"

namespace message
{
	enum class type
	{
		ALL = -1,
		PING = 0,
		LOGIN_REQUEST,
		LOGIN_RESPONSE,
		NEW_CONVERSATION,
		SEND_MESSAGE_REQUEST,
		SEND_MESSAGE_RESPONSE,
		MESSAGE
	};

	typedef uint32_t idType;	// @METO: I tried Uint16 serilizesd with SDLNet_Write16() in a protobuffer::byte field and while worked from sigle module, the data is somehow lost when transfering through the network.
								// The intention being that only the server would manage those values while the clients would only use == operator;
								// Now it uses Uint32 in a protobuffer::fixed32 field for simplicity;
}