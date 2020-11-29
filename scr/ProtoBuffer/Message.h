#pragma once
#include "../ProtoBuffer/Messages.generated.h"

namespace message
{
	enum class type
	{
		ALL = -1,
		PING = 0,
		LOGIN_REQUEST = 1,
		LOGIN_RESPONSE = 2,
		SEND_MESSAGE_REQUEST = 3,
		SEND_MESSAGE_RESPONSE = 4,
		MESSAGE = 5
	};
}