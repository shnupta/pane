#pragma once

#include <termios.h>

namespace pane {

class tui
{
public:

	bool in_raw_mode() const;
	void enable_raw_mode();
	void disable_raw_mode();

private:
	struct termios _original_termios;
	bool _in_raw_mode = false;

};

}
