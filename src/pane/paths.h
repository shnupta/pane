#pragma once

#include <filesystem>

namespace pane {

inline std::filesystem::path get_home_dir()
{
	auto* homedir = getenv("HOME");
	if (homedir == nullptr)
	{
		throw std::runtime_error("can't find user home directory. $HOME is not set");
	}

	return std::filesystem::path(homedir);
}

const std::filesystem::path PANE_DIR = get_home_dir() / ".pane";
const std::filesystem::path PID_FILE = PANE_DIR / "pid";
const std::filesystem::path SOCKET_FILE = PANE_DIR / "socket";
const std::filesystem::path LOG_DIR = PANE_DIR / "log";
const std::filesystem::path LOG_FILE_PREFIX = LOG_DIR / "server";

}
