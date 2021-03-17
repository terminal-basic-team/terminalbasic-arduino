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

#include "basic_internalfuncs.hpp"

#include "math.hpp"
#include "basic_interpreter.hpp"
#include "Arduino.h"
#include "basic_program.hpp"

#include <assert.h>

namespace BASIC
{

static const uint8_t intFuncs[] PROGMEM = {
	'A', 'B', 'S'+0x80,
#if USE_ASC
	'A', 'S', 'C'+0x80,
#endif
#if USE_CHR
	'C', 'H', 'R', '$'+0x80,
#endif
#if USE_GET
	'G', 'E', 'T', '$'+0x80,
#endif
#if USE_REALS
	'I', 'N', 'T'+0x80,
#endif
#if USE_LEN
	'L', 'E', 'N'+0x80,
#endif
	'R', 'E', 'S'+0x80,
#if USE_RANDOM
	'R', 'N', 'D'+0x80,
#endif
	'S', 'G', 'N'+0x80,
        'S', 'T', 'R', '$'+0x80,
	'T', 'I', 'M', 'E'+0x80,
	0
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
#if USE_LEN
	InternalFunctions::func_len,
#endif
	InternalFunctions::func_result,
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
	if (v.type == Parser::Value::INTEGER
#if USE_LONGINT
	 || v.type == Parser::Value::LONG_INTEGER
#endif
#if USE_REALS
	 || v.type == Parser::Value::REAL
#endif
	    ) {
		if (v < Parser::Value(Integer(0)))
			v.switchSign();
		i.pushValue(v);
		return true;
	} else
		return false;
}

#if USE_ASC
bool
InternalFunctions::func_asc(Interpreter &i)
{
	Parser::Value v;
	i.popValue(v);
	if (v.type == Parser::Value::STRING) {
		const char *str;
		i.popString(str);
		v = Integer(str[0]);
		i.pushValue(v);
		return true;
	} else
		return false;
}
#endif

#if USE_CHR
bool
InternalFunctions::func_chr(Interpreter &i)
{
	Parser::Value v;
	i.popValue(v);
	char buf[2] = {0,};
	buf[0] = Integer(v);
	v.type = Parser::Value::STRING;
	i.pushString(buf);
	i.pushValue(v);
	return true;
}
#endif

#if USE_GET
bool
InternalFunctions::func_get(Interpreter &i)
{
	Parser::Value v;
	char buf[2] = {0,};
	buf[0] = i.lastKey();
	v.type = Parser::Value::STRING;
	i.pushString(buf);
	i.pushValue(v);
	return true;
}
#endif // USE_GET

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
	if (v.type == Parser::Value::INTEGER
#if USE_LONGINT
	 || v.type == Parser::Value::LONG_INTEGER
#endif
	 || v.type == Parser::Value::REAL
	    ) {
		v = math<Real>::floor(Real(v));
#if USE_LONGINT
		v = LongInteger(v);
#else
		v = Integer(v);
#endif
		i.pushValue(v);
		return true;
	} else
		return false;
}
#endif // USE_REALS

#if USE_LEN
bool
InternalFunctions::func_len(Interpreter &i)
{
	Parser::Value v;
	i.popValue(v);
	if (v.type == Parser::Value::STRING) {
		const char *str;
		if (i.popString(str)) {
			v = Integer(strnlen(str, STRINGSIZE));
			i.pushValue(v);
			return true;
		} else
			return false;
	} else
		return false;
}
#endif

#if USE_REALS
#define TYP Real
#elif USE_LONGINT
#define TYP LongInteger
#else
#define TYP Integer
#endif // USE_LONGINT
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

	char buf[STRINGSIZE];
	uint8_t pointer;
};

bool
InternalFunctions::func_str(Interpreter &i)
{
	BufferPrint p;
	Parser::Value v;
	i.popValue(v);
	size_t res = p.print(v);
	if (res >= sizeof(p.buf))
		res = sizeof(p.buf)-1;
	p.buf[res] = '\0';
	v.type = Parser::Value::STRING;
	i.pushString(p.buf);
	i.pushValue(v);
	return true;
}

#if USE_RANDOM
bool
InternalFunctions::func_rnd(Interpreter &i)
{
#if USE_REALS
	Parser::Value v(Real(random(0x7FFFFFFF)) / Real(0x7FFFFFFF));
#else
	Parser::Value v(Integer(random(0x7FFFFFFF)));
#endif
	i.pushValue(v);
	return true;
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
	i.pushValue(TYP(TYP(millis()) / TYP(1000)));
	return true;
}

} // namespace BASIC
