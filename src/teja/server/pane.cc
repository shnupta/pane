#include "pane.h"
#include "../expect.h"
#include "spdlog/spdlog.h"

#include <cstdlib>
#include <sys/fcntl.h>
#include <unistd.h>

namespace teja {

pane::pane(runtime* r)
	: _runtime(r)
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

void pane::on_fd_readable()
{
	SPDLOG_INFO("pty {} readable", _pty_parent);
	char buf[1024];
	int res = ::read(_pty_parent, buf, sizeof(buf));

	EXPECT(res > 0, "read failed");
	SPDLOG_INFO("read from master pty: {}", buf);
}

void pane::on_fd_writable()
{

}

void pane::on_fd_error()
{

}

}
