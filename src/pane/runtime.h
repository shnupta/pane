#pragma once

#include "fd.h"

#include <chrono>
#include <vector>
#include <poll.h>

namespace pane {

enum class runtime_result
{
	success = 0,
	failure = 1
};

// event loop.
class runtime
{
public:
	using callback = std::function<void()>;

	runtime_result run();
	void stop();

	void register_fd(int fd, fd_handler*, fd_events);
	void update_fd(int fd, fd_events);
	void remove_fd(int fd);
	// todo: make these return some identifier so we can safely remove them
	void post_callback(callback);
	void post_callback(std::chrono::nanoseconds in, callback);

	// todo: signal handlers
	// todo: callbacks / timers

private:
	bool _stopping = false;
	runtime_result _result = runtime_result::success;
	std::chrono::steady_clock::time_point _next_poll_time{};

	struct callback_info
	{
		std::chrono::steady_clock::time_point fire_at;
		callback cb;
	};

	std::vector<struct pollfd> _fds;
	std::vector<fd_handler*> _fd_handlers;
	std::vector<callback_info> _post_callbacks;

	void loop();
	bool work_to_do() const;

};

}
