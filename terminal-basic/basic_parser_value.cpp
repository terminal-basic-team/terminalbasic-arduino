/*
 * ucBASIC is a lightweight BASIC-like language interpreter
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

#include "basic_parser_value.hpp"

#include "math.hpp"
#include "basic_lexer.hpp"

namespace BASIC
{

Parser::Value::Value() :
type(INTEGER)
{
	value.integer = 0;
}

#if USE_LONGINT

Parser::Value::Value(LongInteger v) :
type(LONG_INTEGER)
{
	value.longInteger = v;
}
#endif

Parser::Value::Value(Integer v) :
type(INTEGER)
{
	value.integer = v;
}

#if USE_REALS

Parser::Value::Value(Real v) :
type(REAL)
{
	value.real = v;
}
#endif

Parser::Value::Value(bool v) :
type(BOOLEAN)
{
	value.boolean = v;
}

#if USE_REALS

Parser::Value::operator Real() const
{
	switch (type) {
#if USE_LONGINT
	case LONG_INTEGER:
		return (Real(value.longInteger));
#endif
	case INTEGER:
		return (Real(value.integer));
	case REAL:
		return (value.real);
	case BOOLEAN:
		return (Real(value.boolean));
	default:
		return (Real(NAN));
	}
}
#endif

Parser::Value::operator bool() const
{
	switch (type) {
#if USE_LONGINT
	case LONG_INTEGER:
		return (bool(value.longInteger));
#endif
	case INTEGER:
		return (bool(value.integer));
#if USE_REALS
	case REAL:
		return (bool(value.real));
#endif
	case BOOLEAN:
		return (value.boolean);
	default:
		return (false);
	}
}

#if USE_LONGINT

Parser::Value::operator LongInteger() const
{
	switch (type) {
	case LONG_INTEGER:
		return value.longInteger;
	case INTEGER:
		return LongInteger(value.integer);
#if USE_REALS
	case REAL:
		return LongInteger(value.real);
#endif
	case BOOLEAN:
		return LongInteger(value.boolean);
	default:
		return LongInteger(0);
	}
}
#endif

Parser::Value::operator Integer() const
{
	switch (type) {
#if USE_LONGINT
	case LONG_INTEGER:
		return Integer(value.longInteger);
#endif
	case INTEGER:
		return value.integer;
#if USE_REALS
	case REAL:
		return Integer(value.real);
#endif
	case BOOLEAN:
		return Integer(value.boolean);
	default:
		return Integer(0);
	}
}

Parser::Value&
Parser::Value::operator-()
{
	switch (type) {
#if USE_LONGINT
	case LONG_INTEGER:
		value.longInteger = -value.longInteger;
		break;
#endif
	case INTEGER:
		value.integer = -value.integer;
		break;
#if USE_REALS
	case REAL:
		value.real = -value.real;
		break;
#endif
	case BOOLEAN:
		value.boolean = !value.boolean;
		break;
	default:
		// undefined
		break;
	}
	return (*this);
}

bool
Parser::Value::operator<(const Value &rhs) const
{
	switch (type) {
#if USE_LONGINT
	case LONG_INTEGER:
		switch (rhs.type) {
		case LONG_INTEGER:
			return (value.longInteger < rhs.value.longInteger);
		case INTEGER:
			return (value.longInteger < rhs.value.integer);
#if USE_REALS
		case REAL:
			return (Real(value.integer) < rhs.value.real);
#endif
		case BOOLEAN:
			return (value.integer < Integer(rhs.value.boolean));
		}
#endif
	case INTEGER:
		switch (rhs.type) {
#if USE_LONGINT
		case LONG_INTEGER:
			return (value.integer < rhs.value.longInteger);
#endif
		case INTEGER:
			return (value.integer < rhs.value.integer);
#if USE_REALS
		case REAL:
			return (Real(value.integer) < rhs.value.real);
#endif
		case BOOLEAN:
			return (value.integer < Integer(rhs.value.boolean));
		}
#if USE_REALS
	case REAL:
		return (value.real < Real(rhs));
#endif
	case BOOLEAN:
		switch (rhs.type) {
		case INTEGER:
			return (Integer(value.boolean) < rhs.value.integer);
#if USE_LONGINT
		case LONG_INTEGER:
			return (LongInteger(value.boolean) < rhs.value.longInteger);
#endif
#if USE_REALS
		case REAL:
			return (Real(value.boolean) < rhs.value.real);
#endif
		case BOOLEAN:
			return (value.boolean < rhs.value.boolean);
		}
	}
}

bool
Parser::Value::operator==(const Value &rhs) const
{
#if USE_REALS
	if (type == REAL || rhs.type == REAL)
		return (almost_equal(Real(*this), Real(rhs), 2));
#endif
	switch (type) {
	case INTEGER:
		switch (rhs.type) {
		case INTEGER:
			return (value.integer == rhs.value.integer);
#if USE_LONGINT
		case LONG_INTEGER:
			return (value.integer == rhs.value.longInteger);
#endif
		case BOOLEAN:
			return (value.integer == Integer(rhs.value.boolean));
		}
#if USE_LONGINT
	case LONG_INTEGER:
		switch (rhs.type) {
		case INTEGER:
			return (value.longInteger == rhs.value.integer);
		case LONG_INTEGER:
			return (value.longInteger == rhs.value.longInteger);
		case BOOLEAN:
			return (value.longInteger == LongInteger(rhs.value.boolean));
		}
#endif // USE_LONGINT
	case BOOLEAN:
		switch (rhs.type) {
		case INTEGER:
			return (Integer(value.boolean) == rhs.value.integer);
#if USE_LONGINT
		case LONG_INTEGER:
			return (LongInteger(value.boolean) == rhs.value.longInteger);
#endif
		case BOOLEAN:
			return (value.boolean == rhs.value.boolean);
		}
	}
	return (false);
}

bool
Parser::Value::operator>(const Value &rhs) const
{
	switch (type) {
	case INTEGER:
		switch (rhs.type) {
		case INTEGER:
			return (value.integer > rhs.value.integer);
#if USE_LONGINT  
		case LONG_INTEGER:
			return (value.longInteger > rhs.value.integer);
#endif
#if USE_REALS
		case REAL:
			return (value.integer > rhs.value.real);
#endif
		case BOOLEAN:
			return (value.integer > Integer(rhs.value.boolean));
		}
#if USE_LONGINT  
	case LONG_INTEGER:
		switch (rhs.type) {
		case INTEGER:
			return (value.longInteger > rhs.value.integer);
		case LONG_INTEGER:
			return (value.longInteger > rhs.value.longInteger);
#if USE_REALS
		case REAL:
			return (value.longInteger > rhs.value.real);
#endif
		case BOOLEAN:
			return (value.integer > Integer(rhs.value.boolean));
		}
#endif
#if USE_REALS
	case REAL:
		return value.real > Real(rhs);
#endif
	case BOOLEAN:
		switch (rhs.type) {
		case INTEGER:
			return (Integer(value.boolean) > rhs.value.integer);
#if USE_LONGINT
		case LONG_INTEGER:
			return (LongInteger(value.boolean) > rhs.value.longInteger);
#endif
#if USE_REALS
		case REAL:
			return (Real(value.boolean) > rhs.value.real);
#endif
		case BOOLEAN:
			return (value.boolean > rhs.value.boolean);
		}
	}
	return (false);
}

bool
operator>=(const Parser::Value &l, const Parser::Value &r)
{
	return (l.operator>(r) || l.operator==(r));
}

bool
operator<=(const Parser::Value &l, const Parser::Value &r)
{
	return (l.operator<(r) || l.operator==(r));
}

Parser::Value&
    Parser::Value::operator+=(const Value &rhs)
{
#if USE_REALS
	value.real = Real(*this) + Real(rhs);
	type = Value::REAL;
#elif USE_LONGINT
	value.longInteger = LongInteger(*this) + LongInteger(rhs);
	type = Value::LONG_INTEGER;
#else
	value.integer = Integer(*this) + Integer(rhs);
	type = Value::INTEGER;
#endif
	return (*this);
}

Parser::Value&
    Parser::Value::operator-=(const Value &rhs)
{
#if USE_REALS
	value.real = Real(*this) - Real(rhs);
	type = Value::REAL;
#elif USE_LONGINT
	value.longInteger = LongInteger(*this) - LongInteger(rhs);
	type = Value::LONG_INTEGER;
#else
	value.integer = Integer(*this) - Integer(rhs);
	type = Value::INTEGER;
#endif
	return (*this);
}

Parser::Value&
    Parser::Value::operator*=(const Value &rhs)
{
#if USE_REALS
	value.real = Real(*this) * Real(rhs);
	type = Value::REAL;
#elif USE_LONGINT
	value.longInteger = LongInteger(*this) * LongInteger(rhs);
	type = Value::LONG_INTEGER;
#else
	value.integer = Integer(*this) * Integer(rhs);
	type = Value::INTEGER;
#endif
	return (*this);
}

Parser::Value&
    Parser::Value::operator/=(const Value &rhs)
{
#if USE_REALS
	value.real = Real(*this) / Real(rhs);
	type = Value::REAL;
#elif USE_LONGINT
	value.longInteger = LongInteger(*this) / LongInteger(rhs);
	type = Value::LONG_INTEGER;
#else
	value.integer = Integer(*this) / Integer(rhs);
	type = Value::INTEGER;
#endif
	return (*this);
}

void
Parser::Value::powerMatchValue(const Value &rhs)
{
#if USE_LONGINT
	if (rhs.type == LONG_INTEGER
#if USE_REALS
	    && type != REAL
#endif
	)
		* this = LongInteger(*this);
#endif
#if USE_REALS
	if (rhs.type == REAL || rhs < Integer(0))
		* this = Real(*this);
#endif
}

Parser::Value&
    Parser::Value::operator^=(const Value &rhs)
{
	powerMatchValue(rhs);
	switch (type) {
	case INTEGER:
	{
		Integer r = 1;
		for (Integer i = 0; i < Integer(rhs); ++i)
			r *= value.integer;
		value.integer = r;
	}
		break;
#if USE_LONGINT
	case LONG_INTEGER:
	{
		LongInteger r = 1;
		for (LongInteger i = 0; i < LongInteger(rhs); ++i)
			r *= value.longInteger;
		value.longInteger = r;
	}
		break;
#endif
#if USE_REALS
	case REAL:
		value.real = pow(value.real, Real(rhs));
		break;
#endif
	}
	return (*this);
}

void
Parser::Value::switchSign()
{
	switch (type) {
	case INTEGER:
		value.integer = -value.integer;
		break;
#if USE_LONGINT
	case LONG_INTEGER:
		value.longInteger = -value.longInteger;
		break;
#endif
#if USE_REALS
	case REAL:
		value.real = -value.real;
		break;
#endif
	case BOOLEAN:
		value.integer = -Integer(value.boolean);
		type = INTEGER;
		break;
	default:
		break;
	}
}

size_t
Parser::Value::printTo(Print& p) const
{
	switch (type) {
	case BOOLEAN:
		if (value.boolean)
			p.print(Lexer::tokenStrings[uint8_t(Token::KW_TRUE)]);
		else
			p.print(Lexer::tokenStrings[uint8_t(Token::KW_FALSE)]);
		break;
#if USE_REALS
	case REAL:
	{
		char buf[17];
#ifdef ARDUINO
		::dtostrf(value.real, 12, 9, buf);
#else
		::sprintf(buf, "% .7G", value.real);
#endif
		p.print(buf);
	}
		break;
#endif
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
		if (value.longInteger >= LongInteger(0))
			p.write(' ');
		p.print(value.longInteger);
		break;
#endif
	case Parser::Value::INTEGER:
		if (value.integer >= Integer(0))
			p.write(' ');
		p.print(value.integer);
		break;
	case Parser::Value::STRING:
	{
		p.print('?');
		break;
	}
	default:
		p.print('?');
		break;
	}
}

}
