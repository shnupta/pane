#include "commands.h"
#include <teja/cli/cli.h>

int main(int argc, char** argv)
{
	teja::cli cli;
	cli.parse(argc, argv);

	switch (cli.subcommand())
	{
		using sc = teja::subcommand;

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
