#include "commands.h"
#include <pane/server/server.h>
#include <pane/client/client.h>

#include <cstring>

void spawn_or_attach()
{
	if (!pane::server::is_running())
	{
		if (pane::server::spawn()) return;
	}

	pane::client::attach();
}

void kill_server()
{
	if (!pane::server::is_running())
	{
		printf("server is not running\n");
		return;
	}

	if (!pane::server::kill())
	{
		printf("failed to kill server: %s\n", strerror(errno));
	}
}

void server_foreground()
{
	if (pane::server::is_running())
	{
		printf("server is already running\n");
		return;
	}

	pane::server::foreground();
}

void list_sessions()
{
	if (!pane::server::is_running())
	{
		printf("server is not running\n");
		return;
	}

	pane::client::perform_action(pane::client::action::list_sessions);
}
