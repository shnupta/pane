#include "session.h"

namespace teja {

session::session(runtime* r, size_t id)
	: _runtime(r), _id(id)
{
	_windows.emplace_back(_runtime);
}

}
