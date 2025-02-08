#include "runtime.h"
#include "expect.h"

#include <algorithm>
#include <csignal>
#include <iterator>
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

void runtime::register_fd(int fd, fd_handler* handler, fd_events events)
{
	struct pollfd pfd{fd, static_cast<uint8_t>(events), 0};
	_fds.push_back(pfd);
	_fd_handlers.push_back(handler);
}

void runtime::remove_fd(int fd)
{
	auto it = std::find_if(_fds.begin(), _fds.end(), [fd](const struct pollfd& pfd) {
			return pfd.fd == fd;
			});
	if (it == _fds.end()) return;

	auto idx = std::distance(_fds.begin(), it);
	auto handler_it = _fd_handlers.begin() + idx;

	_fds.erase(it);
	_fd_handlers.erase(handler_it);
}

void runtime::loop()
{
	while (!_stopping && !stop_requested)
	{
		if (!work_to_do())
		{
			_stopping = true;
			break;
		}

		int poll_res = ::poll(_fds.data(), _fds.size(), 0);
		if (poll_res < 0)
		{
			if (errno == EINTR)
				break;
			EXPECT(false, "poll failed");
		}

		if (poll_res == 0) continue;

		// copy in case fds are added or removed during iteration
		auto fds = _fds;
		auto fd_handlers = _fd_handlers;
		for (auto i = 0; i < fds.size(); ++i)
		{
			auto fd = fds.at(i);
			auto* handler = fd_handlers.at(i);
			if (fd.revents & POLLIN)
			{
				handler->on_fd_readable();
			}
			
			if (fd.revents & POLLOUT)
			{
				handler->on_fd_writable();
			}

			if (fd.revents & POLLERR)
			{
				handler->on_fd_error();
			}
		}
	}
}

bool runtime::work_to_do() const
{
	return _fds.size() > 0;
}

}
