#pragma once

#include "socket.h"

#include "../runtime.h"
#include "../fd.h"
#include "../streambuf.h"

namespace teja::unix_socket {

class connection final : public fd_handler
{
public:
	class handler
	{
	public:
		virtual ~handler() = default;

		virtual void on_connected() = 0;
		virtual void on_disconnected() = 0;
		virtual void on_message(int type, const char* data, size_t size) = 0;
	};

	explicit connection();
	// already bound and connected
	explicit connection(socket sock, runtime* runtime);
	~connection();
	void connect(const char* addr, runtime* runtime);
	const char* addr() const;
	int fd() const;
	
	void set_handler(handler* handler);

	void send_message(int type, const char* data, size_t size);

private:
	enum class state
	{
		disconnected,
		connecting,
		connected
	};

	socket _socket;
	runtime* _runtime = nullptr;
	state _state = state::disconnected;
	handler* _handler = nullptr;
	streambuf _write_buf;
	streambuf _read_buf;

	void on_fd_readable(int) override;
	void on_fd_writable(int) override;
	void on_fd_error(int) override;

	void do_write();
};

}
