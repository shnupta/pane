@0xed544dbdcd84b162;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("teja::proto");

enum Message {
	hello @0;
	helloResponse @1;
	sessionList @2;
	attachRequest @3;
}

enum ProtocolMajorVersion {
	invalid @0;
	initial @1;
}

enum ProtocolMinorVersion {
	initial @0;
}

struct ProtocolVersion {
	major @0 :ProtocolMajorVersion;
	minor @1 :ProtocolMinorVersion;
}

const currentVersion :ProtocolVersion = (major = initial, minor = initial);

# client -> server

struct Hello {
	clientVersion @0 :ProtocolVersion;
}

# on receiving an attach request the server will:
# 1. create a new session, window, and pane if none exist.
# 2. select a default window and send the state to the client.
struct AttachRequest {

}


# server -> client

struct HelloResponse {
	serverVersion @0 :ProtocolVersion;
	error @1 :Text;
}

struct SessionList {
	# todo
}
