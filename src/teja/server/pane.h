#pragma once

#include "../runtime.h"
#include "../fd.h"

namespace teja {

// a pane represents one pty (psuedo terminal).
// we control the parent pty by writing and reading to it.
// the actual commands and ran by the child pty and passed
// to the parent. this way we just write and read to/from the
// parent whenever, whilst the child pty does the real work.
class pane final : public fd_handler
{
public:
	explicit pane(runtime* r);
	~pane();

private:
	runtime* _runtime = nullptr;
	int _pty_parent = -1;
	int _pty_child = -1;
	char* _child_name = nullptr;

	// todo: scrollback buffer
	// todo: current screen??
	// todo: on attach send all of these

	void on_fd_readable() override;
	void on_fd_writable() override;
	void on_fd_error() override;

	void setup_parent_pty();
	void setup_child_pty();

};

}
