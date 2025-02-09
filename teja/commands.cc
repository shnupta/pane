#include "commands.h"
#include <teja/server/server.h>
#include <teja/client/client.h>

#include <cstring>

void spawn_or_attach()
{
	if (!teja::server::is_running())
	{
		if (teja::server::spawn()) return;
	}

	// todo: spin and timeout whilst attach fails??
	teja::client::attach();
}

void kill_server()
{
	if (!teja::server::is_running())
	{
		printf("server is not running\n");
		return;
	}

	if (!teja::server::kill())
	{
		printf("failed to kill server: %s\n", strerror(errno));
	}
}

void server_foreground()
{
	if (teja::server::is_running())
	{
		printf("server is already running\n");
		return;
	}

	teja::server::foreground();
}
