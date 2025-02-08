#include "client_connection.h"
#include "server.h"
#include "../expect.h"
#include "logging.h"

#include "src/teja/proto/teja.capnp.h"
#include <capnp/serialize.h>

#include <cstdlib>
#include <sys/fcntl.h>
#include <unistd.h>

namespace teja {

client_connection::client_connection(server* server, std::unique_ptr<unix_socket::connection> conn)
	: _server(server), _connection(std::move(conn))
{
	static int s_count = 0;
	_id = ++s_count;
	_logger = create_or_get_logger("client_connection");
	_connection->set_handler(this);
	LOG_INFO(_logger, "new client ({})", _id);
}

void client_connection::on_connected()
{
	EXPECT(false, "should never get on_connected in client connection as it's constructed in connected state");
}

void client_connection::on_disconnected()
{
	LOG_INFO(_logger, "client {} disconnected", _id);
	_connection.reset();
	_server->client_disconnected(this);
}

void client_connection::on_message(int type, const char* data, size_t size)
{
	kj::ArrayPtr<const capnp::word> flat_array(reinterpret_cast<const capnp::word*>(data), size);
	capnp::FlatArrayMessageReader reader(flat_array);
	auto mtype = static_cast<proto::Message>(type);

	switch (mtype)
	{
		case proto::Message::HELLO:
			handle(reader.getRoot<proto::Hello>());
			break;
		case proto::Message::HELLO_RESPONSE:
			// unexpected
		default:
			// unknown
			LOG_ERROR(_logger, "received unknown message type {}", type);
	}
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

void client_connection::handle(proto::Hello::Reader reader)
{
	capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<proto::HelloResponse>();
	builder.setServerVersion(proto::CURRENT_VERSION);

	auto server_major = proto::CURRENT_VERSION->getMajor();
	auto server_minor = proto::CURRENT_VERSION->getMinor();
	auto client_version = reader.getClientVersion();
	if (client_version.getMajor() < server_major)
	{
		builder.setError("client major version too old");
	}
	if (client_version.getMajor() > server_major)
	{
		builder.setError("client major version too new");
	}

	if (client_version.getMajor() == server_major && client_version.getMinor() > server_minor)
	{
		builder.setError("client minor version too new");
	}

	auto flat_array = capnp::messageToFlatArray(message);

	_connection->send_message(static_cast<int>(proto::Message::HELLO_RESPONSE), flat_array.asChars().begin(), flat_array.asChars().size());
	LOG_INFO(_logger, "sent hello response");
}

}
