#include <exception>
#include <teja/cli.h>
#include <teja/server.h>

int main(int argc, char** argv)
{
	teja::cli cli;

	try
	{
		teja::server server;
	}
	catch (const std::exception& e)
	{
		printf("uncaught exception: %s", e.what());
	}

	return 0;
}
