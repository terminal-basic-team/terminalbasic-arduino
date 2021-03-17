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
#include "basic_interpreter.hpp"
#include "Arduino.h"

#include <assert.h>

namespace BASIC
{

static const uint8_t intFuncs[] PROGMEM = {
	'A', 'B', 'S'+0x80,
	'R', 'N', 'D'+0x80,
	'T', 'I', 'M', 'E'+0x80,
	0
};

const FunctionBlock::function InternalFunctions::funcs[] PROGMEM = {
	InternalFunctions::func_abs,
	InternalFunctions::func_rnd,
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
		return (true);
	} else
		return (false);
}

bool
InternalFunctions::func_rnd(Interpreter &i)
{
#if USE_REALS
	Parser::Value v(Real(random()) / Real(RANDOM_MAX));
#else
	Parser::Value v(Integer(random()));
#endif
	i.pushValue(v);
	return (true);
}

bool
InternalFunctions::func_tim(Interpreter &i)
{
#if USE_REALS
	Real time = Real(millis()) / Real(1000);
#else
	Integer time = millis() / 1000;
#endif
	Parser::Value v(time);
	i.pushValue(v);
	return (true);
}

}
