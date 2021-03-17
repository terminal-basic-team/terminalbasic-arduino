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

#ifndef BASIC_GFX_HPP
#define BASIC_GFX_HPP

#include "basic_functionblock.hpp"
#include "basic_interpreter.hpp"

namespace BASIC
{

class GFXModule : public FunctionBlock
{
public:
	explicit GFXModule();

	static bool command_box(Interpreter&);
	static bool command_circle(Interpreter&);
	static bool command_color(Interpreter&);
	static bool command_line(Interpreter&);
	static bool command_lineto(Interpreter&);
	static bool command_point(Interpreter&);
	static bool command_screen(Interpreter&);
	
protected:
	
	void _init() override;

private:
	static const FunctionBlock::command comms[] PROGMEM;
};

}

#endif
