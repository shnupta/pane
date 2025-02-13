#include "fd.h"
#include "runtime.h"

namespace teja {

raw_fd::raw_fd(int fd, runtime* r, fd_handler* handler, fd_events events)
	: _fd(fd), _runtime(r)
{
	_runtime->register_fd(_fd, handler, events);
}

raw_fd::~raw_fd()
{
	_runtime->remove_fd(_fd);
}

}
