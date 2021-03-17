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
#if M_REVERSE_TRIGONOMETRIC
	'A', 'C', 'S'+0x80,
	'A', 'S', 'N'+0x80,
	'A', 'T', 'N'+0x80,
#endif // M_REVERSE_TRIGONOMETRIC
#if M_ADDITIONAL
	'C', 'B', 'R'+0x80,
#endif
#if M_TRIGONOMETRIC
	'C', 'O', 'S'+0x80,
	'C', 'O', 'T'+0x80,
#endif // M_TRIGONOMETRIC
	'E', 'X', 'P'+0x80,
	'L', 'O', 'G'+0x80,
	'P', 'I'+0x80,
#if M_TRIGONOMETRIC
	'S', 'I', 'N'+0x80,
#endif
	'S', 'Q', 'R'+0x80,
#if M_TRIGONOMETRIC
	'T', 'A', 'N'+0x80,
#endif
	0
};

const FunctionBlock::function Math::funcs[] PROGMEM = {
#if M_REVERSE_TRIGONOMETRIC
	Math::func_acs,
	Math::func_asn,
	Math::func_atn,
#endif
#if M_ADDITIONAL
	Math::func_cbr,
#endif
#if M_TRIGONOMETRIC
	Math::func_cos,
	Math::func_cot,
#endif
	Math::func_exp,
	Math::func_log,
	Math::func_pi,
#if M_TRIGONOMETRIC
	Math::func_sin,
#endif
	Math::func_sqr
#if M_TRIGONOMETRIC
	,Math::func_tan
#endif
};

Math::Math()
{
	functions = funcs;
	functionTokens = mathTokens;
}

#if M_REVERSE_TRIGONOMETRIC

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

#endif // M_REVERSE_TRIGONOMETRIC

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

#if M_TRIGONOMETRIC
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
Math::func_sin(Interpreter &i)
{
	return general_func(i, &sin_r);
}

bool
Math::func_tan(Interpreter &i)
{
	return general_func(i, &tan_r);
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
Math::tan_r(Real v)
{
	return (tan(v));
}
#endif // M_TRIGONOMETRIC

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
	return true;
}

#if M_ADDITIONAL
bool
Math::func_cbr(Interpreter &i)
{
	return general_func(i, &cbr_r);
}
#endif

#if M_REVERSE_TRIGONOMETRIC
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
Math::atn_r(Real v)
{
	return (atan(v));
}
#endif // M_REVERSE_TRIGONOMETRIC

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

#if M_ADDITIONAL
Real
Math::cbr_r(Real v)
{
	return (cbrt(v));
}
#endif

}

#endif // USEMATH
