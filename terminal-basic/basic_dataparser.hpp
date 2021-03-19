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

/**
 * @file	basic_dataparser.hpp
 */

#ifndef BASIC_DATAPARSER_HPP
#define BASIC_DATAPARSER_HPP

#include "basic_interpreter.hpp"

namespace BASIC
{

class DataParser
{
public:
	DataParser(Interpreter&);
	bool searchData(const char*, Parser::Value&);
	bool read(const char*, Parser::Value&);
	const Lexer &lexer() const { return _lexer; }
private:
	bool readValue(Parser::Value&);
	Lexer	     _lexer;
	Interpreter &_interpreter;
};

} // namespace BASIC

#endif
