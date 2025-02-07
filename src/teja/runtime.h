#pragma once

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

	void add_fd(int fd, uint8_t events);

private:
	bool _stopping = false;
	runtime_result _result = runtime_result::success;

	std::vector<struct pollfd> _fds;

	void loop();

};

}
