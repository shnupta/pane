@0xed544dbdcd84b162;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("teja::proto");

enum Message {
	hello @0;
	helloResponse @1;
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


# server -> client

struct HelloResponse {
	serverVersion @0 :ProtocolVersion;
	error @1 :Text;
}
