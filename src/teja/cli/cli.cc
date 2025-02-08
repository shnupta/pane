#include "cli.h"

namespace teja {

namespace {

const std::string g_program_name = "teja";
const std::string g_version = "0.0.1";

constexpr const char* k_kill_server_cmd = "kill-server";
constexpr const char* k_kill_server_description = "kill the running server";

constexpr const char* k_server_cmd = "server";
constexpr const char* k_server_description = "run the server in the foreground";

}

cli::cli()
	// todo: default arguments is to spawn a new server
	: _parser(g_program_name, g_version)
{
	setup_args();
}

void cli::setup_args()
{
	auto& kill_server_cmd = add_subparser(k_kill_server_cmd);
	kill_server_cmd.add_description(k_kill_server_description);

	auto& server_cmd = add_subparser(k_server_cmd);
	server_cmd.add_description(k_server_description);

	_parser.add_subparser(kill_server_cmd);
	_parser.add_subparser(server_cmd);
}

argparse::ArgumentParser& cli::add_subparser(std::string name)
{
	return *_subparsers.emplace_back(std::make_unique<argparse::ArgumentParser>(name)).get();
}

void cli::parse(int argc, char** argv)
{
	_parser.parse_args(argc, argv);
}

subcommand cli::subcommand()
{
	if (_parser.is_subcommand_used(k_kill_server_cmd))
		return subcommand::kill_server;

	if (_parser.is_subcommand_used(k_server_cmd))
		return subcommand::server;

	return subcommand::none;
}

}
