/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016  Andrey V. Skvortsov <starling13@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARDUINO_LOGGER_HPP
#define ARDUINO_LOGGER_HPP

#include "cps.hpp"
#include "Stream.h"
#include "Arduino.h"

#define ARDUINO_LOG 0

#if ARDUINO_LOG
#define LOG_INIT(T) Logger::init(T)
#define LOG(args...) Logger::log(args)
#define LOG_TRACE LOG(__PRETTY_FUNCTION__)
#else
#define LOG_INIT(T)
#define LOG(...)
#define LOG_TRACE
#endif

Package(Logger) {
	public:

	enum class format_t
	{
		bin = 2, dec = 10, oct = 8, hex = 16, endl
	};

	static void init(Stream&);

	template<typename... Args>
	    static void log(const Args&... args)
	{
		_log(args...);
		_instance._stream->println();
	}

	private:

	friend Logger &operator<<(Logger &logger, format_t formaat)
	{
		logger._format = formaat;
		return (logger);
	}
	
	friend Logger &operator<<(Logger &logger, const Printable &p) {
		logger._stream->print(p);
		return (logger);
	}

	friend Logger &operator<<(Logger &logger, uint8_t val) {
		logger._stream->print(val, int(_format));
		return (logger);
	}

	friend Logger &operator<<(Logger &logger, const char* first) {
		logger._stream->print(first);
		return (logger);
	}

	friend Logger &operator<<(Logger &logger, char* first) {
		logger._stream->print(first);
		return (logger);
	}

	template<typename T>
	friend Logger &operator<<(Logger &logger, T * first)
	{
		logger._stream->print(intptr_t(first), 16);
		return (logger);
	}

	template<typename T>
	friend Logger &operator<<(Logger &logger, T first)
	{
		logger._stream->print(first);
		return (logger);
	}

	template<typename T>
	static void _log(T first)
	{
		_instance << first;
	}

	template<typename T, typename... Args>
	static void _log(T first, const Args&... args)
	{
		_log(first);
		_instance._stream->print(" ");
		_log(args...);
	}

	static Logger _instance;

	static format_t _format;

	Stream *_stream;
};

#endif
