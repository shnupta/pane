#pragma once

#include "window.h"

#include <vector>
#include <string>

namespace pane {

class client_connection;
class session_manager;

class session
{
public:
	explicit session(runtime* r, session_manager* sm, size_t id);

	size_t id() const { return _id; }
	const std::string& name() const { return _name; }
	const std::vector<window>& windows() const { return _windows; };
	const window& active_window() const { return _windows.at(_active_window_idx); }

	void attach(client_connection*);
	void detach(client_connection*);

	pane* try_get_pane(size_t window_id, size_t pane_id);
	void broadcast_terminal_output(size_t window_id, size_t pane_id, char* data, int size);
	void cleanup_window(size_t window_id);

private:
	runtime* _runtime = nullptr;
	session_manager* _session_manager = nullptr;
	size_t _id;
	std::string _name;

	size_t _next_window_id = 0;
	size_t _active_window_idx = 0;
	std::vector<window> _windows;
	std::vector<client_connection*> _attached_clients;
};

}
