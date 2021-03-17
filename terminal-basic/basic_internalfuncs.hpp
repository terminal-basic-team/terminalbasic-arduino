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

#ifndef BASIC_INTERNALFUNCS_HPP
#define BASIC_INTERNALFUNCS_HPP

#include "basic_functionblock.hpp"
#include "helper.hpp"

namespace BASIC
{

class InternalFunctions : public FunctionBlock
{
public:
	InternalFunctions(FunctionBlock* = NULL);
private:
	static bool func_abs(Interpreter&);
	static bool func_chr(Interpreter&);
	static bool func_int(Interpreter&);
#if USE_RANDOM
	static bool func_rnd(Interpreter&);
#endif
	static bool func_sgn(Interpreter&);
	static bool func_tim(Interpreter&);
#if USE_REALS
#define ___TYP Real
#elif USE_LONGINT
#define ___TYP LongInteger
#else
#define ___TYP Integer
#endif // USE_LONGINT
	static ___TYP sgn(___TYP);
#undef ___TYP
	
	static const FunctionBlock::function funcs[] PROGMEM;
};

}

#endif
