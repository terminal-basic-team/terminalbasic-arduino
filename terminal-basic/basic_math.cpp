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

#include "basic_math.hpp"
#include <math.h>
#include <string.h>

namespace BASIC
{

static const char sABS[] PROGMEM = "ABS";
static const char sACS[] PROGMEM = "ACS";
static const char sATN[] PROGMEM = "ATN";
static const char sCOS[] PROGMEM = "COS";
static const char sCOT[] PROGMEM = "COT";
static const char sEXP[] PROGMEM = "EXP";
static const char sLOG[] PROGMEM = "LOG";
static const char sPI[] PROGMEM = "PI";
static const char sRND[] PROGMEM = "RND";
static const char sSIN[] PROGMEM = "SIN";
static const char sSQR[] PROGMEM = "SQR";
static const char sTAN[] PROGMEM = "TAN";

PGM_P const Math::funcStrings[NUM_FUNC] PROGMEM = {
	sABS, sACS, sATN, sCOS, sCOT, sEXP, sLOG, sPI, sRND, sSIN, sSQR, sTAN
};

Math::Math(FunctionBlock *next) :
FunctionBlock(next)
{
}

FunctionBlock::function
Math::_getFunction(const char *name) const
{
	if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_ABS]))) == 0)
		return func_abs;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_ACS]))) == 0)
		return func_acs;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_ATN]))) == 0)
		return func_atn;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_COS]))) == 0)
		return func_cos;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_COT]))) == 0)
		return func_cot;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_EXP]))) == 0)
		return func_exp;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_LOG]))) == 0)
		return func_log;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_SIN]))) == 0)
		return func_sin;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_SQR]))) == 0)
		return func_sqr;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_RND]))) == 0)
		return func_rnd;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_PI]))) == 0)
		return func_pi;
	else if (strcmp_P(name, (PGM_P)pgm_read_word(&(funcStrings[F_TAN]))) == 0)
		return func_tan;
	else
		return NULL;
}

bool
Math::func_abs(Interpreter &i)
{
	Parser::Value v(Integer(0));
	i.popValue(v);
	if (v.type == Parser::Value::INTEGER || v.type == Parser::Value::REAL) {
		if (v < Parser::Value(Integer(0)))
			v.switchSign();
		i.pushValue(v);
		return true;
	} else
		return false;
}

bool
Math::func_acs(Interpreter &i)
{
	return general_func(i, &acs_r);
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
Math::func_rnd(Interpreter &i)
{
	Parser::Value v(Real(random())/Real(RANDOM_MAX));
	i.pushValue(v);
	return (true);
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
	return acos(v);
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

Real
Math::atn_r(Real v)
{
	return atan(v);
}

Real
Math::tan_r(Real v)
{
	return tan(v);
}

}
