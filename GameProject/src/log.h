#pragma once
#include <iostream>
#include <string>

template <typename ...Args>
inline void log(Args&& ..._args) {
	// TODO: output log into file if in release mode.
	(std::cout << ... << _args);
	std::cout << std::endl;
}

#define log_info(...) log("", __VA_ARGS__);
#define log_warning(...) log("'", __FILE__, "' Warning at Line(", __LINE__, "): ", __VA_ARGS__);
#define log_error(...) log("'", __FILE__, "' Error at Line(", __LINE__, "): ", __VA_ARGS__);
