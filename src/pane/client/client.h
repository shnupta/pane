#pragma once

#include "tui.h"
#include "../runtime.h"
#include "../fd.h"
#include "../unix_socket/connection.h"

#include "src/pane/proto/pane.capnp.h"

#include <capnp/message.h>
#include <deque>
#include <optional>

namespace pane {

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

	enum class state
	{
		// default/initial state
		disconnected,
		// attempting to connect to the server
		connecting,
		// connected to the server and doing handshake
		connected
	};

	enum class mode
	{
		// default/initial state 
		none,
		// attached to some session, direct input to server
		raw,
		// prefix sequence pressed, awaiting key specifier
		prefix,
		// typing some command in
		command
	};

	state _state = state::disconnected;
	mode _mode = mode::none;
	size_t _failed_connect_attempts = 0;

	// future: instead of just run maybe do separate method calls so that the 
	// cli is flexible. so attach still exists and just goes default session.
	// but other args might list-sessions or attach to a particular session.
	void run();
	void push_action(action a);

	void connect();

	std::optional<action> next_action() const;
	action pop_action();

	void send_message(proto::Message, capnp::MessageBuilder&);

	void on_connected() override;
	void on_disconnected() override;
	void on_message(int type, const char* data, size_t size) override;

	void on_fd_writable(int) override;
	void on_fd_readable(int) override;
	void on_fd_error(int) override;

	// server messages
	void handle(proto::HelloResponse::Reader);
	void handle(proto::SessionList::Reader);
	void handle(proto::AttachResponse::Reader);
	void handle(proto::PaneTerminalContent::Reader);

	// terminal input
	void handle_raw_input(const char* buf, int len);
	void handle_prefix_input(const char* buf, int len);
	void handle_command_input(const char* buf, int len);

	void send_window_size();
	void send_attach_request();

	void detach();
};

}
