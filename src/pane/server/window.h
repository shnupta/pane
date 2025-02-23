#pragma once

#include "pane.h"
#include "../runtime.h"

#include <vector>
#include <string>

namespace pane {

class session;

class window
{
public:
	explicit window(runtime* r, session* s, size_t id);

	size_t id() const { return _id; }
	const std::string& name() const { return _name; }
	const std::vector<std::unique_ptr<pane>>& panes() const { return _panes; }

	pane* try_get_pane(size_t pane_id);
	void broadcast_terminal_output(size_t pane_id, char* data, int size);
	void cleanup_pane(pane*);

private:
	runtime* _runtime = nullptr;
	session* _session = nullptr;
	size_t _id;
	std::string _name;

	size_t _next_pane_id = 0;
	std::vector<std::unique_ptr<pane>> _panes;
};

}
