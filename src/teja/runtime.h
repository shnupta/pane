#pragma once

#include "fd.h"

#include <vector>
#include <poll.h>

namespace teja {

enum class runtime_result
{
	success = 0,
	failure = 1
};

// event loop.
class runtime
{
public:

	runtime_result run();

	void register_fd(int fd, fd_handler*, fd_events);
	void remove_fd(int fd);

private:
	bool _stopping = false;
	runtime_result _result = runtime_result::success;

	std::vector<struct pollfd> _fds;
	std::vector<fd_handler*> _fd_handlers;

	void loop();
	bool work_to_do() const;

};

}
