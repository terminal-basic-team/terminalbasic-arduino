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
	'A', 'C', 'S', ASCII_NUL,
	'A', 'S', 'N', ASCII_NUL,
	'A', 'T', 'N', ASCII_NUL,
#endif // M_REVERSE_TRIGONOMETRIC
#if M_ADDITIONAL
	'C', 'B', 'R', ASCII_NUL,
#endif
#if M_HYPERBOLIC
        'C', 'O', 'S', 'H', ASCII_NUL,
#endif
#if M_TRIGONOMETRIC
	'C', 'O', 'S', ASCII_NUL,
	'C', 'O', 'T', ASCII_NUL,
#endif // M_TRIGONOMETRIC
	'E', 'X', 'P', ASCII_NUL,
	'L', 'O', 'G', ASCII_NUL,
	'P', 'I', ASCII_NUL,
#if M_HYPERBOLIC
	'S', 'I', 'N', 'H', ASCII_NUL,
#endif
#if M_TRIGONOMETRIC
	'S', 'I', 'N', ASCII_NUL,
#endif
	'S', 'Q', 'R', ASCII_NUL,
#if M_TRIGONOMETRIC
	'T', 'A', 'N', ASCII_NUL,
#endif
	ASCII_ETX
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
#if M_HYPERBOLIC
        Math::func_cosh,
#endif
#if M_TRIGONOMETRIC
	Math::func_cos,
	Math::func_cot,
#endif
	Math::func_exp,
	Math::func_log,
	Math::func_pi,
#if M_HYPERBOLIC
	Math::func_sinh,        
#endif
#if M_TRIGONOMETRIC
	Math::func_sin,
#endif
	Math::func_sqr
#if M_HYPERBOLIC
	,Math::func_tanh
#endif
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
	return sin(v);
}

Real
Math::cos_r(Real v)
{
	return cos(v);
}

Real
Math::cot_r(Real v)
{
	return Real(1) / tan(v);
}

Real
Math::tan_r(Real v)
{
	return tan(v);
}
#endif // M_TRIGONOMETRIC

#if M_HYPERBOLIC
bool
Math::func_cosh(Interpreter &i)
{
	return general_func(i, &cosh_r);
}

bool
Math::func_sinh(Interpreter &i)
{
	return general_func(i, &sinh_r);
}

bool
Math::func_tanh(Interpreter &i)
{
	return general_func(i, &tanh_r);
}

Real
Math::cosh_r(Real v)
{
	return cosh(v);
}

Real
Math::sinh_r(Real v)
{
	return sinh(v);
}

Real
Math::tanh_r(Real v)
{
	return tanh(v);
}
#endif // M_HYPERBOLIC

bool
Math::func_sqr(Interpreter &i)
{
	return general_func(i, &sqr_r);
}

bool
Math::func_pi(Interpreter &i)
{
	Parser::Value v(Real(M_PI));
	return i.pushValue(v);
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
	return acos(v);
}

Real
Math::asn_r(Real v)
{
	return asin(v);
}

Real
Math::atn_r(Real v)
{
	return atan(v);
}
#endif // M_REVERSE_TRIGONOMETRIC

Real
Math::exp_r(Real v)
{
	return exp(v);
}

Real
Math::log_r(Real v)
{
	return log(v);
}

Real
Math::sqr_r(Real v)
{
	return sqrt(v);
}

#if M_ADDITIONAL
Real
Math::cbr_r(Real v)
{
	return cbrt(v);
}
#endif

} // namespace BASIC

#endif // USEMATH
