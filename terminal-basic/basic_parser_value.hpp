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
#include <Printable.h>

namespace BASIC
{

class EXT_PACKED Parser::Value : public Printable
{
public:

	enum Type : uint8_t
	{
		INTEGER,
#if USE_LONGINT
		LONG_INTEGER,
#endif
#if USE_REALS
		REAL,
#endif
		BOOLEAN, STRING
	};

	struct EXT_PACKED String
	{
		uint8_t size;
		//char string[STRINGSIZE];
	};

	union EXT_PACKED Body
	{
#if USE_LONGINT
		LongInteger longInteger;
#endif
		Integer integer;
#if USE_REALS
		Real real;
#endif
		bool boolean;
	};

	Value();
#if USE_LONGINT
	Value(LongInteger);
#endif
	Value(Integer);
#if USE_REALS
	Value(Real);
#endif
	Value(bool);

#if USE_REALS
	explicit operator Real() const;
#endif
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
	Value &operator|=(const Value&);
	Value &operator&=(const Value&);
	void switchSign();
	void notOp();
	
	static size_t size(Type);
	
	Type type;
	Body value;
private:
	/**
	 * @brief match value type with the power type
	 * @param 
	 */
	void powerMatchValue(const Value&);
// Printable interface
	size_t printTo(Print& p) const override;

};

}

#endif
