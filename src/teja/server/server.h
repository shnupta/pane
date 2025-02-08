#include "../runtime.h"
#include "client_connection.h"
#include "../unix_socket/server.h"

#include <quill/Logger.h>

namespace teja {

// runs in background. maintains all state.
class server final : public unix_socket::server::handler
{
public:
	static bool is_running();
	static bool kill();

	// spawns server in a new thread and detaches
	// server process returns true
	static bool spawn();

	// run the server in the foreground
	static void foreground();

	~server();

	void client_disconnected(client_connection*);
	
private:
	quill::Logger* _logger = nullptr;
	runtime _runtime;

	std::unique_ptr<unix_socket::server> _server_socket;

	std::vector<std::unique_ptr<client_connection>> _client_conns;

	server();

	void create_pid_file();
	void remove_pid_file();
	void create_socket();
	void spin();

	void on_new_connection(unix_socket::server*, std::unique_ptr<unix_socket::connection>) override;

};

}
