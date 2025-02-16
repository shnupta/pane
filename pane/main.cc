#include "commands.h"
#include <pane/cli/cli.h>

int main(int argc, char** argv)
{
	pane::cli cli;
	cli.parse(argc, argv);

	switch (cli.subcommand())
	{
		using sc = pane::subcommand;

		case sc::none:
			spawn_or_attach();
			break;
		case sc::kill_server:
			kill_server();
			break;
		case sc::server:
			server_foreground();
			break;
		case sc::list_sessions:
			list_sessions();
			break;
	}

	return 0;
}
