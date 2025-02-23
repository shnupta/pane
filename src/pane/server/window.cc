#include "window.h"
#include "pane/expect.h"
#include "session.h"

#include <algorithm>
#include <string>

namespace pane {

window::window(runtime* r, session* s, size_t id)
	: _runtime(r), _session(s), _id(id), _name(std::to_string(id))
{
	_panes.emplace_back(std::make_unique<pane>(r, this, _next_pane_id));
	++_next_pane_id;
}

void window::broadcast_terminal_output(size_t pane_id, char* data, int size)
{
	_session->broadcast_terminal_output(_id, pane_id, data, size);
}

pane* window::try_get_pane(size_t pane_id)
{
	auto it = std::find_if(_panes.begin(), _panes.end(), [=](const auto& o) { return o->id() == pane_id; });

	if (it == _panes.end()) return nullptr;

	return it->get();
}

void window::cleanup_pane(pane* p)
{
	auto it = std::find_if(_panes.begin(), _panes.end(), [=](const auto& o) { return o.get() == p; });
	EXPECT(it != _panes.end(), "couldn't find pane to cleanup");

	_panes.erase(it);

	if (_panes.empty())
	{
		_session->cleanup_window(_id);
	}
	else
	{
		// todo: attach the client to the next pane
	}
}

}
