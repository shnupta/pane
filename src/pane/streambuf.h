#pragma once

#include <span>
#include <vector>
namespace pane {

// dynamic buffer with source + sink cursors.
// push data into the source and flush it to the sink.
// typical usage is reading/writing into the source and
// flushing to the sink for consumers/producers.
class streambuf
{
public:
	using storage_t = char;
	using span_t = std::span<storage_t>;

	// 2MB default
	explicit streambuf(size_t initial_bufsize = 2 * 1024 * 1024);

	char* prepare_source(size_t size);
	void decrement_source(size_t size);
	void flush_source();

	span_t get_sink();
	void flush_sink(size_t size);

private:
	size_t _source_cursor = 0;
	size_t _sink_cursor = 0;
	size_t _flush_cursor = 0;
	std::vector<storage_t> _buf;

	void maybe_grow(size_t size);
	void maybe_reset();
};

}
