#include "client_connection.h"
#include "server.h"
#include "../expect.h"
#include "logging.h"

#include "spdlog/spdlog.h"
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
	_connection->set_handler(this);
	SPDLOG_INFO("new client ({})", _id);
}

void client_connection::on_connected()
{
	EXPECT(false, "should never get on_connected in client connection as it's constructed in connected state");
}

void client_connection::on_disconnected()
{
	SPDLOG_INFO("client {} disconnected", _id);
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
		case proto::Message::ATTACH_REQUEST:
			handle(reader.getRoot<proto::AttachRequest>());
			break;

		case proto::Message::HELLO_RESPONSE:
			// unexpected
		default:
			// unknown
			SPDLOG_ERROR("received unknown message type {}", type);
	}
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
	SPDLOG_INFO("sent hello response (error={})", builder.hasError());

	if (builder.hasError())
	{
		// todo: disconnect client
	}
}

void client_connection::handle(proto::AttachRequest::Reader reader)
{
	SPDLOG_INFO("received attach request from client {}", _id);
	_server->attach_to_default_session(this);
}

}
