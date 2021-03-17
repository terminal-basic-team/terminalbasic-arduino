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

#include "basic.hpp"

#if USEMATH
#include "basic_math.hpp"
#include <math.h>
#include <string.h>

#include "ascii.hpp"
#include "helper.hpp"

namespace BASIC
{

static const uint8_t mathTokens[] PROGMEM = {
	'A', 'C', 'S'+0x80,
	'A', 'S', 'N'+0x80,
	'A', 'T', 'N'+0x80,
	'C', 'O', 'S'+0x80,
	'C', 'O', 'T'+0x80,
	'E', 'X', 'P'+0x80,
	'L', 'O', 'G'+0x80,
	'P', 'I'+0x80,
	'S', 'I', 'N'+0x80,
	'S', 'Q', 'R'+0x80,
	'T', 'A', 'N'+0x80,
	0
};

const FunctionBlock::function Math::funcs[] PROGMEM = {
	Math::func_acs,
	Math::func_asn,
	Math::func_atn,
	Math::func_cos,
	Math::func_cot,
	Math::func_exp,
	Math::func_log,
	Math::func_pi,
	Math::func_sin,
	Math::func_sqr,
	Math::func_tan
};

Math::Math(FunctionBlock *next) :
FunctionBlock(next)
{
	functions = funcs;
	functionTokens = mathTokens;
}

bool
Math::func_acs(Interpreter &i)
{
	return general_func(i, &acs_r);
}

bool
Math::func_asn(Interpreter &i)
{
	return general_func(i, &asn_r);
}

bool
Math::func_atn(Interpreter &i)
{
	return general_func(i, &atn_r);
}

bool
Math::func_cos(Interpreter &i)
{
	return general_func(i, &cos_r);
}

bool
Math::func_cot(Interpreter &i)
{
	return general_func(i, &cot_r);
}

bool
Math::func_exp(Interpreter &i)
{
	return general_func(i, &exp_r);
}

bool
Math::func_log(Interpreter &i)
{
	return general_func(i, &log_r);
}

bool
Math::func_sin(Interpreter &i)
{
	return general_func(i, &sin_r);
}

bool
Math::func_sqr(Interpreter &i)
{
	return general_func(i, &sqr_r);
}

bool
Math::func_pi(Interpreter &i)
{
	Parser::Value v(Real(M_PI));
	i.pushValue(v);
	return (true);
}

bool
Math::func_tan(Interpreter &i)
{
	return general_func(i, &tan_r);
}

Real
Math::acs_r(Real v)
{
	return (acos(v));
}

Real
Math::asn_r(Real v)
{
	return (asin(v));
}

Real
Math::sin_r(Real v)
{
	return (sin(v));
}

Real
Math::cos_r(Real v)
{
	return (cos(v));
}

Real
Math::cot_r(Real v)
{
	return (Real(1) / tan(v));
}

Real
Math::exp_r(Real v)
{
	return (exp(v));
}

Real
Math::log_r(Real v)
{
	return (log(v));
}

Real
Math::sqr_r(Real v)
{
	return (sqrt(v));
}

Real
Math::atn_r(Real v)
{
	return (atan(v));
}

Real
Math::tan_r(Real v)
{
	return (tan(v));
}

}

#endif
