#include "client.h"
#include "../paths.h"

#include "src/teja/proto/teja.capnp.h"

#include <capnp/serialize.h>

namespace teja {

void client::attach()
{
	client client;
	client.run();
}

void client::run()
{
	_connection.set_handler(this);
	_connection.connect(SOCKET_FILE.c_str(), &_runtime);
	_runtime.run();
}

void client::send_message(proto::Message type, capnp::MessageBuilder& builder)
{
	auto flat_array = capnp::messageToFlatArray(builder);
	_connection.send_message(static_cast<int>(type), flat_array.asChars().begin(), flat_array.asChars().size());
}

void client::on_connected()
{
	capnp::MallocMessageBuilder message;
	proto::Hello::Builder hello = message.initRoot<proto::Hello>();
	hello.setClientVersion(proto::CURRENT_VERSION);
	send_message(proto::Message::HELLO, message);
}

void client::on_disconnected()
{

}

void client::on_message(int type, const char* data, size_t size)
{
	kj::ArrayPtr<const capnp::word> flat_array(reinterpret_cast<const capnp::word*>(data), size);
	capnp::FlatArrayMessageReader reader(flat_array);
	auto mtype = static_cast<proto::Message>(type);

	switch (mtype)
	{
		case proto::Message::HELLO_RESPONSE:
			handle(reader.getRoot<proto::HelloResponse>());
			break;

		case proto::Message::HELLO:
			// unexpected
		default:
			// unknown
			break;
	}
}

void client::handle(proto::HelloResponse::Reader reader)
{
	if (reader.hasError())
	{
		printf("cannot connect to server: %s", reader.getError().cStr());
		// todo: destroy connection
		return;
	}

	send_attach_request();
}

void client::send_attach_request()
{
	capnp::MallocMessageBuilder message;
	proto::AttachRequest::Builder attach_request = message.initRoot<proto::AttachRequest>();
	send_message(proto::Message::ATTACH_REQUEST, message);
}

}
