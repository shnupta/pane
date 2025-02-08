#include "connection.h"
#include "../runtime.h"
#include "../expect.h"

namespace teja::unix_socket {

connection::connection(socket sock, runtime* runtime)
	: _socket(std::move(sock)), _runtime(runtime), _state(state::connected)
{
	_socket.set_non_blocking();
	_runtime->register_fd(_socket.fd(), this, fd_events::read);
}

connection::~connection()
{
	if (_runtime)
	{
		_runtime->remove_fd(_socket.fd());
	}
}

const char* connection::addr() const
{
	return _socket.addr();
}

int connection::fd() const
{
	return _socket.fd();
}

void connection::set_handler(handler* handler)
{
	_handler = handler;
}

void connection::on_fd_readable() 
{
	char buf[1024];
	int res = _socket.recv(buf, sizeof(buf));
	if (res == 0)
	{
		if (_handler)
		{
			// expect to be destroyed here
			_handler->on_disconnected();
			return;
		}
	}
	if (res == EWOULDBLOCK || res == EAGAIN)
		return;

	EXPECT(res > 0, "read failed?");
	// todo: make getting destroyed in on_data safe
	if (_handler)
		_handler->on_data(buf, res);
}

void connection::on_fd_writable()
{

}

void connection::on_fd_error() 
{

}

}
