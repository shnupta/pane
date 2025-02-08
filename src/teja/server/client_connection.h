#pragma once

#include "../unix_socket/connection.h"
#include <memory>

namespace teja {

class server;

class client_connection : public unix_socket::connection::handler 
{
public:
	explicit client_connection(server* server, std::unique_ptr<unix_socket::connection>);

private:
	server* _server = nullptr;
	std::unique_ptr<unix_socket::connection> _connection;

	void on_disconnected() override;
	void on_data(const char*, size_t) override;
	void setup_pty();

	void setup_child_pty(int pty_parent, char* name);
	void setup_parent_pty(int pty_parent);

};

}
