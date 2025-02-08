#pragma once

#include "../runtime.h"
#include "../unix_socket/connection.h"

#include "src/teja/proto/teja.capnp.h"

#include <capnp/message.h>

namespace teja {

class client final : public unix_socket::connection::handler
{
public:
	static void attach();

private:
	runtime _runtime;
	unix_socket::connection _connection{};

	void run();

	void on_connected() override;
	void on_disconnected() override;
	void on_message(int type, const char* data, size_t size) override;

	void handle(proto::HelloResponse::Reader);
};

}
