#include "server.h"
#include "logging.h"
#include "../expect.h"
#include "../paths.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <csignal>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sys/fcntl.h>
#include <unistd.h>

namespace teja {

namespace {

pid_t get_pid()
{
	pid_t pid;
	std::ifstream is(PID_FILE);
	is >> pid;
	return pid;
}

}

bool server::is_running()
{
	bool pid_exists = std::filesystem::exists(PID_FILE);
	if (!pid_exists) return false;

	pid_t pid = get_pid();
	return ::kill(pid, 0) == 0;
}

bool server::kill()
{
	pid_t pid = get_pid();
	return ::kill(pid, SIGTERM) == 0;
}

bool server::spawn()
{
	pid_t pid = fork();
	EXPECT(pid >= 0, "fork failed whilst trying to spawn server");
	if (pid != 0) return false;

	setsid();
	pid = fork();
	EXPECT(pid >= 0, "second fork failed whilst trying to spawn server");

	if (pid != 0) exit(EXIT_SUCCESS);

	int devnull = open("/dev/null", O_RDWR);
	if (devnull >= 0) {
		dup2(devnull, STDIN_FILENO);
		dup2(devnull, STDOUT_FILENO);
		dup2(devnull, STDERR_FILENO);
		close(devnull);
	}

	configure_file_logging();

	// we are the new server process
	server s;
	try
	{
		s.spin();
	}
	catch (std::exception& e)
	{
		SPDLOG_ERROR("server died: {}", e.what());
		std::cerr << "server died: " << e.what() << std::endl;
	}
	return true;
}

void server::foreground()
{
	server s;
	try
	{
		s.spin();
	}
	catch (std::exception& e)
	{
		SPDLOG_ERROR("server died: {}", e.what());
		std::cerr << "server died: " << e.what() << std::endl;
	}
}

server::server()
{
	SPDLOG_INFO("server spawning");

	remove_pid_file();
	create_pid_file();
	create_socket();
}

server::~server()
{
	SPDLOG_INFO("server shutting down");
	remove_pid_file();
}

void server::client_disconnected(client_connection* conn)
{
	auto it = std::find_if(_client_conns.begin(), _client_conns.end(), [conn](const auto& c) {
			return c.get() == conn;
			});

	_client_conns.erase(it);
}

void server::attach_to_default_session(client_connection* conn)
{
	auto* session = _session_manager.get_or_create_default_session();
	_session_manager.attach_client_to_session(conn, session);
}

void server::create_pid_file()
{
	EXPECT(!std::filesystem::exists(PID_FILE), "teja server pid file already exists");
	std::ofstream os(PID_FILE);
	os << getpid() << '\n';
}

void server::remove_pid_file()
{
	if (std::filesystem::exists(PID_FILE))
	{
		std::filesystem::remove(PID_FILE);
		SPDLOG_INFO("pid file at {} removed", PID_FILE.c_str());
	}

}

void server::create_socket()
{
	SPDLOG_INFO("creating socket");
	_server_socket = std::make_unique<unix_socket::server>(unix_socket::type::stream);
	SPDLOG_INFO("binding to {}", SOCKET_FILE.c_str());
	_server_socket->bind(SOCKET_FILE.c_str());
	SPDLOG_INFO("unix socket bound fd={}", _server_socket->fd());
	_server_socket->listen(&_runtime, this);
	SPDLOG_INFO("socket listening");
}

void server::spin()
{
	_runtime.run();
}

void server::on_new_connection(unix_socket::server*, std::unique_ptr<unix_socket::connection> conn)
{
	SPDLOG_INFO("new client connected fd={}", conn->fd());
	_client_conns.push_back(std::make_unique<client_connection>(this, std::move(conn)));
}

}
