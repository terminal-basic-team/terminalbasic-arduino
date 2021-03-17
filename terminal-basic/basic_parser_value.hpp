/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016, 2017 Andrey V. Skvortsov <starling13@mail.ru>
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

#ifndef BASIC_PARSER_VALUE_HPP
#define BASIC_PARSER_VALUE_HPP

#include "basic_parser.hpp"

namespace BASIC
{

class CPS_PACKED Parser::Value
{
public:

	enum Type : uint8_t
	{
		INTEGER,
#if USE_LONGINT
		    LONG_INTEGER,
#endif
		REAL, BOOLEAN, STRING
	};

	struct CPS_PACKED String
	{
		uint8_t size;
		//char string[STRINGSIZE];
	};

	union CPS_PACKED Body
	{
#if USE_LONGINT
		LongInteger longInteger;
#endif
		Integer integer;
		Real real;
		bool boolean;
	};

	Value();
#if USE_LONGINT
	Value(LongInteger);
#endif
	Value(Integer);
	Value(float);
	Value(bool);

	explicit operator Real() const;
	explicit operator bool() const;
	explicit operator Integer() const;
#if USE_LONGINT
	explicit operator LongInteger() const;
#endif

	Value &operator-();
	
	bool operator<(const Value&) const;
	bool operator==(const Value&) const;
	bool operator>(const Value&) const;
	friend bool operator >=(const Value&, const Value&);
	friend bool operator <=(const Value&, const Value&);
	Value &operator+=(const Value&);
	Value &operator-=(const Value&);
	Value &operator*=(const Value&);
	Value &operator/=(const Value&);
	Value &operator^=(const Value&);
	void switchSign();

	Type type;
	Body value;
};
}

#endif
