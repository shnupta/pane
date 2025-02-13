#include "pane.h"
#include "window.h"
#include "../expect.h"
#include "spdlog/spdlog.h"

#include <cstdlib>
#include <string>
#include <sys/fcntl.h>
#include <unistd.h>

namespace teja {

pane::pane(runtime* r, window* window, size_t id)
	: _runtime(r), _window(window), _id(id), _name(std::to_string(id))
{
	_pty_parent = posix_openpt(O_RDWR | O_NOCTTY);
	EXPECT(_pty_parent != -1, "failed to open parent pty");
	EXPECT(grantpt(_pty_parent) != -1, "grantpt failed");
	EXPECT(unlockpt(_pty_parent) != -1, "unlockpt failed");

	_child_name = ptsname(_pty_parent);
	EXPECT(_child_name != nullptr, "ptsname failed");

	pid_t pid = fork();
	if (pid == 0)
	{
		setup_child_pty();
	}
	else
	{
		setup_parent_pty();
	}
}

pane::~pane()
{

}

void pane::setup_child_pty()
{
	setsid();
	_pty_child = open(_child_name, O_RDWR);
	EXPECT(_pty_child != -1, "failed to open child pty");

	dup2(_pty_child, STDIN_FILENO);
	dup2(_pty_child, STDOUT_FILENO);
	dup2(_pty_child, STDERR_FILENO);

	close(_pty_child);
	close(_pty_parent);

	execlp("sh", "sh", nullptr);
}

void pane::setup_parent_pty()
{
	_runtime->register_fd(_pty_parent, this, fd_events::read);
}

void pane::on_fd_readable(int)
{
	SPDLOG_INFO("pty {} readable", _pty_parent);
	char buf[1 << 16];
	int res = ::read(_pty_parent, buf, sizeof(buf));

	EXPECT(res > 0, "read failed");
	broadcast_terminal_output(buf, res);
}

void pane::broadcast_terminal_output(char* data, int size)
{
	_window->broadcast_terminal_output(_id, data, size);
}

void pane::on_fd_writable(int)
{

}

void pane::on_fd_error(int)
{

}

void pane::consume_input(const char* data, size_t size)
{
	::write(_pty_parent, data, size);
}

}
