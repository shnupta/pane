#pragma once

#include <sys/socket.h>
#include <string>

namespace teja::unix_socket {

enum class type
{
	stream = SOCK_STREAM
};


class socket
{
public:
	explicit socket(type t);
	explicit socket(int fd, type t);
	socket(const socket&) = delete;
	socket(socket&&);
	~socket();

	void bind(const char* address);
	void listen();
	socket accept();
	int fd() const;
	type type() const;
	const char* addr() const;
	bool set_non_blocking();

	int recv(char* buf, size_t buf_size);

private:
	enum type _type;
	int _fd = -1;
	std::string _address;
};
	
}
