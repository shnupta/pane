#include "logging.h"
#include "../paths.h"
#include "../expect.h"

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/sinks/FileSink.h>
#include <quill/sinks/ConsoleSink.h>

namespace teja {


quill::FileSinkConfig& get_file_sink_config()
{
	thread_local quill::FileSinkConfig config{};
	config.set_filename_append_option(quill::FilenameAppendOption::StartDate);
	config.set_open_mode('a');
	return config;
}

class logging_state
{
public:
	static logging_state& get()
	{
		static logging_state ls{};
		return ls;
	}

	~logging_state()
	{
		quill::Backend::stop();
	}

	void configure_file_sink()
	{
		EXPECT(!_sink, "sink already configured, not valid");
		_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(LOG_FILE_PREFIX, get_file_sink_config());
	}

	std::shared_ptr<quill::Sink> sink()
	{
		EXPECT(_sink, "cannot get logger without a sink configured");
		return _sink;
	}

	void configure_console_sink()
	{
		EXPECT(!_sink, "sink already configured, not valid");
		_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("server");

	}

private:
	std::shared_ptr<quill::Sink> _sink;

	logging_state()
	{
		quill::Backend::start();
	}

};

void configure_file_logging()
{
	logging_state::get().configure_file_sink();
}

void configure_console_logging()
{
	logging_state::get().configure_console_sink();
}

quill::Logger* create_or_get_logger(const char* name)
{
	return quill::Frontend::create_or_get_logger(name, logging_state::get().sink());
}

}
