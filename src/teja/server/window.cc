#include "window.h"

namespace teja {

window::window(runtime* r)
	: _runtime(r)
{
	_panes.emplace_back(r);
}

}
