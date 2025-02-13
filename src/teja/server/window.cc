#include "window.h"
#include "session.h"

#include <algorithm>
#include <string>

namespace teja {

window::window(runtime* r, session* s, size_t id)
	: _runtime(r), _session(s), _id(id), _name(std::to_string(id))
{
	_panes.emplace_back(r, this, _next_pane_id);
	++_next_pane_id;
}

void window::broadcast_terminal_output(size_t pane_id, char* data, int size)
{
	_session->broadcast_terminal_output(_id, pane_id, data, size);
}

pane* window::try_get_pane(size_t pane_id)
{
	auto it = std::find_if(_panes.begin(), _panes.end(), [=](const auto& o) { return o.id() == pane_id; });

	if (it == _panes.end()) return nullptr;

	return &(*it);
}

}
