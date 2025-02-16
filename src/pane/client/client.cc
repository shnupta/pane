#include "client.h"
#include "../paths.h"

#include "src/pane/proto/pane.capnp.h"
#include "pane/expect.h"

#include <capnp/serialize.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

namespace pane {

void client::attach()
{
	client client;
	client.run();
}

void client::perform_action(action a)
{
	client client;
	client.push_action(a);
	client.run();
}

void client::run()
{
	_connection.set_handler(this);
	_connection.connect(SOCKET_FILE.c_str(), &_runtime);
	_runtime.run();
}

client::~client()
{
	if (_tui.in_raw_mode()) _tui.disable_raw_mode();
}

void client::push_action(action a)
{
	_actions.push_back(a);
}

auto client::next_action() const -> std::optional<action>
{
	if (_actions.size() > 0)
		return _actions.front();

	return std::nullopt;
}

auto client::pop_action() -> action
{
	auto a = _actions.front();
	_actions.pop_front();
	return a;
}

void client::send_message(proto::Message type, capnp::MessageBuilder& builder)
{
	auto flat_array = capnp::messageToFlatArray(builder);
	_connection.send_message(static_cast<int>(type), flat_array.asChars().begin(), flat_array.asChars().size());
}

void client::on_connected()
{
	capnp::MallocMessageBuilder message;
	proto::Hello::Builder hello = message.initRoot<proto::Hello>();
	hello.setClientVersion(proto::CURRENT_VERSION);
	send_message(proto::Message::HELLO, message);
}

void client::on_disconnected()
{

}

void client::on_message(int type, const char* data, size_t size)
{
	kj::ArrayPtr<const capnp::word> flat_array(reinterpret_cast<const capnp::word*>(data), size);
	capnp::FlatArrayMessageReader reader(flat_array);
	auto mtype = static_cast<proto::Message>(type);

	switch (mtype)
	{
		case proto::Message::HELLO_RESPONSE:
			handle(reader.getRoot<proto::HelloResponse>());
			break;
		case proto::Message::SESSION_LIST:
			handle(reader.getRoot<proto::SessionList>());
			break;
		case proto::Message::ATTACH_RESPONSE:
			handle(reader.getRoot<proto::AttachResponse>());
			break;
		case proto::Message::PANE_TERMINAL_CONTENT:
			handle(reader.getRoot<proto::PaneTerminalContent>());
			break;

		case proto::Message::HELLO:
			// unexpected
		default:
			// unknown
			break;
	}
}

void client::handle(proto::HelloResponse::Reader reader)
{
	if (reader.hasError())
	{
		printf("cannot connect to server: %s", reader.getError().cStr());
		// todo: destroy connection
		return;
	}
}

void client::handle(proto::SessionList::Reader reader)
{
	if (next_action() && pop_action() == action::list_sessions)
	{
		std::cerr << reader.toString().flatten().cStr() << std::endl;
		_runtime.stop();
		return;
	}

	send_window_size();
	send_attach_request();
}

void client::send_window_size()
{
}

void client::send_attach_request()
{
	capnp::MallocMessageBuilder message;
	proto::AttachRequest::Builder attach_request = message.initRoot<proto::AttachRequest>();
	send_message(proto::Message::ATTACH_REQUEST, message);
	// TODO: connect to session id
}

void client::handle(proto::AttachResponse::Reader reader)
{
	if (reader.hasError())
	{
		// TODO
		return;
	}

	_tui.enable_raw_mode();
	_stdin_fd.emplace(STDIN_FILENO, &_runtime, this, fd_events::read);

	_active_session = reader.getPane().getSessionId();
	_active_window = reader.getPane().getWindowId();
	_active_pane = reader.getPane().getPaneId();
}

void client::handle(proto::PaneTerminalContent::Reader reader)
{
	// TODO: check and write to the correct panes buffer
	auto content = reader.getContent();
	::write(STDOUT_FILENO, content.asChars().begin(), content.asChars().size());
}

void client::on_fd_writable(int)
{

}

void client::on_fd_readable(int)
{
	char buf[1 << 16];
	// stuff was written to the terminal
	int res = ::read(STDIN_FILENO, buf, sizeof(buf));
	if (res > 0)
	{
		// TODO: check the input for special characters (prefix + something)
		capnp::MallocMessageBuilder message;
		auto builder = message.initRoot<proto::ClientTerminalInput>();

		auto pane = builder.initPane();
		pane.setSessionId(_active_session);
		pane.setWindowId(_active_window);
		pane.setPaneId(_active_pane);

		auto input = builder.initInput(res);
		std::memcpy(input.begin(), buf, res);

		send_message(proto::Message::CLIENT_TERMINAL_INPUT, message);
	}
}

void client::on_fd_error(int)
{

}

}
