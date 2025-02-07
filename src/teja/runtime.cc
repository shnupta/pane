#include "runtime.h"
#include "expect.h"

#include <csignal>
#include <sys/poll.h>

namespace teja {

namespace {

std::atomic<bool> stop_requested{false};

void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        stop_requested = true; // Set the flag
    }
}

void setup_signal_handler()
{
    struct sigaction sa {};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

}

runtime_result runtime::run()
{
	setup_signal_handler();
	_stopping = false;

	loop();

	return _result;
}

void runtime::add_fd(int fd, uint8_t events)
{
	struct pollfd pfd{fd, events, 0};
	_fds.push_back(pfd);
}

void runtime::loop()
{
	while (!_stopping && !stop_requested)
	{
		int poll_res = ::poll(_fds.data(), _fds.size(), 0);
		if (poll_res < 0)
		{
			if (errno == EINTR)
				break;
			EXPECT(false, "poll failed");
		}

		if (poll_res == 0) continue;

		for (auto& fd : _fds)
		{
			if (fd.revents & POLLIN)
			{
				printf("read");
			}
			
			if (fd.revents & POLLOUT)
			{
				printf("write");
			}

			if (fd.revents & POLLERR)
			{
				printf("error");
			}
		}
	}
}

}
