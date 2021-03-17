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

#include <math.h>
#include <string.h>
#include <signal.h>

#include "basic_parser.hpp"
#include "basic_interpreter.hpp"
#include "basic_program.hpp"
#include "ascii.hpp"

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
 *	COMMAND |
 *	KW_MAT MATRIX_OPERATION
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
 * MATRIX_OPERATION =
 *      PRINT MATRIX_PRINT |
 *	DET VAR
 *      VAR EQUALS MATRIX_EXPRESSION
 */

namespace BASIC
{

Parser::Parser(Lexer &l, Interpreter &i) :
_lexer(l), _interpreter(i), _mode(EXECUTE)
{
}

void Parser::init()
{
	_mode = EXECUTE;
	_internal.init();
}

void Parser::addModule(FunctionBlock *module)
{
	_internal.setNext(module);
}

void
Parser::stop()
{
	_stopParse = true;
}

bool
Parser::parse(const char *s, bool &ok)
{
	LOG_TRACE;

	_lexer.init(s);
	_stopParse = false;
	_error = NO_ERROR;
	
	if (_lexer.getNext()) {
		return fOperators(ok);
	} else {
		ok = true;
		return false;
	}
}

/*
 * OPERATORS = OPERATOR | OPERATOR COLON OPERATORS
 */
bool
Parser::fOperators(bool &ok)
{
	Token t;
	if (!fOperator()) {
		if (_error == NO_ERROR)
			_error = OPERATOR_EXPECTED;
		ok = false;
		return true;
	}
	if (_stopParse) {
		ok = true;
		return false;
	}
	t = _lexer.getToken();
	if (t == Token::COLON) {
		ok = true;
		return true;
	} else if (t == Token::NOTOKENS) {
		ok = true;
		return false;
	} else {
		ok = false;
		return true;
	}
}

/*
 * OPERATOR =
 *	KW_DIM ARRAYS_LIST |
 *      KW_END |
 *	KW_STOP |
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
 *	COMMAND |
 *      KW_MAT MATRIX_OPERATION
 */
bool
Parser::fOperator()
{
	LOG_TRACE;

	const Token t = _lexer.getToken();
	LOG(t);
	switch (t) {
	case Token::KW_DIM:
		if (_lexer.getNext())
			return fArrayList();
		return false;
	case Token::KW_END:
		_interpreter._program.reset();
#if USESTOPCONT
	case Token::KW_STOP:
#endif
		if (_mode == EXECUTE)
			_interpreter.end();
		_stopParse = true;
		_lexer.getNext();
		break;
	case Token::KW_FOR:
		if (_lexer.getNext())
			return fForConds();
		return false;
	case Token::KW_GOSUB: {
		Value v;
		if (!_lexer.getNext() || !fExpression(v)) {
			_error = EXPRESSION_EXPECTED;
			return false;
		}
		if (_mode == EXECUTE) {
			_interpreter.pushReturnAddress(_lexer.getPointer());
			_interpreter.gotoLine(v);
		}
		_stopParse = true;
		break;
	}
	case Token::KW_IF: {
		Value v;
		if (!_lexer.getNext() || !fExpression(v)) {
			_error = EXPRESSION_EXPECTED;
			return false;
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
		return res;
	}
	case Token::KW_INPUT:
		if (!fVarList()) {
			_error = VARIABLES_LIST_EXPECTED;
			return false;
		} else if (_mode == EXECUTE)
			_interpreter.input();
		break;
	case Token::KW_LET: {
		char vName[VARSIZE];
		if (!_lexer.getNext() || !fImplicitAssignment(vName))
			return false;
	}
		break;
#if USE_MATRIX
	case Token::KW_MAT:
		if (!_lexer.getNext() || !fMatrixOperation())
			return false;
		break;
#endif
	case Token::KW_NEXT: {
		char vName[VARSIZE];
		if (!_lexer.getNext() || !fVar(vName))
			return false;
		if (_mode == EXECUTE)
			_stopParse = !_interpreter.next(_lexer.id());
		if (!_stopParse)
			_lexer.getNext();
	}
		break;
	case Token::KW_PRINT:
		if (_lexer.getNext()) {
			if (!fPrintList())
				return false;
		} else
			_interpreter.newline();
		break;
#if USE_RANDOM
	case Token::KW_RANDOMIZE:
		if (_mode == EXECUTE)
			_interpreter.randomize();
		_lexer.getNext();
		break;
#endif
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
		return false;
	}
//	if (t == Token::KW_DIM) {
//		if (_lexer.getNext())
//			return fArrayList();
//		return false;
//	} else if (t == Token::KW_END) {
//		if (_mode == EXECUTE) {
//			_interpreter._program.reset();
//			_interpreter.end();
//		}
//		_stopParse = true;
//		_lexer.getNext();
//	} else if (t == Token::KW_STOP) {
//		if (_mode == EXECUTE)
//			_interpreter.end();
//		_stopParse = true;
//		_lexer.getNext();
//	} else if (t == Token::KW_FOR) {
//		if (_lexer.getNext())
//			return fForConds();
//		return false;
//	} else if (t == Token::KW_GOSUB) {
//		Value v;
//		if (!_lexer.getNext() || !fExpression(v)) {
//			_error = EXPRESSION_EXPECTED;
//			return false;
//		}
//		if (_mode == EXECUTE) {
//			_interpreter.pushReturnAddress(_lexer.getPointer());
//			_interpreter.gotoLine(v);
//		}
//		_stopParse = true;
//	} else if (t == Token::KW_IF) {
//		Value v;
//		if (!_lexer.getNext() || !fExpression(v)) {
//			_error = EXPRESSION_EXPECTED;
//			return false;
//		}
//		bool res;
//		if (!bool(v))
//			_mode = SCAN;
//		if (fIfStatement())
//			res = true;
//		else {
//			_error = THEN_OR_GOTO_EXPECTED;
//			res = false;
//		}
//		_mode = EXECUTE;
//		return res;
//	} else if (t == Token::KW_INPUT) {
//		if (!fVarList()) {
//			_error = VARIABLES_LIST_EXPECTED;
//			return false;
//		} else if (_mode == EXECUTE)
//			_interpreter.input();
//	} else if (t == Token::KW_LET) {
//		char vName[VARSIZE];
//		if (!_lexer.getNext() || !fImplicitAssignment(vName))
//			return false;
//	}
//#if USE_MATRIX
//	 else if (t == Token::KW_MAT)
//		return _lexer.getNext() && fMatrixOperation();
//#endif
//	else if (t == Token::KW_NEXT) {
//		char vName[VARSIZE];
//		if (!_lexer.getNext() || !fVar(vName))
//			return false;
//		if (_mode == EXECUTE)
//			_stopParse = !_interpreter.next(_lexer.id());
//		if (!_stopParse)
//			_lexer.getNext();
//	} else if (t == Token::KW_PRINT) {
//		if (_lexer.getNext())
//			return fPrintList();
//		else
//			_interpreter.newline();
//	}
//#if USE_RANDOM
//	else if (t == Token::KW_RANDOMIZE) {
//		if (_mode == EXECUTE)
//			_interpreter.randomize();
//		_lexer.getNext();
//	}
//#endif
//	else if (t == Token::KW_REM)
//		while (_lexer.getNext());
//	else if (t == Token::KW_RETURN) {
//		if (_mode == EXECUTE) {
//			_interpreter.returnFromSub();
//			_stopParse = true;
//		}
//		_lexer.getNext();
//	} else {
//		if (fGotoStatement() || fCommand())
//			return true;
//		{
//			char vName[VARSIZE];
//			if (fImplicitAssignment(vName))
//				return true;
//		}
//		_error = OPERATOR_EXPECTED;
//		return false;
//	}
	return true;
}

/*
 * IMPLICIT_ASSIGNMENT =
 * VAR EQUALS EXPRESSION |
 * VAR ARRAY EQUALS EXPRESSION
 */
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

/*
 * PRINT_LIST =
 * PRINT_ITEM |
 * PRINT_ITEM COMMA PRINT_LIST |
 * PRINT_ITEM SEMI PRINT_LIST
 */
bool
Parser::fPrintList()
{
	LOG_TRACE;
	
	if (!fPrintItem()) 
		return false;
	
	while (true) {
		const Token t = _lexer.getToken();
		switch (t) {
		case Token::COMMA:
			if (_mode == EXECUTE)
				_interpreter.print(char(ASCII::HT));
			if (!_lexer.getNext() || !fPrintItem())
				return false;
			break;
		case Token::SEMI:
			if (_lexer.getNext() && _lexer.getToken() != Token::COLON) {
				if (!fPrintItem())
					return false;
			} else
				return true;
			break;
		default:
			if (_mode == EXECUTE)
				_interpreter.newline();
			return true;
		}
	}
}

/*
 * PRINT_ITEM =
 * KW_TAB LPAREN EXPRESSION RPAREN |
 * EXPRESSION
 */
bool
Parser::fPrintItem()
{
	const Token t = _lexer.getToken();
	if (t != Token::COMMA && t != Token::COLON) { // printable tokens
		Value v;
		if (t == Token::KW_TAB) {
			if (_lexer.getNext() && _lexer.getToken() == Token::LPAREN &&
			    _lexer.getNext() && fExpression(v) &&
			    _lexer.getToken() == Token::RPAREN) {
				if (_mode == EXECUTE)
					_interpreter.printTab(v);
				_lexer.getNext();
			} else
				return false;
		} else {
			if (!fExpression(v)) {
				if (_error == NO_ERROR)
					_error = EXPRESSION_EXPECTED;
				return false;
			}

			if (_mode == EXECUTE)
				_interpreter.print(v);
		}
	}
	return true;
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
	
	if (!fSimpleExpression(v))
		return false;

	while (true) {
		const Token t = _lexer.getToken();
		Value v2;
#if OPT == OPT_SPEED
		switch (t) {
		case Token::LT:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v = v < v2;
				continue;
			} else
				return false;
		case Token::LTE:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v = (v < v2) || (v == v2);
				continue;
			} else
				return false;
		case Token::GT:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v = v > v2;
				continue;
			} else
				return false;
		case Token::GTE:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v = (v > v2) || (v == v2);
				continue;
			} else
				return false;
		case Token::EQUALS:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
#if USE_STRINGOPS
				if (v.type == Value::STRING &&
				    v2.type == Value::STRING)
					v = _interpreter.strCmp();
				else
#endif
					v = v == v2;
				continue;
			} else
				return false;
		case Token::NE:
		case Token::NEA:
			if (_lexer.getNext() && fSimpleExpression(v2)) {
				v = !(v == v2);
				continue;
			} else
				return false;
		default:
			return true;
		}
#else
		if (t == Token::LT || t == Token::LTE || t == Token::GT ||
		    t == Token::GTE || t == Token::EQUALS || t == Token::NE ||
		    t == Token::NEA) {
			if (!_lexer.getNext() || !fSimpleExpression(v2))
				return false;
			
			if (t == Token::LT)
				v = v < v2;
			else if (t == Token::LTE)
				v = (v < v2) || (v == v2);
			else if (t == Token::GT)
				v = v > v2;
			else if (t ==Token::GTE)
				v = (v > v2) || (v == v2);
			else if (t == Token::EQUALS) {
#if USE_STRINGOPS
				if (v.type == Value::STRING &&
				    v2.type == Value::STRING)
					v = _interpreter.strCmp();
				else
#endif // USE_STRINGOPS
					v = v == v2;
			} else if (t == Token::NE || t == Token::NEA)
				v = !(v == v2);
		} else
			return true;
#endif
		v.type = Value::BOOLEAN;
	}
}

/*
 * SIMPLE_EXPRESSION =
 *	TERM |
 *	TERM ADD TERM
 */
bool
Parser::fSimpleExpression(Value &v)
{
	LOG_TRACE;

	if (!fTerm(v))
		return false;

	while (true) {
		const Token t = _lexer.getToken();
		LOG(t);
		Value v2;
#if OPT == OPT_SPEED
		switch (t) {
		case Token::PLUS:
			if (_lexer.getNext() && fTerm(v2)) {
#if USE_STRINGOPS
				if (v.type == Value::STRING &&
				    v2.type == Value::STRING)
					_interpreter.strConcat();
				else
#endif // USE_STRINGOPS
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
		case Token::OP_OR:
			if (_lexer.getNext() && fTerm(v2)) {
				v |= v2;
				continue;
			} else
				return false;
		default:
			return true;
		}
#else
		if (t == Token::PLUS || t == Token::MINUS || t == Token::OP_OR) {
			if (!_lexer.getNext() || !fTerm(v2))
				return false;
			if (t == Token::PLUS) {
#if USE_STRINGOPS
				if (v.type == Value::STRING &&
				    v2.type == Value::STRING)
					_interpreter.strConcat();
				else
#endif // USE_STRINGOPS
					v += v2;
			} else if (t == Token::MINUS)
				v -= v2;
			else if (t == Token::OP_OR)
				v |= v2;
		} else
			return true;
#endif // OPT == OPT_SPEED
	}
}

bool
Parser::fTerm(Value &v)
{
	LOG_TRACE;

	if (!fFactor(v))
		return false;

	while (true) {
		const Token t = _lexer.getToken();
		LOG(t);
		Value v2;
#if OPT == OPT_SPEED
		switch (t) {
		case Token::STAR:
			if (_lexer.getNext() && fFactor(v2)) {
				v *= v2;
				continue;
			} else
				return false;
		case Token::SLASH:
			if (_lexer.getNext() && fFactor(v2)) {
				v /= v2;
				continue;
			} else
				return false;
#if USE_REALS
			if (_lexer.getNext() && fFactor(v2)) {
				v = INT(v /= v2);
				continue;
			} else
				return false;
#endif
		case Token::OP_AND:
			if (_lexer.getNext() && fFactor(v2)) {
				v &= v2;
				continue;
			} else
				return false;	
		default:
			return true;
		}
#else
		if (t == Token::STAR || t == Token::SLASH || t == Token::OP_AND
#if USE_REALS
		 || t == Token::BACK_SLASH
#endif
		   ) {
			if (!_lexer.getNext() || !fFactor(v2))
				return false;
			
			if (t == Token::STAR)
				v *= v2;
			else if (t == Token::SLASH)
				v /= v2;
			else if (t == Token::OP_AND)
				v &= v2;
#if USE_REALS
			else if (t == Token::BACK_SLASH) {
				v = INT(v /= v2);
			}
#endif
		} else
			return true;
#endif
	}
}

bool
Parser::fFactor(Value &v)
{
	LOG_TRACE;

	if (!fFinal(v))
		return false;

	while (true) {
		const Token t = _lexer.getToken();
		LOG(t);
		Value v2;
		if (t == Token::POW) {
			if (_lexer.getNext() && fFinal(v2)) {
				v ^= v2;
			} else
				return false;
		} else
			return true;
	}
}

bool
Parser::fFinal(Value &v)
{
	LOG_TRACE;

	const Token t = _lexer.getToken();
	LOG(t);

	while (true) {
#if OPT == OPT_SPEED
		switch (t) {
		case Token::PLUS:
			return _lexer.getNext() && fFinal(v);
		case Token::MINUS:
			if (!_lexer.getNext() || !fFinal(v))
				return false;
			if (_mode == EXECUTE)
				v.switchSign();
			return true;
		case Token::OP_NOT:
			if (!_lexer.getNext() || !fFinal(v))
				return false;
			if (_mode == EXECUTE)
				v.notOp();
			return true;
		case Token::C_INTEGER:
		case Token::C_REAL:
		case Token::C_BOOLEAN:
			if (_mode == EXECUTE)
				v = _lexer.getValue();
			_lexer.getNext();
			return true;
		case Token::C_STRING:
			if (_lexer.getError() == Lexer::STRING_OVERFLOW) {
				_error = STRING_OVERFLOW;
				_lexer.getNext();
				return false;
			}
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
		default:
		{
			char varName[VARSIZE];
			if (fVar(varName))
				return fIdentifierExpr(varName, v);
		}
			return false;
		}
#else
		if (t == Token::PLUS) { // Unary plus, ignored
			return _lexer.getNext() && fFinal(v);
		} else if (t == Token::MINUS) { // Unary minus, switch sign
			if (!_lexer.getNext() || !fFinal(v))
				return false;
			if (_mode == EXECUTE)
				v.switchSign();
			return true;
		} else if (t == Token::OP_NOT) {
			if (!_lexer.getNext() || !fFinal(v))
				return false;
			if (_mode == EXECUTE)
				v.notOp();
			return true;
		} else if (t == Token::C_INTEGER || t == Token::C_REAL ||
		    t == Token::C_BOOLEAN) {
			if (_mode == EXECUTE)
				v = _lexer.getValue();
			_lexer.getNext();
			return true;
		} else if (t == Token::C_STRING) {
			if (_lexer.getError() == Lexer::STRING_OVERFLOW) {
				_error = STRING_OVERFLOW;
				_lexer.getNext();
				return false;
			}
			if (_mode == EXECUTE) {
				_interpreter.pushString(_lexer.id());
				v.type = Value::Type::STRING;
			}
			_lexer.getNext();
			return true;
		} else if (t == Token::LPAREN) {
			if (!_lexer.getNext() || !fExpression(v))
				return false;
			if (_lexer.getToken() != Token::RPAREN)
				return false;
			else {
				_lexer.getNext();
				return true;
			}
		} else {
			char varName[VARSIZE];
			if (fVar(varName))
				return fIdentifierExpr(varName, v);
			return false;
		}
#endif
	}

}

bool
Parser::fIfStatement()
{
	const Token t = _lexer.getToken();
	LOG(t);
	if (t == Token::KW_THEN) {
		if (_lexer.getNext()) {
			bool res;
			if (_lexer.getToken() == Token::C_INTEGER) {
				if (_mode == EXECUTE)
					_interpreter.gotoLine(_lexer.getValue());
				_lexer.getNext();
				return true;
			} else {
				while (fOperators(res)) {
					if (!res)
						return false;
					else
						_lexer.getNext();
				}
				return true;
			}
		}
	} else if (fGotoStatement())
		return true;
	
	return false;
}

bool
Parser::fGotoStatement()
{
	const Token t = _lexer.getToken();
	LOG(t);
	if (t == Token::KW_GOTO) {
		Value v;
		if (!_lexer.getNext() || !fExpression(v)) {
			_error = EXPRESSION_EXPECTED;
			return false;
		}
		if (_mode == EXECUTE)
			_interpreter.gotoLine(v.value.integer);
		return true;
	} else
		return false;
}

bool
Parser::fCommand()
{
	const Token t = _lexer.getToken();
	LOG(t);
	typedef void (Interpreter::*func)();
	func f = nullptr;
	switch (t) {
#if USE_SAVE_LOAD
	case Token::COM_CHAIN:
		f = &Interpreter::chain;
		break;
#endif
	case Token::COM_CLS:
		f = &Interpreter::cls;
		break;
#if USESTOPCONT
	case Token::COM_CONT:
		f = &Interpreter::cont;
		break;
#endif
#if USE_DUMP
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
#endif
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
		return true;
#if USE_SAVE_LOAD
	case Token::COM_LOAD:
		f = &Interpreter::load;
		break;
#endif
	case Token::COM_NEW:
		f = &Interpreter::newProgram;
		break;
	case Token::COM_RUN:
		f = &Interpreter::run;
		break;
#if USE_SAVE_LOAD
	case Token::COM_SAVE:
		f = &Interpreter::save;
		break;
#endif
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
		break;
	}
	if (f != nullptr) {
		if (_mode == EXECUTE)
			(_interpreter.*f)();
		_lexer.getNext();
		return true;
	}
	return false;
}

/*
 * FOR_CONDS =
 * IMPLICIT_ASSIGNMENT KW_TO EXPRESSION |
 * IMPLICIT_ASSIGNMENT KW_TO EXPRESSION KW_STEP EXPRESSION
 */
bool
Parser::fForConds()
{
	Value v;
	char vName[VARSIZE];
	if (!fImplicitAssignment(vName) ||
	    _lexer.getToken()!=Token::KW_TO || !_lexer.getNext() ||
	    !fExpression(v))
		return false;
	
	Value vStep(Integer(1));
	if (_lexer.getToken() == Token::KW_STEP && (!_lexer.getNext() ||
	    !fExpression(vStep)))
		return false;
	
	if (_mode == EXECUTE)
		_interpreter.pushForLoop(vName, _lexer.getPointer(), v, vStep);
	
	return true;
}

bool
Parser::fVarList()
{
	Token t;
	char varName[VARSIZE];
	do {
		if (!_lexer.getNext() || !fVar(varName))
			return false;
		if (_mode == EXECUTE) {
			_interpreter.pushInputObject(varName);
		} if (!_lexer.getNext())
			return true;
		t = _lexer.getToken();
	} while (t == Token::COMMA);
	return true;
}

bool
Parser::fVar(char *varName)
{
	if ((_lexer.getToken() >= Token::INTEGER_IDENT) &&
	    (_lexer.getToken() <= Token::BOOL_IDENT)) {
		strncpy(varName, _lexer.id(), VARSIZE);
		varName[VARSIZE-1] = 0;
		return true;
	} else
		return false;
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
			return false;
		_interpreter.pushDimensions(dimensions);
		_interpreter.newArray(arrName);
		t = _lexer.getToken();
		if (t != Token::COMMA)
			return true;
		else
			if (!_lexer.getNext())
				return false;
	} while (true);
}

bool
Parser::fArray(uint8_t &dimensions)
{
	if (_lexer.getToken() != Token::LPAREN ||
	    !fDimensions(dimensions) || _lexer.getToken() != Token::RPAREN)
		return false;

	_lexer.getNext();
	return true;
}

bool
Parser::fDimensions(uint8_t &dimensions)
{
	Parser::Value v;
	dimensions = 0;
	do {
		if (!_lexer.getNext() || !fExpression(v))
			return false;
		_interpreter.pushDimension(Integer(v));
		++dimensions;
	} while (_lexer.getToken() == Token::COMMA);
	return true;
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
					return false;
				else if (_lexer.getToken() == Token::RPAREN) {
					break;
				} else {
					if (!fExpression(arg))
						return false;
					_interpreter.pushValue(arg);
				}
			} while (_lexer.getToken() == Token::COMMA);
			_lexer.getNext();
			bool result = true;
			if (_mode == EXECUTE) {
				result = ((*f)(_interpreter));
				if (!result || !_interpreter.popValue(v))
					return false;
			}
		} else { // No such function, array variable
			uint8_t dim;
			if (fArray(dim)) {
				if (_mode == EXECUTE &&
				    _interpreter.valueFromArray(v, varName))
					return true;
			} else
				return false;
		}
	} else
		if (_mode == EXECUTE)
			_interpreter.valueFromVar(v, varName);
	return true;
}

#if USE_MATRIX
/*
 * MATRIX_OPERATION =
 *     PRINT MATRIX_PRINT |
 *     DET VAR |
 *     VAR EQUALS MATRIX_EXPRESSION
 */
bool
Parser::fMatrixOperation()
{
	char buf[VARSIZE];
	if (fVar(buf)) {
		if (!_lexer.getNext())
			return false;
		if (_lexer.getToken() == Token::EQUALS) {
			if (_lexer.getNext() && fMatrixExpression(buf)) {
				_lexer.getNext();
				return true;
			} else
				return false;
		} else
			return false;
	} else if (_lexer.getToken() == Token::KW_PRINT) {
		if (_lexer.getNext() && fMatrixPrint()) {
			_lexer.getNext();
			return true;
		}
	} else if (_lexer.getToken() == Token::KW_DET) {
		if (_lexer.getNext() && fVar(buf)) {
			_interpreter.matrixDet(buf);
			_lexer.getNext();
			return true;
		}
	}
	return false;
}

bool
Parser::fMatrixPrint()
{
	char buf[VARSIZE];
	if (fVar(buf)) {
		_interpreter.printMatrix(buf);
		return true;
	} else
		return false;
}

bool
Parser::fMatrixExpression(const char *buf)
{
	Interpreter::MatrixOperation_t mo;
	
	switch (_lexer.getToken()) {
	case Token::KW_ZER: // Zero matrix
		_interpreter.zeroMatrix(buf);
		return true;
	case Token::KW_CON: // Ones matrix
		_interpreter.onesMatrix(buf);
		return true;
	case Token::KW_IDN: // Identity matrix
		_interpreter.identMatrix(buf);
		return true;
	case Token::LPAREN: { // Scalar
		Value v;
		char first[VARSIZE];
		if (_lexer.getNext() && fExpression(v) &&
		    _lexer.getToken() == Token::RPAREN &&
		    _lexer.getNext() && _lexer.getToken() == Token::STAR &&
		    _lexer.getNext() && fVar(first)) {
			_interpreter.pushValue(v);
			_interpreter.assignMatrix(buf, first, nullptr,
			    Interpreter::MO_SCALE);
		} else
			return false;
	}
		break;
	case Token::KW_TRN:
		mo = Interpreter::MO_TRANSPOSE;
	case Token::KW_INV:
		mo = Interpreter::MO_INVERT;
	{
		char first[VARSIZE];
		if (_lexer.getNext() && _lexer.getToken() == Token::LPAREN &&
		    _lexer.getNext() && fVar(first) &&
		    _lexer.getNext() && _lexer.getToken() == Token::RPAREN) {
			_interpreter.assignMatrix(buf, first, nullptr, mo);
			return true;
		} else
			return false;
	}
		break;
	default:
		break;
	}
	
	char first[VARSIZE];
	if (fVar(first)) { // Matrix expression
		if (_lexer.getNext()) {
			switch (_lexer.getToken()) {
			case Token::PLUS:
				mo = Interpreter::MO_SUM;
				break;
			case Token::MINUS:
				mo = Interpreter::MO_SUB;
				break;
			case Token::STAR:
				mo = Interpreter::MO_MUL;
				break;
			default:
				return false;
			}
			char second[VARSIZE];
			if (_lexer.getNext() && fVar(second)) {
				_interpreter.assignMatrix(buf, first, second,
				    mo);
				return true;
			} else
				return false;
		}
		_interpreter.assignMatrix(buf, first);
		return true;
	}
	return false;
}

#endif // USE_MATRIX

}
