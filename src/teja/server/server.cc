#include "server.h"
#include "../expect.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/sinks/FileSink.h>
#include <quill/LogMacros.h>

#include <filesystem>
#include <stdexcept>
#include <unistd.h>

namespace teja {

namespace {

// extra portable
std::filesystem::path get_home_dir()
{
	auto* homedir = getenv("HOME");
	if (homedir == nullptr)
	{
		throw std::runtime_error("can't find user home directory. $HOME is not set");
	}

	return std::filesystem::path(homedir);
}

const std::filesystem::path TEJA_DIR = get_home_dir() / ".teja";
const std::filesystem::path PID_FILE = TEJA_DIR / "pid";
const std::filesystem::path SOCKET_FILE = TEJA_DIR / "socket";
const std::filesystem::path LOG_DIR = TEJA_DIR / "log";
const std::filesystem::path LOG_FILE_PREFIX = LOG_DIR / "server";

quill::FileSinkConfig& get_file_sink_config()
{
	thread_local quill::FileSinkConfig config{};
	config.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
	return config;
}

}

bool server::is_running()
{
	return false;
}

server::server()
{
	quill::Backend::start();
	auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(LOG_FILE_PREFIX, get_file_sink_config());
	_logger = quill::Frontend::create_or_get_logger("server", file_sink);
	LOG_INFO(_logger, "server spawning");

	remove_pid_file();
	destroy_socket();
	create_pid_file();
	create_socket();
	spin();
}

server::~server()
{
	LOG_INFO(_logger, "server shutting down");
	destroy_socket();
	remove_pid_file();
	quill::Backend::stop();
}

void server::client_disconnected(client_connection* conn)
{
	auto it = std::find_if(_client_conns.begin(), _client_conns.end(), [conn](const auto& c) {
			return c.get() == conn;
			});

	_client_conns.erase(it);
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
		LOG_INFO(_logger, "pid file at {} removed", PID_FILE.c_str());
	}

}

void server::create_socket()
{
	LOG_INFO(_logger, "creating socket");
	_server_socket = std::make_unique<unix_socket::server>(unix_socket::type::stream);
	LOG_INFO(_logger, "binding to {}", SOCKET_FILE.c_str());
	_server_socket->bind(SOCKET_FILE.c_str());
	LOG_INFO(_logger, "unix socket bound fd={}", _server_socket->fd());
	_server_socket->listen(&_runtime, this);
	LOG_INFO(_logger, "socket listening");
}

void server::destroy_socket()
{
	if (_server_socket)
	{
		_server_socket.reset();
		LOG_INFO(_logger, "socket destroyed");
	}

	if (std::filesystem::exists(SOCKET_FILE))
	{
		std::filesystem::remove(PID_FILE);
		LOG_INFO(_logger, "pid file at {} forcefully removed", PID_FILE.c_str());
	}
}

void server::spin()
{
	_runtime.run();
}

void server::on_new_connection(unix_socket::server*, std::unique_ptr<unix_socket::connection> conn)
{
	LOG_INFO(_logger, "new client connected fd={}", conn->fd());
	_client_conns.push_back(std::make_unique<client_connection>(this, std::move(conn)));
}

}
