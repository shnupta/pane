#pragma once

#include "argparse.hpp"
#include <vector>

namespace pane {

enum class subcommand
{
	none,
	kill_server,
	server,
	list_sessions
};

class cli
{
public:
	explicit cli();

	void parse(int argc, char** argv);
	subcommand subcommand();

private:
	argparse::ArgumentParser _parser;
	std::vector<std::unique_ptr<argparse::ArgumentParser>> _subparsers;

	void setup_args();
	argparse::ArgumentParser& add_subparser(std::string name);
};

}
