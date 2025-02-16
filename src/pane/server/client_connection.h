#pragma once

#include "../unix_socket/connection.h"

#include "src/pane/proto/pane.capnp.h"

#include <memory>

namespace pane {

class server;
class session_manager;
class pane;

class client_connection : public unix_socket::connection::handler 
{
public:
	// todo: pass session_manager here so client can interact
	explicit client_connection(server* server, session_manager*, std::unique_ptr<unix_socket::connection>);

	size_t id() const { return _id; };

	void send_message(proto::Message, capnp::MessageBuilder&);

private:
	server* _server = nullptr;
	session_manager* _session_manager = nullptr;
	std::unique_ptr<unix_socket::connection> _connection;
	bool _valid = false; // until we know the client is compatible
	size_t _id = 0;

	void on_connected() override;
	void on_disconnected() override;
	void on_message(int type, const char* data, size_t size) override;

	void handle(proto::Hello::Reader);
	void handle(proto::AttachRequest::Reader);
	void handle(proto::ClientTerminalInput::Reader);

	void send_session_list();

};

}
