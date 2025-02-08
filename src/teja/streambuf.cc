#include "streambuf.h"
#include "expect.h"

namespace teja {

streambuf::streambuf(size_t initial_bufsize)
{
	_buf.resize(initial_bufsize);
}

streambuf::storage_t* streambuf::prepare_source(size_t size)
{
	maybe_grow(size);
	auto* ptr = _buf.data() + _source_cursor;
	_source_cursor += size;
	return ptr;
}

void streambuf::decrement_source(size_t size)
{
	_source_cursor -= size;
	EXPECT(_source_cursor >= _flush_cursor, "source cursor decremented to be before flush cursor");
}

void streambuf::flush_source()
{
	_flush_cursor = _source_cursor;
}

streambuf::span_t streambuf::get_sink()
{
	EXPECT(_sink_cursor <= _flush_cursor, "sink overtook flush");
	if (_sink_cursor == _flush_cursor) return {};

	size_t available = _flush_cursor - _sink_cursor;
	return {_buf.data() + _sink_cursor, available};
}

void streambuf::flush_sink(size_t size)
{
	_sink_cursor += size;
	EXPECT(_sink_cursor <= _flush_cursor, "sink overtook flush");
	maybe_reset();
}

void streambuf::maybe_grow(size_t size)
{
	if (_source_cursor + size > _buf.size())
	{
		_buf.resize(_buf.size() * 2);
	}
}

void streambuf::maybe_reset()
{
	if (_sink_cursor == _source_cursor)
	{
		_sink_cursor = 0;
		_source_cursor = 0;
		_flush_cursor = 0;
	}
}

}
