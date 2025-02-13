#pragma once

#include "session.h"
#include "../runtime.h"

#include <unordered_map>
#include <vector>

namespace teja {

class client_connection;

class session_manager
{
public:
	explicit session_manager(runtime*);

	session* get_or_create_default_session();
	void attach_client_to_session(client_connection*, session*);
	void client_disconnected(client_connection*);
	const std::vector<std::unique_ptr<session>>& get_sessions() const;
	pane* try_get_pane(size_t session_id, size_t window_id, size_t pane_id);

private:
	runtime* _runtime = nullptr;
	size_t _next_session_id = 0;
	session* _default_session = nullptr;

	std::vector<std::unique_ptr<session>> _sessions;
	std::unordered_map<client_connection*, session*> _client_attachments;

	session* create_session();
	void send_attach_response(client_connection*, session*);
};

}
