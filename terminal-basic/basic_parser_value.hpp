/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016-2019 Andrey V. Skvortsov <starling13@mail.ru>
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
#include "basic_value.h"

#include <Printable.h>

namespace BASIC
{

class EXT_PACKED Parser::Value
{
public:
	/**
	 * @brief types of the value
	 */
	enum Type : uint8_t
	{
		INTEGER = BASIC_VALUE_TYPE_INTEGER,
#if USE_LONGINT
		LONG_INTEGER = BASIC_VALUE_TYPE_LONG_INTEGER,
#endif
#if USE_REALS
		REAL = BASIC_VALUE_TYPE_REAL,
#endif
		LOGICAL = BASIC_VALUE_TYPE_LOGICAL,
		STRING = BASIC_VALUE_TYPE_STRING
	};

	/**
	 * @brief Default constructor
	 * 
	 * Initializes a value with 0 INTEGER, which s always available
	 */
	Value()
	{
		basic_value_setFromInteger(&m_value, 0);
	}
	
#if USE_LONGINT
	/**
	 * @brief Constructor ftrom LongInteger number
	 * @param v
	 */
	Value(LongInteger v)
	{
		basic_value_setFromLongInteger(&m_value, v);
	}
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
	
	bool operator<(const Value&) const;
	bool operator==(const Value&) const;
	bool operator>(const Value&) const;
	friend bool operator >=(const Value&, const Value&);
	friend bool operator <=(const Value&, const Value&);

	Value &operator+=(const Value&);
	Value &operator-=(const Value&);
	Value &operator*=(const Value&);
	Value &operator/=(const Value&);
	Value &divEquals(const Value&);
	Value &modEquals(const Value&);
	Value &operator^=(const Value&);
	Value &operator|=(const Value&);
	Value &operator&=(const Value&);
	void switchSign();
	
	static size_t size(Type);
	
	Type type() const
	{
		return Type(m_value.type);
	}
	
	void setType(Type newVal)
	{
		m_value.type = basic_value_type_t(newVal);
	}
        
        basic_value_t m_value;
	
	// Printable interface
	size_t printTo(Print& p) const;
};

} // namespace BASIC

#endif // BASIC_PARSER_VALUE_HPP
