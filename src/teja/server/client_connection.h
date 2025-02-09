#pragma once

#include "../unix_socket/connection.h"

#include "src/teja/proto/teja.capnp.h"

#include <memory>

namespace teja {

class server;

class client_connection : public unix_socket::connection::handler 
{
public:
	// todo: pass session_manager here so client can interact
	explicit client_connection(server* server, std::unique_ptr<unix_socket::connection>);

	size_t id() const { return _id; };

private:
	server* _server = nullptr;
	std::unique_ptr<unix_socket::connection> _connection;
	bool _valid = false; // until we know the client is compatible
	size_t _id = 0;

	void on_connected() override;
	void on_disconnected() override;
	void on_message(int type, const char* data, size_t size) override;

	// void setup_pty();
	// void setup_child_pty(int pty_parent, char* name);
	// void setup_parent_pty(int pty_parent);

	void handle(proto::Hello::Reader);
	void handle(proto::AttachRequest::Reader);

};

}
