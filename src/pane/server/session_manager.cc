#include "session_manager.h"
#include "client_connection.h"
#include "../expect.h"

#include "src/pane/proto/pane.capnp.h"

#include <capnp/serialize.h>
#include <capnp/message.h>

#include <spdlog/spdlog.h>

namespace pane {

session_manager::session_manager(runtime* r)
	: _runtime(r)
{

}

session* session_manager::get_or_create_default_session()
{
	if (_default_session) return _default_session;

	_default_session = create_session();
	return _default_session;
}

session* session_manager::create_session()
{
	SPDLOG_INFO("creating new session {}", _next_session_id);
	auto& s = _sessions.emplace_back(std::make_unique<session>(_runtime, this, _next_session_id));
	++_next_session_id;
	return s.get();
}

void session_manager::cleanup_session(session* s)
{
	auto it = std::find_if(_sessions.begin(), _sessions.end(), [=](const auto& other) { return other.get() == s; });
	EXPECT(it != _sessions.end(), "could not find session to cleanup");

	_sessions.erase(it);

	if (_sessions.empty())
	{
		SPDLOG_INFO("last session closed, stopping server");
		_runtime->stop();
	}
	else
	{
		// todo: move client to next session
	}
}

void session_manager::attach_client_to_session(client_connection* conn, session* s)
{
	SPDLOG_INFO("attaching client {} to session {}", conn->id(), s->id());
	auto it = std::find_if(_sessions.begin(), _sessions.end(), [=](const auto& other) { return other.get() == s; });
	EXPECT(it != _sessions.end(), "session doesn't exist, can't attach");
	s->attach(conn);
	_client_attachments[conn] = s;
	send_attach_response(conn, s);
	// TODO: also send the terminal back buffer??
}

void session_manager::send_attach_response(client_connection* conn, session* s)
{
	capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<proto::AttachResponse>();

	auto pane = builder.initPane();
	pane.setSessionId(s->id());
	auto& window = s->active_window();
	pane.setWindowId(window.id());
	pane.setPaneId(0); // TODO: better :)

	conn->send_message(proto::Message::ATTACH_RESPONSE, message);
}

void session_manager::client_disconnected(client_connection* conn)
{
	auto* s = _client_attachments[conn];
	if (s) s->detach(conn);
	_client_attachments.erase(conn);
}

const std::vector<std::unique_ptr<session>>& session_manager::get_sessions() const
{
	return _sessions;
}

pane* session_manager::try_get_pane(size_t session_id, size_t window_id, size_t pane_id)
{
	auto it = std::find_if(_sessions.begin(), _sessions.end(), [=](const auto& o) { return o->id() == session_id; });

	if (it == _sessions.end()) return nullptr;

	return (*it)->try_get_pane(window_id, pane_id);
}

}
