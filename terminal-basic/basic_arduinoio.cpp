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

#include "basic_arduinoio.hpp"
#include "Arduino.h"

#include <assert.h>

namespace BASIC
{

static const uint8_t arduinoIOFuncs[] PROGMEM = {
#if USE_REALS
	'A', 'R', 'E', 'A', 'D'+0x80,
#endif
	'A', 'R', 'E', 'A', 'D', '%'+0x80,
	'D', 'R', 'E', 'A', 'D'+0x80,
	0
};

const FunctionBlock::function ArduinoIO::_funcs[] PROGMEM = {
#if USE_REALS
	ArduinoIO::func_aread,
#endif
	ArduinoIO::func_aread_int,
	ArduinoIO::func_dread
};

static const uint8_t arduinoIOCommands[] PROGMEM = {
	'A', 'W', 'R', 'I', 'T', 'E'+0x80,
	'D', 'W', 'R', 'I', 'T', 'E'+0x80,
	0
};

const FunctionBlock::command  ArduinoIO::_commands[] PROGMEM = {
	ArduinoIO::comm_awrite,
	ArduinoIO::comm_dwrite
};

ArduinoIO::ArduinoIO(FunctionBlock *next) :
FunctionBlock(next)
{
	commands = _commands;
	commandTokens = arduinoIOCommands;
	functions = _funcs;
	functionTokens = arduinoIOFuncs;
}

#if USE_REALS
bool
ArduinoIO::func_aread(Interpreter &i)
{
	return (general_func(i, aread_r));
}
#endif

bool
ArduinoIO::func_aread_int(Interpreter &i)
{
	return (general_func(i, aread_i));
}

bool
ArduinoIO::func_dread(Interpreter &i)
{
	Parser::Value v(Integer(0));
	i.popValue(v);
#if USE_LONGINT
	if (v.type == Parser::Value::INTEGER ||
	    v.type == Parser::Value::LONG_INTEGER) {
#else
	if (v.type == Parser::Value::INTEGER) {
#endif
		pinMode(Integer(v), INPUT);
		v = bool(digitalRead(Integer(v)));
		i.pushValue(v);
		return (true);
	} else
		return (false);
}

bool
ArduinoIO::comm_awrite(Interpreter &i)
{
	Parser::Value v(Integer(0));
	i.popValue(v);
#if USE_LONGINT
	if (v.type == Parser::Value::INTEGER ||
	    v.type == Parser::Value::LONG_INTEGER) {
#else
	if (v.type == Parser::Value::INTEGER) {
#endif
		Parser::Value v2(Integer(0));
		i.popValue(v2);
#if USE_LONGINT
		if (v2.type == Parser::Value::INTEGER ||
		    v2.type == Parser::Value::LONG_INTEGER) {
#else
		if (v2.type == Parser::Value::INTEGER) {
#endif
			pinMode(Integer(v2), OUTPUT);
			analogWrite(Integer(v2), Integer(v));
			return (true);
		}
	}

	return (false);
}

bool
ArduinoIO::comm_dwrite(Interpreter &i)
{
	Parser::Value v(Integer(0));
	i.popValue(v);
	if (v.type == Parser::Value::BOOLEAN) {
		Parser::Value v2(Integer(0));
		i.popValue(v2);
#if USE_LONGINT
		if (v2.type == Parser::Value::INTEGER ||
		    v2.type == Parser::Value::LONG_INTEGER) {
#else
		if (v2.type == Parser::Value::INTEGER) {
#endif
			pinMode(Integer(v2), OUTPUT);
			digitalWrite(Integer(v2), bool(v));
			return (true);
		}
	}

	return (false);
}

#if USE_REALS
Real
ArduinoIO::aread_r(Real v)
{
	pinMode(v, INPUT);

	return Real(analogRead(v)) / Real(1023) * Real(5.0);
}
#endif

Integer
ArduinoIO::aread_i(Integer v)
{
	return analogRead(v);
}

}
