#pragma once

#include <filesystem>

namespace teja {

inline std::filesystem::path get_home_dir()
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

}
