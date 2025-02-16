@0xed544dbdcd84b162;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("pane::proto");

enum Message {
	hello @0;
	helloResponse @1;
	sessionList @2;
	clientWindowSize @3;
	attachRequest @4;
	attachResponse @5;
	paneTerminalContent @6;
	clientTerminalInput @7;
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

# common structures

struct PaneDetails {
	id @0 :UInt32;
	name @1 :Text;
}

struct WindowDetails {
	id @0 :UInt32;
	name @1 :Text;
	panes @2 :List(PaneDetails);
}

struct SessionDetails {
	id @0 :UInt32; 
	name @1 :Text;
	windows @2 :List(WindowDetails);
}

struct PaneIdentifier {
	sessionId @0 :UInt32;
	windowId @1 :UInt32;
	paneId @2 :UInt32;
}

# client -> server

# clients must send a hello immediately after connecting.
# the server then responds with a HelloResponse, and if valid sends the SessionList.
# following this clients can attach to a session (or default) with AttachRequest
struct Hello {
	clientVersion @0 :ProtocolVersion;
}

# todo: window resizing should be sent straight after hello (and on any subsequent resize)
# perhaps I make this a more general client details message
struct ClientWindowSize {

}

# on receiving an attach request the server will:
# 1. create a new session, window, and pane if none exist.
# 2. select a default window and send the state to the client.
struct AttachRequest {
	session :union {
		id @0 :UInt32;
		default @1 :Void;
	}
}

struct ClientTerminalInput {
	pane @0 :PaneIdentifier;
	input @1 :Data;
}

# server -> client

struct HelloResponse {
	serverVersion @0 :ProtocolVersion;
	error @1 :Text;
}

struct SessionList {
	sessions @0 :List(SessionDetails);
}

struct AttachResponse {
	pane @0 :PaneIdentifier;
	error @1 :Text;
}

struct PaneTerminalContent {
	pane @0 :PaneIdentifier;
	content @1 :Data;
}
