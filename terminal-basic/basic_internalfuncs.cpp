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

#include "basic_internalfuncs.hpp"

#include "math.hpp"
#include "basic_interpreter.hpp"
#include "Arduino.h"
#include "basic_program.hpp"

#include <assert.h>

namespace BASIC
{

static const uint8_t intFuncs[] PROGMEM = {
	'A', 'B', 'S', ASCII_NUL,
#if USE_ASC
	'A', 'S', 'C', ASCII_NUL,
#endif
#if USE_CHR
	'C', 'H', 'R', '$', ASCII_NUL,
#endif
#if USE_GET
	'G', 'E', 'T', '$', ASCII_NUL,
#endif
#if USE_REALS
	'I', 'N', 'T', ASCII_NUL,
#endif
#if USE_LEFT
	'L', 'E', 'F', 'T', '$', ASCII_NUL,
#endif	
#if USE_LEN
	'L', 'E', 'N', ASCII_NUL,
#endif
#if USE_PEEK_POKE
	'P', 'E', 'E', 'K', ASCII_NUL,
#endif
	'R', 'E', 'S', ASCII_NUL,
#if USE_RIGHT
	'R', 'I', 'G', 'H', 'T', '$', ASCII_NUL,
#endif
#if USE_RANDOM
	'R', 'N', 'D', ASCII_NUL,
#endif
	'S', 'G', 'N', ASCII_NUL,
        'S', 'T', 'R', '$', ASCII_NUL,
	'T', 'I', 'M', 'E', ASCII_NUL,
	ASCII_ETX
};

const FunctionBlock::function InternalFunctions::funcs[] PROGMEM = {
	InternalFunctions::func_abs,
#if USE_ASC
	InternalFunctions::func_asc,
#endif
#if USE_CHR
	InternalFunctions::func_chr,
#endif
#if USE_GET
	InternalFunctions::func_get,
#endif
#if USE_REALS
	InternalFunctions::func_int,
#endif
#if USE_LEFT
	InternalFunctions::func_left,
#endif
#if USE_LEN
	InternalFunctions::func_len,
#endif
#if USE_PEEK_POKE
	InternalFunctions::func_peek,
#endif
	InternalFunctions::func_result,
#if USE_RIGHT
	InternalFunctions::func_right,
#endif
#if USE_RANDOM
	InternalFunctions::func_rnd,
#endif
	InternalFunctions::func_sgn,
	InternalFunctions::func_str,
	InternalFunctions::func_tim
};

InternalFunctions::InternalFunctions(FunctionBlock *first) :
FunctionBlock(first)
{
	functions = funcs;
	functionTokens = intFuncs;
}

bool
InternalFunctions::func_abs(Interpreter &i)
{
	Parser::Value v(Integer(0));
	i.popValue(v);
	if (v.type() == Parser::Value::INTEGER
#if USE_LONGINT
	 || v.type() == Parser::Value::LONG_INTEGER
#endif
#if USE_REALS
	 || v.type() == Parser::Value::REAL
#endif
	    ) {
		if (v < Parser::Value(Integer(0)))
			v.switchSign();
		if (i.pushValue(v))
			return true;
	}
        return false;
}

#if USE_ASC
bool
InternalFunctions::func_asc(Interpreter &i)
{
	const char *str;
	if (i.popString(str)) {
		Parser::Value v;
		v = Integer(str[0]);
		if (i.pushValue(v))
			return true;
	}
	return false;
}
#endif // USE_ASC

#if USE_CHR
bool
InternalFunctions::func_chr(Interpreter &i)
{
	INT iv;
	if (getIntegerFromStack(i, iv)) {
		char buf[2] = { iv ,0 };
		Parser::Value v;
		v.setType(Parser::Value::STRING);
		i.pushString(buf);
		if (i.pushValue(v))
			return true;
	}
	return false;
}
#endif

#if USE_GET
bool
InternalFunctions::func_get(Interpreter &i)
{
	Parser::Value v;
	char buf[2] = {0,0};
	buf[0] = i.lastKey();
	v.setType(Parser::Value::STRING);
	i.pushString(buf);
	return i.pushValue(v);
}
#endif // USE_GET

#if USE_PEEK_POKE
bool
InternalFunctions::func_peek(Interpreter &i)
{
	INT addr;
	if (getIntegerFromStack(i, addr)) {
		Parser::Value v(Integer(*((volatile uint8_t*)(addr))));
		if (i.pushValue(v))
                    return true;
	}
	return false;
}
#endif // USE_PEEK_POKE

bool
InternalFunctions::func_result(Interpreter &i)
{
	return i.pushResult();
}

#if USE_REALS
bool
InternalFunctions::func_int(Interpreter &i)
{
	Parser::Value v(Integer(0));
	i.popValue(v);
	if (v.type() == Parser::Value::INTEGER
#if USE_LONGINT
	 || v.type() == Parser::Value::LONG_INTEGER
#endif
	 || v.type() == Parser::Value::REAL
	    ) {
		v = math<Real>::floor(Real(v));
#if USE_LONGINT
		v = LongInteger(v);
#else
		v = Integer(v);
#endif
		if (i.pushValue(v))
                    return true;
	}
        return false;
}
#endif // USE_REALS


#if USE_LEFT
bool
InternalFunctions::func_left(Interpreter &i)
{
	INT len;
	if (getIntegerFromStack(i, len)) {
		const char *str;
		if (i.popString(str)) {
			char buf[STRING_SIZE];
			strncpy(buf, str, STRING_SIZE);
			const uint8_t pos = min(len, strlen(str));
			buf[pos] = char(0);
			i.pushString(buf);
			Parser::Value v;
			v.setType(Parser::Value::STRING);
			if (i.pushValue(v))
				return true;
		}
	}
	return false;
}
#endif // USE_LEFT

#if USE_RIGHT
bool
InternalFunctions::func_right(Interpreter &i)
{
	INT len;
	if (getIntegerFromStack(i, len)) {
		const char *str;
		if (i.popString(str)) {
			char buf[STRING_SIZE];
			strncpy(buf, str, STRING_SIZE);
			const uint8_t strl = strlen(str);
			len = min(len, strl);
			i.pushString(buf+strl-len);
			Parser::Value v;
			v.setType(Parser::Value::STRING);
			if (i.pushValue(v))
				return true;
		}
	}
	return false;
}
#endif // USE_RIGHT

#if USE_LEN
bool
InternalFunctions::func_len(Interpreter &i)
{
	const char *str;
	if (i.popString(str)) {
		Parser::Value v(Integer(strnlen(str, STRING_SIZE)));
		if (i.pushValue(v))
			return true;
	}
	return false;
}
#endif

#if USE_REALS
#define TYP Real
#else
#define TYP INT
#endif // USE_REALS
TYP
InternalFunctions::sgn(TYP v)
{
	if (v > TYP(0))
		return TYP(1);
	else if (v == TYP(0))
		return TYP(0);
	else
		return TYP(-1);
}
#undef TYP

bool
InternalFunctions::func_sgn(Interpreter &i)
{
	return general_func(i, sgn);
}

class BufferPrint : public Print
{
public:
	BufferPrint() : pointer(0) {}
	
	size_t write(uint8_t c) override
	{
		if (pointer < sizeof(buf)) {
			buf[pointer++] = c;
			return 1;
		} else
			return -1;
	}

	char buf[STRING_SIZE];
	uint8_t pointer;
};

bool
InternalFunctions::func_str(Interpreter &i)
{
	BufferPrint p;
	Parser::Value v;
	i.popValue(v);
	size_t res = v.printTo(p);
	if (res >= sizeof(p.buf))
		res = sizeof(p.buf)-1;
	p.buf[res] = '\0';
	v.setType(Parser::Value::STRING);
	i.pushString(p.buf);
	return (i.pushValue(v));
}

#if USE_RANDOM
bool
InternalFunctions::func_rnd(Interpreter &i)
{
	INT val;
	getIntegerFromStack(i, val);
#if USE_REALS
	Parser::Value v(Real(random(0x7FFFFFFF)) / Real(0x7FFFFFFF));
#else
	Parser::Value v(Integer(random(0x7FFFFFFF)));
#endif
	return i.pushValue(v);
}
#endif // USE_RANDOM

bool
InternalFunctions::func_tim(Interpreter &i)
{
#if USE_REALS
#define TYP Real
#elif USE_LONGINT
#define TYP LongInteger
#else
#define TYP Integer
#endif
	return i.pushValue(TYP(TYP(millis()) / TYP(1000)));
}

} // namespace BASIC
