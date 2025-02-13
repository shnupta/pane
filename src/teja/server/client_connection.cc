#include "client_connection.h"
#include "server.h"
#include "pane.h"
#include "session_manager.h"
#include "../expect.h"
#include "logging.h"

#include "spdlog/spdlog.h"
#include "src/teja/proto/teja.capnp.h"
#include <capnp/serialize.h>

#include <cstdlib>
#include <sys/fcntl.h>
#include <unistd.h>

namespace teja {

client_connection::client_connection(server* server, session_manager* sm, std::unique_ptr<unix_socket::connection> conn)
	: _server(server), _session_manager(sm), _connection(std::move(conn))
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
		case proto::Message::CLIENT_TERMINAL_INPUT:
			handle(reader.getRoot<proto::ClientTerminalInput>());
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

	send_message(proto::Message::HELLO_RESPONSE, message);
	SPDLOG_INFO("sent hello response (error={})", builder.hasError());

	if (builder.hasError())
	{
		// todo: disconnect client
		return;
	}

	send_session_list();
}

void client_connection::handle(proto::AttachRequest::Reader reader)
{
	SPDLOG_INFO("received attach request from client {}", _id);
	auto* session = _session_manager->get_or_create_default_session();
	_session_manager->attach_client_to_session(this, session);
}

void client_connection::handle(proto::ClientTerminalInput::Reader reader)
{
	auto readerPane = reader.getPane();
	auto* pane = _session_manager->try_get_pane(readerPane.getSessionId(), readerPane.getWindowId(), readerPane.getPaneId());

	if (!pane) return;

	pane->consume_input(reader.getInput().asChars().begin(), reader.getInput().asChars().size());
}

void client_connection::send_session_list()
{
	capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<proto::SessionList>();
	const auto& sessions = _session_manager->get_sessions();

	auto sessionsList = builder.initSessions(sessions.size());
	for (size_t sIdx = 0; sIdx < sessions.size(); ++sIdx)
	{
		const auto& session = sessions.at(sIdx);
		auto sessionDetails = sessionsList[sIdx];
		sessionDetails.setId(session->id());
		sessionDetails.setName(session->name());

		const auto& windows = session->windows();
		auto windowsList = sessionDetails.initWindows(windows.size());
		for (size_t wIdx = 0; wIdx < windows.size(); ++wIdx)
		{
			const auto& window = windows.at(wIdx);
			auto windowDetails = windowsList[wIdx];
			windowDetails.setId(window.id());
			windowDetails.setName(window.name());

			const auto& panes = window.panes();
			auto panesList = windowDetails.initPanes(panes.size());
			for (size_t pIdx = 0; pIdx < panes.size(); ++pIdx)
			{
				const auto& pane = panes.at(pIdx);
				auto paneDetails = panesList[pIdx];

				paneDetails.setId(pane.id());
				paneDetails.setName(pane.name());
			}
		}
	}

	send_message(proto::Message::SESSION_LIST, message);
	SPDLOG_INFO("sent session list to client {}", _id);
}

void client_connection::send_message(proto::Message type, capnp::MessageBuilder& builder)
{
	auto data = capnp::messageToFlatArray(builder);
	_connection->send_message(static_cast<int>(type), data.asChars().begin(), data.asChars().size());
}

}
