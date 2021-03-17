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

/**
 * @file basic_math.hpp
 * @brief Mathematical functions container
 */

#ifndef BASIC_MATH_HPP
#define BASIC_MATH_HPP

#include "basic_functionblock.hpp"
#include "basic_interpreter.hpp"

namespace BASIC
{

class Math : public FunctionBlock
{
public:
	explicit Math(FunctionBlock* =NULL);
// FunctionBlock interface
protected:
	FunctionBlock::function _getFunction(const char*) const override;
private:
	static bool func_abs(Interpreter&);
	static bool func_acs(Interpreter&);
	static bool func_atn(Interpreter&);
	static bool func_cos(Interpreter&);
	static bool func_cot(Interpreter&);
	static bool func_exp(Interpreter&);
	static bool func_log(Interpreter&);
	static bool func_sin(Interpreter&);
	static bool func_sqr(Interpreter&);
	static bool func_pi(Interpreter&);
	static bool func_tan(Interpreter&);
	
	enum FuncNames : uint8_t
	{
		F_ABS = 0, F_ACS, F_ATN, F_COS, F_COT, F_EXP, F_LOG, F_PI,
		F_SIN, F_SQR, F_TAN, NUM_FUNC
	};
	
	static Real acs_r(Real);
	static Real atn_r(Real);
	static Real sin_r(Real);
	static Real cos_r(Real);
	static Real cot_r(Real);
	static Real exp_r(Real);
	static Real log_r(Real);
	static Real sqr_r(Real);
	static Real tan_r(Real);
	
	static PGM_P const funcStrings[NUM_FUNC];
};

}

#endif
