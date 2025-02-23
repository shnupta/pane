#include "connection.h"
#include "../runtime.h"
#include "../expect.h"
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

namespace pane::unix_socket {

namespace {

struct message_header
{
	int type;
	size_t size;
};

}

connection::connection()
	: _socket(unix_socket::type::stream)
{
}

connection::connection(socket sock, runtime* runtime)
	: _socket(std::move(sock)), _runtime(runtime), _state(state::connected)
{
	EXPECT(_socket.set_non_blocking(), "non blocking failed");
	_runtime->register_fd(_socket.fd(), this, fd_events::read);
}

connection::~connection()
{
	if (_runtime)
	{
		_runtime->remove_fd(_socket.fd());
	}
}

void connection::connect(const char* addr, runtime* runtime)
{
	EXPECT(_state == state::disconnected, "already connected");
	_runtime = runtime;
	_state = state::connecting;
	EXPECT(_socket.set_non_blocking(), "non blocking failed");
	_runtime->register_fd(_socket.fd(), this, fd_events::write);
	int res = _socket.connect(addr);
	if (res < 0)
	{
		_state = state::disconnected;
		_handler->on_disconnected();
	}
	if (res == 0 && _handler)
	{
		_state = state::connected;
		_handler->on_connected();
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

void connection::on_fd_readable(int) 
{
	constexpr size_t k_read_size = 65'536;
	auto* buf = _read_buf.prepare_source(k_read_size);
	int size = _socket.recv(buf, k_read_size);
	if (size == 0)
	{
		if (_handler)
		{
			// expect to be destroyed here
			_handler->on_disconnected();
			_state = state::disconnected;
			return;
		}
	}
	if (size == EWOULDBLOCK || size == EAGAIN)
		return;
	EXPECT(size > 0, "read failed?");

	_read_buf.decrement_source(k_read_size - size);
	_read_buf.flush_source();

	auto span = _read_buf.get_sink();
	size_t offset = 0;
	while (offset < span.size())
	{
		auto* header = reinterpret_cast<message_header*>(span.data() + offset);
		offset += sizeof(message_header);
		// todo: make deletion in callback safe
		if (_handler)
		{
			_handler->on_message(header->type, span.data() + offset, header->size);
		}
		offset += header->size;
		_read_buf.flush_sink(sizeof(message_header) + header->size);
	}
}

void connection::on_fd_writable(int)
{
	switch (_state)
	{
		case state::disconnected:
			EXPECT(false, "got fd writable whilst disconnected");
		case state::connected:
			do_write();
			break;
		case state::connecting:
			int err;
			socklen_t len = sizeof(err);
			if (getsockopt(_socket.fd(), SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0)
			{
				_handler->on_disconnected();
			}
			else
			{
				_handler->on_connected();
			}
			break;
	}
}

void connection::on_fd_error(int) 
{
	// TODO
}

void connection::send_message(int type, const char *data, size_t size)
{
	auto* buf = _write_buf.prepare_source(sizeof(message_header) + size);
	auto* mh = reinterpret_cast<message_header*>(buf);
	mh->type = type;
	mh->size = size;

	auto* payload = buf + sizeof(message_header);
	std::memcpy(payload, data, size);
	_write_buf.flush_source();

	_runtime->update_fd(_socket.fd(), fd_events::read_write);
}

void connection::do_write()
{
	auto span = _write_buf.get_sink();
	if (span.size() == 0)
	{
		_runtime->update_fd(_socket.fd(), fd_events::read);
		return;
	}

	auto sent = _socket.send(span.data(), span.size());
	if (sent < 0)
	{
		EXPECT(false, "send failed");
	}

	_write_buf.flush_sink(sent);

	if (sent == span.size())
	{
		_runtime->update_fd(_socket.fd(), fd_events::read);
	}
}

}
