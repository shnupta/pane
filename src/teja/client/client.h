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

	// future: instead of just run maybe do separate method calls so that the 
	// cli is flexible. so attach still exists and just goes default session.
	// but other args might list-sessions or attach to a particular session.
	void run();

	void send_message(proto::Message, capnp::MessageBuilder&);

	void on_connected() override;
	void on_disconnected() override;
	void on_message(int type, const char* data, size_t size) override;

	void handle(proto::HelloResponse::Reader);

	void send_attach_request();
};

}
