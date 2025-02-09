#pragma once

#include "window.h"

#include <vector>

namespace teja {

class session
{
public:
	explicit session(runtime* r, size_t id);

	size_t id() const { return _id; }

private:
	runtime* _runtime = nullptr;
	size_t _id;
	std::vector<window> _windows;

};

}
