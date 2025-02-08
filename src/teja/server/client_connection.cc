#include "client_connection.h"
#include "server.h"
#include "../expect.h"

#include <cstdlib>
#include <sys/fcntl.h>
#include <unistd.h>

namespace teja {

client_connection::client_connection(server* server, std::unique_ptr<unix_socket::connection> conn)
	: _server(server), _connection(std::move(conn))
{
	_connection->set_handler(this);
}

void client_connection::on_disconnected()
{
	printf("client disconnected\n");
	_connection.reset();
	_server->client_disconnected(this);
}

void client_connection::on_data(const char* buf, size_t size)
{
	printf("got data: %s", buf);
	// todo: this should actually go to the server or session manager to handle this stuff
	setup_pty();
}

void client_connection::setup_pty()
{
	int pty_parent = posix_openpt(O_RDWR | O_NOCTTY);
	EXPECT(pty_parent != -1, "failed to open parent pty");
	EXPECT(grantpt(pty_parent) != -1, "grantpt failed");
	EXPECT(unlockpt(pty_parent) != -1, "unlockpt failed");

	char* child_name = ptsname(pty_parent);
	EXPECT(child_name != nullptr, "ptsname failed");

	pid_t pid = fork();
	if (pid == 0)
	{
		setup_child_pty(pty_parent, child_name);
	}
	else
	{
		setup_parent_pty(pty_parent);
	}
}

void client_connection::setup_child_pty(int pty_parent, char* name)
{
	setsid();
	int pty_child = open(name, O_RDWR);
	EXPECT(pty_child != -1, "failed to open child pty");

	dup2(pty_child, STDIN_FILENO);
	dup2(pty_child, STDOUT_FILENO);
	dup2(pty_child, STDERR_FILENO);

	close(pty_child);
	close(pty_parent);

	execlp("sh", "sh", nullptr);
}

void client_connection::setup_parent_pty(int pty_parent)
{

}

}
