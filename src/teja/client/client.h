#pragma once

#include "tui.h"
#include "../runtime.h"
#include "../fd.h"
#include "../unix_socket/connection.h"

#include "src/teja/proto/teja.capnp.h"

#include <capnp/message.h>
#include <deque>
#include <optional>

namespace teja {

class client final : public unix_socket::connection::handler, public fd_handler
{
public:
	enum class action
	{
		list_sessions
	};

	static void attach();
	static void perform_action(action);

	~client();

private:
	runtime _runtime;
	unix_socket::connection _connection{};
	tui _tui;
	std::optional<raw_fd> _stdin_fd;

	int _active_session = -1;
	int _active_window = -1;
	int _active_pane = -1;

	std::deque<action> _actions;

	// future: instead of just run maybe do separate method calls so that the 
	// cli is flexible. so attach still exists and just goes default session.
	// but other args might list-sessions or attach to a particular session.
	void run();
	void push_action(action a);

	std::optional<action> next_action() const;
	action pop_action();

	void send_message(proto::Message, capnp::MessageBuilder&);

	void on_connected() override;
	void on_disconnected() override;
	void on_message(int type, const char* data, size_t size) override;

	void on_fd_writable(int) override;
	void on_fd_readable(int) override;
	void on_fd_error(int) override;

	void handle(proto::HelloResponse::Reader);
	void handle(proto::SessionList::Reader);
	void handle(proto::AttachResponse::Reader);
	void handle(proto::PaneTerminalContent::Reader);

	void send_window_size();
	void send_attach_request();
};

}
