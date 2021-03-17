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

#include "basic_dataparser.hpp"

#if USE_DATA

namespace BASIC
{

DataParser::DataParser(Interpreter &interpreter) :
_interpreter(interpreter)
{	
}

bool
DataParser::searchData(const char *str, Parser::Value &value)
{
	_lexer.init(str);
	while (_lexer.getNext()) {
		if ((_lexer.getToken() == Token::KW_DATA) &&
		    _lexer.getNext())
			return readValue(value);
	}
	return false;
}

bool
DataParser::read(const char *str, Parser::Value &value)
{
	_lexer.init(str);
	if (_lexer.getNext() && (_lexer.getToken() == Token::COMMA)
	    && _lexer.getNext())
		return readValue(value);
	return false;
}

bool
DataParser::readValue(Parser::Value &value)
{
	bool minus = false;
	if (_lexer.getToken() == Token::MINUS) {
		if (!_lexer.getNext())
			return false;
		minus = true;
	}
	if ((_lexer.getToken() >= Token::C_INTEGER)
	 && (_lexer.getToken() <= Token::C_STRING)) {
		value = _lexer.getValue();
		if (minus)
			value.switchSign();
		if (_lexer.getToken() == Token::C_STRING)
			_interpreter.pushString(_lexer.id());
		return true;
	}
	return false;
}

} // namespace BASIC

#endif // USE_DATA
