/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
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

#ifndef BASIC_FUNCTIONBLOCK_HPP
#define BASIC_FUNCTIONBLOCK_HPP

#include <stdlib.h>
#include "basic.hpp"

namespace BASIC
{

class Interpreter;

class FunctionBlock
{
public:

	typedef bool (*function)(Interpreter&);
	
	typedef bool (*command)(Interpreter&);
	
	function getFunction(const char*) const;
	
	command getCommand(const char*) const;
	
	void init();
	
	FunctionBlock *next() { return _next; }
	void setNext(FunctionBlock *newVal) { _next = newVal; }
	
protected:
	
	typedef Real (*_funcReal)(Real);
	typedef Integer (*_funcInteger)(Integer);
	
	explicit FunctionBlock(FunctionBlock* =NULL);
	
	virtual void _init() {}
	/**
	 * @brief return pointer to the function of this block
	 *   or NULL if no one
	 * @param fName function name
	 * @return function pointer
	 */
	virtual function _getFunction(const char*) const { return NULL; }
	/**
	 * @brief return pointer to the command, provided by this block
	 *   or NULL if no one
	 * @param fName command name
	 * @return command pointer
	 */
	virtual command _getCommand(const char*) const { return NULL; }
	/**
	 * @brief general function wrapper with 1 Real argument
	 * @param interpreter Interpreter object
	 * @param func underlying function pointer
	 * @return ok status
	 */
	static bool general_func(Interpreter&, _funcReal);
	/**
	 * @brief general function wrapper with 1 Real argument
	 * @param interpreter Interpreter object
	 * @param func underlying function pointer
	 * @return ok status
	 */
	static bool general_func(Interpreter&, _funcInteger);
private:
	FunctionBlock *_next;
};

}

#endif
