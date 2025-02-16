#include "socket.h"
#include "../expect.h"

#include <cstring>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace pane::unix_socket {

socket::socket(enum type t)
	: _type(t)
{
	_fd = ::socket(AF_UNIX, static_cast<int>(_type), 0);
	EXPECT(_fd > 0, "failed to create socket");
}

socket::socket(int fd, enum type t)
	: _fd(fd), _type(t)
{
	EXPECT(_fd > 0, "cannot create socket from invalid existing fd");
}

socket::~socket()
{
	if (_fd > 0)
	{
		::close(_fd);
	}
	_fd = -1;

	if (!_address.empty())
	{
		::unlink(_address.c_str());
	}
}

socket::socket(socket&& other)
{
	_fd = other._fd;
	_type = other._type;
	_address = other._address;

	other._fd = -1;
	other._address.clear();
}

void socket::bind(const char* address)
{
	struct sockaddr_un sa{0};
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, address, sizeof(sa.sun_path) - 1);

	int res = ::bind(_fd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(struct sockaddr_un));
	EXPECT(res != -1, "unix socket failed to bind");
	_address = address;
}

void socket::listen()
{
	::listen(_fd, SOMAXCONN);
}

socket socket::accept()
{
	struct sockaddr_un client_addr;
	socklen_t len = sizeof(client_addr);
	int fd = ::accept(_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &len);
	return socket(fd, _type);
}

int socket::fd() const
{
	return _fd;
}

type socket::type() const
{
	return _type;
}

const char* socket::addr() const
{
	return _address.c_str();
}

int socket::recv(char* buf, size_t buf_size)
{
	return ::recv(_fd, buf, buf_size, 0);
}

bool socket::set_non_blocking()
{
	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags == -1) return false;
	return fcntl(_fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

int socket::connect(const char* address)
{
	struct sockaddr_un server_addr{};
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, address, sizeof(server_addr.sun_path) - 1);

	return ::connect(_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));
}

ssize_t socket::send(const char* data, size_t size)
{
	return ::send(_fd, data, size, 0);
}

}
