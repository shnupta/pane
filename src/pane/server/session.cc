#include "session.h"
#include "client_connection.h"
#include "session_manager.h"

#include "pane/expect.h"
#include "spdlog/spdlog.h"
#include "src/pane/proto/pane.capnp.h"

#include <capnp/message.h>
#include <capnp/serialize.h>

#include <algorithm>
#include <cstring>
#include <string>

namespace pane {

session::session(runtime* r, session_manager* sm, size_t id)
	: _runtime(r), _session_manager(sm), _id(id), _name(std::to_string(id))
{
	_windows.emplace_back(_runtime, this, _next_window_id);
	_active_window_idx = 0;
	++_next_window_id;
}

void session::attach(client_connection* conn)
{
	_attached_clients.push_back(conn);
}

void session::detach(client_connection* conn)
{
	auto it = std::find_if(_attached_clients.begin(), _attached_clients.end(), [=](auto* other) { return conn == other; });
	_attached_clients.erase(it);
}

void session::broadcast_terminal_output(size_t window_id, size_t pane_id, char* data, int size)
{
	capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<proto::PaneTerminalContent>();
	auto pane = builder.initPane();
	pane.setSessionId(_id);
	pane.setWindowId(window_id);
	pane.setPaneId(pane_id);

	auto content = builder.initContent(size);
	std::memcpy(content.begin(), data, size);

	for (auto* client : _attached_clients)
	{
		client->send_message(proto::Message::PANE_TERMINAL_CONTENT, message);
	}
}

pane* session::try_get_pane(size_t window_id, size_t pane_id)
{
	auto it = std::find_if(_windows.begin(), _windows.end(), [=](const auto& o) { return o.id() == window_id; });

	if (it == _windows.end()) return nullptr;

	return it->try_get_pane(pane_id);
}

void session::cleanup_window(size_t window_id)
{
	auto it = std::find_if(_windows.begin(), _windows.end(), [=](const auto& o) { return o.id() == window_id; });
	EXPECT(it != _windows.end(), "could not find window to cleanup");

	_windows.erase(it);

	if (_windows.empty())
	{
		SPDLOG_INFO("window {} closed", window_id);
		_session_manager->cleanup_session(this);
	}
	else
	{
		// attach client to next window
	}
}

}
