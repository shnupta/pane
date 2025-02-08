#include "server.h"
#include "../expect.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <memory>

namespace teja::unix_socket {

server::server(type t)
	: _socket(t)
{
}

server::~server()
{
	if (_runtime)
	{
		_runtime->remove_fd(_socket.fd());
	}
}

const char* server::addr() const
{
	return _socket.addr();
}

int server::fd() const
{
	return _socket.fd();
}

void server::bind(const char* address)
{
	_socket.bind(address);
	_socket.set_non_blocking();
}

void server::listen(runtime* r, handler* handler)
{
	_socket.listen();
	_runtime = r;
	_handler = handler;
	_runtime->register_fd(_socket.fd(), this, fd_events::read); // connections only
}

void server::set_handler(handler* handler)
{
	_handler = handler;
}

void server::on_fd_readable()
{
	if (_handler)
	{
		socket sock = _socket.accept();
		if (sock.fd() > 0)
		{
			_handler->on_new_connection(this,
					std::unique_ptr<connection>(new connection(std::move(sock), _runtime)));
			return;
		}
		EXPECT(errno == EWOULDBLOCK || errno == EAGAIN, "accept failed");
	}
}

void server::on_fd_writable()
{
}

void server::on_fd_error()
{

}

}
