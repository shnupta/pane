#include "server.h"

#include <cstdlib>
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
	_socket = std::make_unique<unix_socket>(unix_socket::type::stream);
	LOG_INFO(_logger, "binding to {}", SOCKET_FILE.c_str());
	_socket->bind(SOCKET_FILE.c_str());
	LOG_INFO(_logger, "unix socket bound");
	_socket->listen(&_runtime);
	LOG_INFO(_logger, "socket listening");
}

void server::destroy_socket()
{
	if (_socket)
	{
		_socket.reset();
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

}
