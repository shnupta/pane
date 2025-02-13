#pragma once

#include "fd.h"

#include <chrono>
#include <vector>
#include <poll.h>

namespace teja {

enum class runtime_result
{
	success = 0,
	failure = 1
};

// event loop.
// todo: make this friendlier to the cpu :)
class runtime
{
public:

	runtime_result run();
	void stop();

	void register_fd(int fd, fd_handler*, fd_events);
	void update_fd(int fd, fd_events);
	void remove_fd(int fd);

private:
	bool _stopping = false;
	runtime_result _result = runtime_result::success;
	std::chrono::steady_clock::time_point _next_poll_time{};

	std::vector<struct pollfd> _fds;
	std::vector<fd_handler*> _fd_handlers;

	void loop();
	bool work_to_do() const;

};

}
