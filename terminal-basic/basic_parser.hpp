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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <stdlib.h>
#include <inttypes.h>

#include "arduinoext.hpp"
#include "basic.hpp"
#include "basic_internalfuncs.hpp"

namespace BASIC
{

class Lexer;
class Interpreter;

/**
 * @brief Syntactic analyzer class
 */
class Parser
{
public:
	/**
	 * @brief Static semantic errors
	 */
	enum ErrorCodes : uint8_t
	{
		NO_ERROR = 0,
		OPERATOR_EXPECTED = 1,
		EXPRESSION_EXPECTED = 2,
		INTEGER_CONSTANT_EXPECTED = 3,
		THEN_OR_GOTO_EXPECTED = 4,
		INVALID_DATA_EXPR = 5,
		INVALID_READ_EXPR = 6,
		VARIABLES_LIST_EXPECTED = 7,
		STRING_OVERFLOW = 8
	};
	
	class EXT_PACKED Value;
	/**
	 * @brief constructor
	 * @param lexer Lexical analyzer object refertence
	 * @param interpreter Interpreter context object reference
	 * @param module Pointer to the first module in chain
	 */
	Parser(Lexer&, Interpreter&);
	/**
	 * @brief Parse a text string
	 * @param str string to parse
	 * @param ok successfull parsing flag
	 * @return end of parsed string
	 */
	bool parse(const char*, bool&);
	
	void stop();
	/**
	 * @brief get last static error code
	 * @return error code
	 */
	ErrorCodes getError() const { return _error; }
	
	void init();
	
	void addModule(FunctionBlock*);
private:
	/**
	 * Parser mode: syntax check or execute commands of the interpreter
	 * context
	 */
	enum Mode : uint8_t
	{
		SCAN = 0
		, EXECUTE
	};
	bool fOperators(bool&);
	bool fOperator();
#if USE_DATA
	bool fDataStatement();
	bool fReadStatement();
#endif // USE_DATA
	bool fImplicitAssignment(char*);
	bool fPrintList();
	bool fPrintItem();
	bool fExpression(Value&);
	bool fSimpleExpression(Value&);
	bool fTerm(Value&);
	bool fFactor(Value&);
	bool fFinal(Value&);
	bool fIfStatement();
	bool fCommand();
	bool fGotoStatement();
	bool fForConds();
	bool fIdentifier(char*);
	bool fVarList();
	bool fArrayList();
	bool fArray(uint8_t&);
	bool fDimensions(uint8_t&);
	bool fIdentifierExpr(char*, Value&);
#if USE_MATRIX
	bool fMatrixOperation();
	bool fMatrixPrint();
	bool fMatrixExpression(const char*);
#endif
	
	// last static semantic error
	ErrorCodes _error;
	// lexical analyser object reference
	Lexer &_lexer;
	// interpreter context object reference
	Interpreter &_interpreter;
	// current mode
	Mode	_mode;
	// stop parsing string flag
	bool	_stopParse;
	// first module in chain reference
	InternalFunctions _internal;
};

} // namespace BASIC

#endif
