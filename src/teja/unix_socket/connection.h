#pragma once

#include "socket.h"

#include "../runtime.h"
#include "../fd.h"

namespace teja::unix_socket {

class connection final : public fd_handler
{
public:
	class handler
	{
	public:
		virtual ~handler() = default;

		virtual void on_disconnected() = 0;
		virtual void on_data(const char*, size_t) = 0;
	};

	// already bound and connected
	explicit connection(socket sock, runtime* runtime);
	~connection();
	const char* addr() const;
	int fd() const;

	void set_handler(handler* handler);

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

	void on_fd_readable() override;
	void on_fd_writable() override;
	void on_fd_error() override;
};

}
