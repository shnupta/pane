#pragma once

#include "../runtime.h"
#include "../fd.h"

#include <string>

namespace pane {

class window;

// a pane represents one pty (psuedo terminal).
// we control the parent pty by writing and reading to it.
// the actual commands and ran by the child pty and passed
// to the parent. this way we just write and read to/from the
// parent whenever, whilst the child pty does the real work.
class pane final : public fd_handler
{
public:
	explicit pane(runtime* r, window* w, size_t id);
	~pane();

	size_t id() const { return _id; }
	const std::string& name() const { return _name; }

	void consume_input(const char* data, size_t size);

private:
	runtime* _runtime = nullptr;
	window* _window = nullptr;
	size_t _id;
	std::string _name;
	int _pty_parent = -1;
	int _pty_child = -1;
	char* _child_name = nullptr;

	// todo: scrollback buffer
	// todo: current screen??
	// todo: on attach send all of these
	//
	// client resizing info:
	// struct winsize ws;
	// ws.ws_row = new_rows;
	// ws.ws_col = new_cols;
	// ioctl(pty_fd, TIOCSWINSZ, &ws);
	// 
	// the pane size is the smallest of any attached clients sizes

	void on_fd_readable(int) override;
	void on_fd_writable(int) override;
	void on_fd_error(int) override;

	void setup_parent_pty();
	void setup_child_pty();

	void broadcast_terminal_output(char* data, int size);

};

}
