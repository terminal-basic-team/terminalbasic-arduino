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

/**
 * @file basic_arduinoio.hpp
 * @brief Arduino io container
 */

#ifndef BASIC_ARDUINOIO_HPP
#define BASIC_ARDUINOIO_HPP

#include "basic_functionblock.hpp"
#include "basic_interpreter.hpp"

namespace BASIC
{

class ArduinoIO : public FunctionBlock
{
public:
	explicit ArduinoIO(FunctionBlock* =NULL);
// FunctionBlock interface
protected:
	FunctionBlock::function _getFunction(const char*) const override;
	FunctionBlock::command _getCommand(const char*) const override;
private:
	static bool func_aread(Interpreter&);
	static bool func_aread_int(Interpreter&);
	static bool func_dread(Interpreter&);
	static bool comm_awrite(Interpreter&);
	static bool comm_dwrite(Interpreter&);
	
	static Real aread_r(Real);
	static Integer aread_i(Integer);
};

}

#endif
