#include "tui.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <termios.h>
#include <unistd.h>

namespace teja {

bool tui::in_raw_mode() const
{
	return _in_raw_mode;
}

void tui::enable_raw_mode()
{
	if (tcgetattr(STDIN_FILENO, &_original_termios))
	{
		perror("tcgetattr failed trying to enable raw mode");
		exit(EXIT_FAILURE);
	}

	struct termios raw = _original_termios;
	// raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
	raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
	{
		perror("tcsetattr failed trying to enable raw mode");
		exit(EXIT_FAILURE);
	}
	// Clear screen and move cursor to top-left
	std::cout << "\033[2J\033[H" << std::flush;
	_in_raw_mode = true;
}

void tui::disable_raw_mode()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &_original_termios);
	_in_raw_mode = false;
}

}
