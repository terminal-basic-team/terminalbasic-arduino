/*
 * ucBASIC is a lightweight BASIC-like language interpreter
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

#include "basic_arduinoio.hpp"
#include "Arduino.h"

#include <assert.h>

namespace BASIC
{

ArduinoIO::ArduinoIO(FunctionBlock *next) :
FunctionBlock(next)
{
}

FunctionBlock::function
ArduinoIO::_getFunction(const char *name) const
{
	assert(name != NULL);
	uint8_t position = 0;
	char c = name[position];
	if (c != 0) {
		switch (c) {
		case 'A':
			++position;
			if (name[position] == 'R') {
				++position;
				if (name[position] == 'E') {
					++position;
					if (name[position] == 'A') {
						++position;
						if (name[position] == 'D') {
							++position;
							if (name[position] == '%') {
								++position;
								if (name[position] == 0)
									return (func_aread_int);
							} else if (name[position] == 0)
								return (func_aread);
						}
					}
				}
			}
			break;
		case 'D':
			++position;
			if (name[position] == 'R') {
				++position;
				if (name[position] == 'E') {
					++position;
					if (name[position] == 'A') {
						++position;
						if (name[position] == 'D') {
							++position;
							if (name[position] == '!') {
								++position;
								if (name[position] == 0)
									return (func_dread);
							}
						}
					}
				}
			}
			break;
		}
	}
	return (NULL);
}

FunctionBlock::command
ArduinoIO::_getCommand(const char *name) const
{
	assert(name != NULL);
	uint8_t position = 0;
	char c = name[position];
	if (c != 0) {
		switch (c) {
		case 'D':
			++position;
			if (name[position] == 'W') {
				++position;
				if (name[position] == 'R') {
					++position;
					if (name[position] == 'I') {
						++position;
						if (name[position] == 'T') {
							++position;
							if (name[position] == 'E') {
								++position;
								if (name[position] == 0)
									return (comm_dwrite);
							}
						}
					}
				}
			}
			break;
		}
	}
	return (NULL);
}

bool
ArduinoIO::func_aread(Interpreter &i)
{
	return (general_func(i, aread_r));
}

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
	if (v.type == Parser::Value::INTEGER) {
		pinMode(Integer(v), INPUT);
		v = bool(digitalRead(Integer(v)));
		i.pushValue(v);
		return (true);
	} else
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
		if (v2.type == Parser::Value::INTEGER) {
			pinMode(Integer(v2), OUTPUT);
			digitalWrite(Integer(v2), bool(v));
			return (true);
		}
	}
	
	return (false);
}

Real
ArduinoIO::aread_r(Real v)
{
	pinMode(v, INPUT);

	return Real(analogRead(v)) / Real(1023) * Real(5.0);
}

Integer
ArduinoIO::aread_i(Integer v)
{
	return analogRead(v);
}

}
