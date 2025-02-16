#include "logging.h"
#include "../paths.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/cfg/env.h>

namespace pane {

void configure_file_logging()
{
	spdlog::details::registry::instance().drop_all();
	auto new_logger = spdlog::basic_logger_mt("", LOG_FILE_PREFIX.string());
	spdlog::set_default_logger(new_logger);
	spdlog::flush_on(spdlog::level::level_enum::trace);
}

}
