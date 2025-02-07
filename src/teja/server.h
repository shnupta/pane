#include "runtime.h"
#include "unix_socket.h"

#include <quill/Logger.h>

namespace teja {

// runs in background. maintains all state.
class server
{
public:
	static bool is_running();

	// probably make constructor private and only allow creation via spawn
	server();
	~server();

	// todo: method to spawn a new server and return the pid?
	// then we can launch a client to connect to the server socket

private:
	quill::Logger* _logger = nullptr;
	runtime _runtime;

	std::unique_ptr<unix_socket> _socket;

	void remove_pid_file();
	void create_socket();
	void destroy_socket();
	void spin();
};

}
