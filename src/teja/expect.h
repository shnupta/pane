#pragma once

#include <stdexcept>

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define EXPECT(x, ...) \
	if (!(x)) throw std::runtime_error("!(" STRINGIZE(x) ") " __VA_ARGS__)
