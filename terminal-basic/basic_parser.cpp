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

#include <math.h>
#include <string.h>
#include <signal.h>

#include "basic_parser.hpp"
#include "basic_interpreter.hpp"
#include "basic_interpreter_program.hpp"

#ifdef ARDUINO
#include "config_arduino.hpp"
#else
#include "config_linux.hpp"
#endif

/*
 * TEXT = OPERATORS | C_INTEGER OPERATORS
 * OPERATORS = OPERATOR | OPERATOR COLON OPERATORS
 * OPERATOR =
 *	KW_DIM ARRAYS_LIST |
 *      KW_END |
 *	KW_FOR FOR_CONDS |
 *	KW_GOSUB EXPRESSION |
 *	KW_IF EXPRESSION IF_STATEMENT |
 *	KW_INPUT VAR_LIST |
 *	KW_LET IMPLICIT_ASSIGNMENT |
 *	KW_NEXT IDENT |
 *	KW_PRINT | KW_PRINT PRINT_LIST |
 *	KW_REM TEXT |
 *	KW_RETURN |
 *	KW_RANDOMIZE |
 *	GOTO_STATEMENT |
 *	COMMAND
 * COMMAND = COM_DUMP | COM_DUMP KW_VARS | COM_DUMP KW_ARRAYS
 *	COM_LIST | COM_NEW | COM_RUN | COM_SAVE | COM_LOAD
 * ASSIGNMENT = KW_LET IMPLICIT_ASSIGNMENT | IMPLICIT_ASSIGNMENT
 * IMPLICIT_ASSIGNMENT = VAR EQUALS EXPRESSION | VAR ARRAY EQUALS EXPRESSION
 * EXPRESSION = SIMPLE_EXPRESSION | SIMPLE_EXPRESSION REL SIMPLE_EXPRESSION
 * REL = LT | LTE | EQUALS | GT | GTE | NE | NEA
 * SIMPLE_EXPRESSION = TERM | TERM ADD TERM
 * ADD = PLUS MINUS KW_OR
 * TERM = FACTOR | FACTOR MUL FACTOR
 * MUL = STAR | SLASH | DIV | MOD | KW_AND
 * FACTOR = FINAL | FINAL POW FINAL
 * FINAL = C_INTEGER | C_REAL | C_STRING | VAR | VAR ARRAY |
 *	LPAREN EXPRESSION RPAREN | MINUS FINAL
 * VAR = REAL_IDENT | INTEGER_IDENT | STRING_IDENT
 * VAR_LIST = VAR | VAR VAR_LIST
 * PRINT_LIST = EXPRESSION | EXPRESSION COMMA PRINT_LIST
 * IF_STATEMENT = GOTO_STATEMEMNT | KW_THEN OPERATORS
 * GOTO_STATEMENT = KW_GOTO C_INTEGER
 * FOR_CONDS = IMPLICIT_ASSIGNMENT KW_TO EXPRESSION |
 *	IMPLICIT_ASSIGNMENT KW_TO EXPRESSION KW_STEP EXPRESSION
 * ARRAYS_LIST = VAR ARRAY | VAR ARRAY ARRAYS_LIST
 * ARRAY = LPAREN DIMENSIONS RPAREN
 * DIMENSIONS = C_INTEGER | C_INTEGER COMMA DIMENSIONS
 */

namespace BASIC
{

Parser::Parser(Lexer &l, Interpreter &i, FunctionBlock *first) :
_lexer(l), _interpreter(i), _mode(EXECUTE), _internal(first)
{
}

void Parser::init()
{
	_internal.init();
}

bool
Parser::parse(const char *s)
{
	LOG_TRACE;

	_lexer.init(s);
	_stopParse = false;
	_error = NO_ERROR;
	
	if (_lexer.getNext())
		return fOperators();
	else
		return true;
}

/*
 * OPERATORS = OPERATOR | OPERATOR COLON OPERATORS
 */
bool
Parser::fOperators()
{
	Token t;
	do {
		if (!fOperator()) {
			if (_error == NO_ERROR)
				_error = OPERATOR_EXPECTED;
			return false;
		}
		if (_stopParse)
			break;
		t = _lexer.getToken();
		if (t == Token::COLON) {
			if (_lexer.getNext())
				continue;
			else
				break;
		} else if (t == Token::NOTOKENS)
			break;
		else
			return false;
	} while (true);
	return true;
}

/*
 * OPERATOR =
 *	KW_DIM ARRAYS_LIST |
 *      KW_END |
 *	KW_FOR FOR_CONDS |
 *	KW_GOSUB EXPRESSION |
 *	KW_IF EXPRESSION IF_STATEMENT |
 *	KW_INPUT VAR_LIST |
 *	KW_LET IMPLICIT_ASSIGNMENT |
 *	KW_NEXT IDENT |
 *	KW_PRINT | KW_PRINT PRINT_LIST |
 *	KW_REM TEXT |
 *	KW_RETURN |
 *	KW_RANDOMIZE |
 *	GOTO_STATEMENT |
 *	COMMAND
 */
bool
Parser::fOperator()
{
	LOG_TRACE;

	Token t = _lexer.getToken();
	LOG(t);
	switch (t) {
	case Token::KW_DIM:
		if (!_lexer.getNext())
			return (false);
		return (fArrayList());
	case Token::KW_END:
		if (_mode == EXECUTE)
			_interpreter.end();
		_stopParse = true;
		_lexer.getNext();
		break;
	case Token::KW_FOR:
		if (!_lexer.getNext())
			return (false);
		return (fForConds());
	case Token::KW_GOSUB:
	{
		Value v;
		if (!_lexer.getNext() || !fExpression(v)) {
			_error = EXPRESSION_EXPECTED;
			return (false);
		}
		if (_mode == EXECUTE) {
			_interpreter.pushReturnAddress(_lexer.getPointer());
			_interpreter.gotoLine(v);
		}
		_stopParse = true;
		break;
	}
	case Token::KW_IF:
	{
		Value v;
		if (!_lexer.getNext() || !fExpression(v)) {
			_error = EXPRESSION_EXPECTED;
			return (false);
		}
		bool res;
		if (!bool(v))
			_mode = SCAN;
		if (fIfStatement())
			res = true;
		else {
			_error = THEN_OR_GOTO_EXPECTED;
			res = false;
		}
		_mode = EXECUTE;
		return (res);
	}
	case Token::KW_INPUT:
	{
		if (!fVarList()) {
			_error = VARIABLES_LIST_EXPECTED;
			return (false);
		}
		break;
	}
	case Token::KW_LET:
	{
		char vName[VARSIZE];
		if (!_lexer.getNext() || !fImplicitAssignment(vName))
			return (false);
		break;
	}
	case Token::KW_NEXT:
		if (!_lexer.getNext() || (_lexer.getToken() != Token::REAL_IDENT
		    && _lexer.getToken() != Token::INTEGER_IDENT))
			return (false);
		if (_mode == EXECUTE) {
			_stopParse = !_interpreter.next(_lexer.id());
		}
		if (!_stopParse)
			_lexer.getNext();
		break;
	case Token::KW_PRINT:
		if (_lexer.getNext())
			if (!fPrintList())
				return false;
		if (_mode == EXECUTE) {
			_interpreter.print('\r');
                        _interpreter.print('\n');
                }
		break;
	case Token::KW_RANDOMIZE:
		if (_mode == EXECUTE)
			_interpreter.randomize();
		_lexer.getNext();
		break;
	case Token::KW_REM:
		while (_lexer.getNext());
		break;
	case Token::KW_RETURN:
		if (_mode == EXECUTE) {
			_interpreter.returnFromSub();
			_stopParse = true;
		}
		_lexer.getNext();
		break;
	default:
		if (fCommand() || fGotoStatement())
			break;
		{
			char vName[VARSIZE];
			if (fImplicitAssignment(vName))
				break;
		}
		_error = OPERATOR_EXPECTED;
		return (false);
	}
	return (true);
}

bool
Parser::fImplicitAssignment(char *varName)
{
	LOG_TRACE;

	if (fVar(varName) && _lexer.getNext()) {
		uint8_t dimensions;
		Value v;
		bool array;
		if (_lexer.getToken() == Token::LPAREN) {
			if (fArray(dimensions))
				array = true;
			else
				return false;
		} else
			array = false;
		if ((_lexer.getToken() == Token::EQUALS) && _lexer.getNext() &&
			fExpression(v)) {
			if (_mode == EXECUTE) {
				if (array)
					_interpreter.setArrayElement(varName, v);
				else
					_interpreter.setVariable(varName, v);
			}
			return true;
		} else {
			_error = EXPRESSION_EXPECTED;
		}
	}
	return false;
}

bool
Parser::fPrintList()
{
	LOG_TRACE;

	Value v;
	if (!fExpression(v)) {
		_error = EXPRESSION_EXPECTED;
		return false;
	}

	if (_mode == EXECUTE)
		_interpreter.print(v);
	while (true) {
		Token t = _lexer.getToken();
		switch (t) {
		case Token::COMMA:
			if (!_lexer.getNext() || !fExpression(v)) {
				_error = EXPRESSION_EXPECTED;
				return false;
			}
			if (_mode == EXECUTE) {
				_interpreter.print('\t');
				_interpreter.print(v);
			}
			break;
		default:
			return true;
		}
	}
}

/*
 * EXPRESSION =
 *	SIMPLE_EXPRESSION |
 *	OP_NOT SIMPLE_EXPRESSION |
 *	SIMPLE_EXPRESSION REL SIMPLE_EXPRESSION
 */
bool
Parser::fExpression(Value &v)
{
	LOG_TRACE;
	
	Token t = _lexer.getToken();
	if(t == Token::OP_NOT) {
		if (!_lexer.getNext() || !fSimpleExpression(v))
			return (false);
		if (_mode == EXECUTE)
			v = !bool(v);
		return (true);
	}
	
	if (!fSimpleExpression(v))
		return false;

	while (true) {
		t = _lexer.getToken();
		Value v2;
		switch (t) {
		case Token::LT:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v.value.boolean = v < v2;
				v.type = Value::BOOLEAN;
				continue;
			} else
				return false;
		case Token::LTE:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v.value.boolean = (v < v2) || (v == v2);
				v.type = Value::BOOLEAN;
				continue;
			} else
				return false;
		case Token::GT:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v.value.boolean = v > v2;
				v.type = Value::BOOLEAN;
				continue;
			} else
				return false;
		case Token::GTE:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v.value.boolean = (v > v2) || (v == v2);
				v.type = Value::BOOLEAN;
				continue;
			} else
				return false;
		case Token::EQUALS:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v.value.boolean = v == v2;
				v.type = Value::BOOLEAN;
				continue;
			} else
				return false;
		case Token::NE:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v.value.boolean = !(v == v2);
				v.type = Value::BOOLEAN;
				continue;
			} else
				return false;
		default:
			return true;
		}
		v.type = Value::BOOLEAN;
	}
}

bool
Parser::fSimpleExpression(Value &v)
{
	LOG_TRACE;

	if (!fTerm(v))
		return false;

	while (true) {
		Token t = _lexer.getToken();
		LOG(t);
		Value v2;
		switch (t) {
		case Token::PLUS:
			if (_lexer.getNext() && fTerm(v2)) {
				if (v.type == Value::STRING &&
				    v2.type == Value::STRING)
					_interpreter.strConcat(v,v2);
				v += v2;
				continue;
			} else
				return false;
		case Token::MINUS:
			if (_lexer.getNext() && fTerm(v2)) {
				v -= v2;
				continue;
			} else
				return false;
		default:
			return true;
		}
	}
}

bool
Parser::fTerm(Value &v)
{
	LOG_TRACE;

	if (!fFactor(v))
		return false;

	while (true) {
		Token t = _lexer.getToken();
		LOG(t);
		Value v2;
		switch (t) {
		case Token::STAR:
			if (_lexer.getNext() && fTerm(v2)) {
				v *= v2;
				continue;
			} else
				return false;
		case Token::SLASH:
			if (_lexer.getNext() && fTerm(v2)) {
				v /= v2;
				continue;
			} else
				return false;
		default:
			return true;
		}
	}
}

bool
Parser::fFactor(Value &v)
{
	LOG_TRACE;

	if (!fFinal(v))
		return false;

	while (true) {
		Token t = _lexer.getToken();
		LOG(t);
		Value v2;
		switch (t) {
		case Token::POW:
			if (_lexer.getNext() && fFactor(v2)) {
				v ^= v2;
				continue;
			} else
				return false;
		default:
			return true;
		}
	}
}

bool
Parser::fFinal(Value &v)
{
	LOG_TRACE;

	Token t = _lexer.getToken();
	LOG(t);
	while (true) {
		switch (t) {
		case Token::MINUS:
			if (!_lexer.getNext() || !fFinal(v))
				return false;
			if (_mode == EXECUTE)
				v.switchSign();
			return true;
		case Token::C_INTEGER:
		case Token::C_REAL:
			if (_mode == EXECUTE)
				v = _lexer.getValue();
			_lexer.getNext();
			return true;
		case Token::C_STRING:
			if (_mode == EXECUTE) {
				_interpreter.pushString(_lexer.id());
				v.type = Value::Type::STRING;
			}
			_lexer.getNext();
			return true;
		case Token::LPAREN:
			if (!_lexer.getNext() || !fExpression(v))
				return false;
			if (_lexer.getToken() != Token::RPAREN)
				return false;
			else {
				_lexer.getNext();
				return true;
			}
		case Token::KW_TRUE:
			if (_mode == EXECUTE)
				v = true;
			_lexer.getNext();
			return true;
		case Token::KW_FALSE:
			if (_mode == EXECUTE)
				v = false;
			_lexer.getNext();
			return true;
		default:
		{
			char varName[VARSIZE];
			if (fVar(varName))
				return fIdentifierExpr(varName, v);
		}
			return false;
		}
	}
}

bool
Parser::fIfStatement()
{
	Token t = _lexer.getToken();
	LOG(t);
	switch (t) {
	case Token::KW_THEN:
		if (_lexer.getNext()) {
			if (_lexer.getToken() == Token::C_INTEGER) {
				if (_mode == EXECUTE)
					_interpreter.gotoLine(_lexer.getValue());
				_lexer.getNext();
				return true;
			} else if (fOperators())
				return true;
		}
		break;
	default:
		if (fGotoStatement())
			return true;
		break;
	}
	return false;
}

bool
Parser::fGotoStatement()
{
	Token t = _lexer.getToken();
	LOG(t);
	if (t == Token::KW_GOTO) {
		Value v;
		if (!_lexer.getNext() || !fExpression(v)) {
			_error = EXPRESSION_EXPECTED;
			return false;
		}
		if (_mode == EXECUTE) {
			_interpreter.gotoLine(v.value.integer);
			_stopParse = true;
		}
		return true;
	} else
		return false;
}

bool
Parser::fCommand()
{
	Token t = _lexer.getToken();
	LOG(t);
	switch (t) {
	case Token::COM_CLS:
		if (_mode == EXECUTE)
			_interpreter.cls();
		_lexer.getNext();
		return true;
	case Token::COM_DUMP:
	{
		Interpreter::DumpMode mode = Interpreter::MEMORY;
		if (_lexer.getNext()) {
			if (_lexer.getToken() == Token::KW_VARS) {
				mode = Interpreter::VARS;
				_lexer.getNext();
			} else if (_lexer.getToken() == Token::KW_ARRAYS) {
				mode = Interpreter::ARRAYS;
				_lexer.getNext();
			}
		}
		if (_mode == EXECUTE)
			_interpreter.dump(mode);
		return true;
	}
	case Token::COM_LIST:
	{
		Integer start = 1, stop = 0;
		_lexer.getNext();
		if (_lexer.getToken() == Token::C_INTEGER) {
			start = Integer(_lexer.getValue());
			stop = start;
			_lexer.getNext();
		}
		if (_lexer.getToken() == Token::MINUS) {
			if (!_lexer.getNext() || _lexer.getToken() !=
			    Token::C_INTEGER) {
				_error = INTEGER_CONSTANT_EXPECTED;
				return false;
			}
			stop = Integer(_lexer.getValue());
			_lexer.getNext();
		}
		if (_mode == EXECUTE)
			_interpreter.list(start, stop);
	}
		return (true);
	case Token::COM_LOAD:
		if (_mode == EXECUTE)
			_interpreter.load();
		_lexer.getNext();
		return (true);
	case Token::COM_NEW:
		if (_mode == EXECUTE)
			_interpreter.newProgram();
		_lexer.getNext();
		return (true);
	case Token::COM_RUN:
		if (_mode == EXECUTE)
			_interpreter.run();
		_lexer.getNext();
		return (true);
	case Token::COM_SAVE:
		if (_mode == EXECUTE)
			_interpreter.save();
		_lexer.getNext();
		return (true);
	case Token::REAL_IDENT:
	case Token::INTEGER_IDENT:
		FunctionBlock::command c;
		if ((c=_internal.getCommand(_lexer.id())) != NULL) {
			while (_lexer.getNext()) {
				Value v;
				// String value already on stack after fExpression
				if (fExpression(v)) {
					if (v.type != Value::STRING)
						_interpreter.pushValue(v);
				} else
					break;
				
				if (_lexer.getToken() == Token::COMMA)
					continue;
				else
					break;
			}
			return (*c)(_interpreter);
		}
	default:
		return (false);
	}
}

bool
Parser::fForConds()
{
	Value v;
	char vName[VARSIZE];
	if (!fImplicitAssignment(vName) ||
	    _lexer.getToken()!=Token::KW_TO || !_lexer.getNext() ||
	    !fExpression(v)) {
		return (false);
	}
	Value vStep(Integer(1));
	if (_lexer.getToken() == Token::KW_STEP && (!_lexer.getNext() ||
	    !fExpression(vStep)))
		return (false);
	
	if (_mode == EXECUTE)
		_interpreter.pushForLoop(vName, _lexer.getPointer(), v, vStep);
	
	return (true);
}

bool
Parser::fVarList()
{
	Token t;
	char varName[VARSIZE];
	do {
		if (!_lexer.getNext() || !fVar(varName))
			return (false);
		if (_mode == EXECUTE) {
			_interpreter.input(varName);
		} if (!_lexer.getNext())
			return (true);
		t = _lexer.getToken();
	} while (t == Token::COMMA);
	return (true);
}

bool
Parser::fVar(char *varName)
{
	if ((_lexer.getToken() >= Token::REAL_IDENT) &&
	    (_lexer.getToken() <= Token::BOOL_IDENT)) {
		strcpy(varName, _lexer.id());
		return (true);
	} else
		return (false);
}

bool
Parser::fArrayList()
{
	Token t;
	char arrName[VARSIZE];
	uint8_t dimensions;
	do {
		if (!fVar(arrName) ||
		    !_lexer.getNext() || !fArray(dimensions))
			return (false);
		_interpreter.pushDimensions(dimensions);
		_interpreter.newArray(arrName);
		t = _lexer.getToken();
		if (t != Token::COMMA)
			return (true);
		else
			if (!_lexer.getNext())
				return (false);
	} while (true);
}

bool
Parser::fArray(uint8_t &dimensions)
{
	if (_lexer.getToken() != Token::LPAREN ||
	    !fDimensions(dimensions) || _lexer.getToken() != Token::RPAREN)
		return (false);

	_lexer.getNext();
	return (true);
}

bool
Parser::fDimensions(uint8_t &dimensions)
{
	Parser::Value v;
	dimensions = 0;
	do {
		if (!_lexer.getNext() || !fExpression(v))
			return (false);
		_interpreter.pushDimension(Integer(v));
		++dimensions;
	} while (_lexer.getToken() == Token::COMMA);
	return (true);
}

bool
Parser::fIdentifierExpr(const char *varName, Value &v)
{
	 // Identifier, var or func or array ?
	if (_lexer.getNext() && _lexer.getToken()==
	    Token::LPAREN) { // (, array or function
		FunctionBlock::function f;
		if ((f=_internal.getFunction(varName)) != NULL) {
			// function
			Value arg;
			do {
				if (!_lexer.getNext())
					return (false);
				else if (_lexer.getToken() == Token::RPAREN) {
					break;
				} else {
					if (!fExpression(arg))
						return (false);
					_interpreter.pushValue(arg);
				}
			} while (_lexer.getToken() == Token::COMMA);
			_lexer.getNext();
			bool result = true;
			if (_mode == EXECUTE) {
				result = ((*f)(_interpreter));
				if (!result || !_interpreter.popValue(v))
					return (false);
			}
		} else { // No such function, array variable
			uint8_t dim;
			if (fArray(dim)) {
				if (_mode == EXECUTE &&
				    _interpreter.valueFromArray(v, varName))
					return (true);
			} else
				return (false);
		}
	} else
		if (_mode == EXECUTE)
			_interpreter.valueFromVar(v, varName);
	return (true);
}

}
