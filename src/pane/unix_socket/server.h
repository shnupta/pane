#pragma once

#include "connection.h"
#include "../runtime.h"
#include "../fd.h"

#include <sys/socket.h>

namespace pane::unix_socket {

class server final : public fd_handler
{
public:	
	class handler
	{
	public:
		virtual ~handler() = default;

		virtual void on_new_connection(server*, std::unique_ptr<connection>) = 0;
	};

	explicit server(type);
	~server();
	const char* addr() const;
	int fd() const;

	void bind(const char* address);
	void listen(runtime*, handler*);
	void set_handler(handler*);

private:
	socket _socket;
	runtime* _runtime = nullptr;
	handler* _handler = nullptr;

	void on_fd_readable(int) override;
	void on_fd_writable(int) override;
	void on_fd_error(int) override;
};

}
