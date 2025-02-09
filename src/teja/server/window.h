#pragma once

#include "pane.h"
#include "../runtime.h"

#include <vector>
namespace teja {

class window
{
public:
	explicit window(runtime* r);

private:
	runtime* _runtime = nullptr;
	std::vector<pane> _panes;
};

}
