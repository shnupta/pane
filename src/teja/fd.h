#pragma once

#include <sys/poll.h>

namespace teja {

class runtime;

enum class fd_events
{
	read = POLLERR | POLLIN,
	write = POLLERR | POLLOUT,
	read_write = POLLERR | POLLIN | POLLOUT
};

class fd_handler
{
public:
	virtual ~fd_handler() = default;

	virtual void on_fd_writable(int) = 0;
	virtual void on_fd_readable(int) = 0;
	virtual void on_fd_error(int) = 0;
};

class raw_fd final
{
public:
	explicit raw_fd(int fd, runtime*, fd_handler*, fd_events);
	~raw_fd();

private:
	int _fd = -1;
	runtime* _runtime = nullptr;
	fd_handler* _handler = nullptr;
};

}
