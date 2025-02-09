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

private:
	runtime* _runtime = nullptr;
	size_t _next_session_id = 0;
	session* _default_session = nullptr;

	struct session_details
	{
		std::vector<client_connection*> connected_clients;
	};

	std::unordered_map<size_t, std::unique_ptr<session>> _sessions;
	std::unordered_map<session*, session_details> _session_details;


	session* create_session();

};

}
