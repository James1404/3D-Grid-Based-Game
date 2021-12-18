#pragma once
#include <iostream>
#include <string>

namespace logger {
	enum class LogType {
		INFO = 0,
		WARNING,
		ERROR
	};

	template <typename ...Args>
	inline void log(LogType _type, Args&& ..._args) {
		// TODO: output log into file if in release mode.

		switch (_type) {
		case LogType::INFO:
			std::cout << "Info: ";
			break;
		case LogType::WARNING:
			std::cout << "Warning: ";
			break;
		case LogType::ERROR:
			std::cout << "Error: ";
			break;
		}

		(std::cout << ... << _args);
		std::cout << std::endl;
	}

	template <typename ...Args>
	inline void info(std::string _message, Args&& ..._args) {
		log(LogType::INFO, _message, _args...);
	}

	template <typename ...Args>
	inline void warning(std::string _message, Args&& ..._args) {
		log(LogType::WARNING, _message, _args...);
	}

	template <typename ...Args>
	inline void error(std::string _message, Args&& ..._args) {
		log(LogType::ERROR, _message, _args...);
	}
}