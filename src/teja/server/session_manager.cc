#include "session_manager.h"
#include "client_connection.h"

#include <spdlog/spdlog.h>

namespace teja {

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
	auto [it, _] = _sessions.try_emplace(_next_session_id, std::make_unique<session>(_runtime, _next_session_id));
	++_next_session_id;
	return it->second.get();
}

void session_manager::attach_client_to_session(client_connection* conn, session* session)
{
	SPDLOG_INFO("attaching client {} to session {}", conn->id(), session->id());
	auto& details = _session_details[session];
	details.connected_clients.emplace_back(conn);
}

}
