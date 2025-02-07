#include "unix_socket.h"
#include "expect.h"

#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace teja {

unix_socket::unix_socket(type t)
	: _type(t)
{
	_fd = ::socket(AF_UNIX, static_cast<int>(_type), 0);
	EXPECT(_fd > 0, "failed to create socket");
}

unix_socket::~unix_socket()
{
	if (_fd > 0)
	{
		::close(_fd);
	}
	_fd = -1;

	if (_address)
	{
		::unlink(_address);
	}
}

void unix_socket::bind(const char* address)
{
	struct sockaddr_un sa{0};
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, address, strlen(address));

	int res = ::bind(_fd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(struct sockaddr_un));
	EXPECT(res != -1, "unix socket failed to bind");
	_address = address;
}

void unix_socket::listen(runtime* r)
{
	::listen(_fd, SOMAXCONN);
	r->add_fd(_fd, POLLIN | POLLOUT | POLLERR);
}

}
