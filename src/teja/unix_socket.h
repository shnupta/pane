#pragma once

#include "runtime.h"

#include <sys/socket.h>

namespace teja {

class unix_socket
{
public:	
	enum class type
	{
		stream = SOCK_STREAM
	};

	explicit unix_socket(type);
	~unix_socket();

	void bind(const char* address);
	void listen(runtime*);

private:
	type _type;
	int _fd = -1;
	const char* _address = nullptr;
	runtime* _runtime = nullptr;
};

}
