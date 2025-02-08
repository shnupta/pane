#pragma once

#include <quill/Logger.h>
#include <quill/LogMacros.h>

namespace teja {

void configure_file_logging();
void configure_console_logging();
quill::Logger* create_or_get_logger(const char* name);
	
}
