#pragma once

#include <sys/poll.h>
namespace teja {

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

	virtual void on_fd_writable() = 0;
	virtual void on_fd_readable() = 0;
	virtual void on_fd_error() = 0;
};

}
