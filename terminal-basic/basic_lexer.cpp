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

#include "basic_lexer.hpp"
#include "helper.hpp"

#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

/*
 * OP_AND = "AND"       // 1
 * KW_ARRAYS = "ARRAYS" // 1
 * KW_BASE = "BASE"     // 2
 * COM_CHAIN = "CHAIN"  // 3
 * COM_CLS = "CLS"      // 4
 * KW_CON = "CON"
 * KW_DATA = "DATA"     // 5
 * KW_DEF = "DEF"       // 6
 * KW_DET = "DET"
 * COM_DELAY = "DELAY"  // 7
 * KW_DIM = "DIM"       // 8
 * KW_DO = "DO"
 * COM_DUMP = "DUMP"    // 9
 * KW_END = "END"       // 10
 * KW_FALSE = "FALSE"   // 11
 * KW_FOR = "FOR"       // 12
 * KW_GOSUB = "GOSUB"   // 13
 * KW_GOTO = "GOTO"     // 14
 * KW_GO = "GO"         // 15
 * KW_IDN = "IDN"
 * KW_IF = "IF"         // 16
 * KW_INPUT = "INPUT"   // 17
 * KW_INV = "INV"
 * KW_LET = "LET"       // 18
 * COM_LIST = "LIST"    // 19
 * COM_LOAD = "LOAD"    // 20
 * KW_MAT = "MAT"
 * COM_NEW = "NEW"      // 21
 * KW_NEXT = "NEXT"     // 22
 * OP_NOT = "NOT"
 * KW_ON = "ON"         // 23
 * KW_OPTION = "OPTION" // 24
 * OP_OR = "OR"
 * KW_PRINT = "PRINT"   // 25
 * KW_RANDOMIZE = "RANDOMIZE"
 * KW_READ = "READ"
 * KW_REM = "REM"
 * KW_RETURN = "RETURN"
 * COM_RUN = "RUN"
 * COM_SAVE = "SAVE"
 * KW_STEP = "STEP"
 * KW_STOP = "STOP"
 * KW_TAB = "TAB"
 * KW_THEN = "THEN"
 * KW_TO = "TO"
 * KW_TRN = "TRN"
 * KW_TRUE = "TRUE"
 * KW_VARS = "VARS"
 * KW_ZER = "ZER"
 *
 * STAR = '*'
 * SLASH = '/'
 * PLUS = '+'
 * COLON = ':'
 * SEMI = ';'
 * LT = '<'
 * LTE = "<="
 * GT = '>'
 * GTE = ">="
 * EQUALS = '='
 * NE = "<>"
 * NEA = "><"
 * MINUS = '-'
 * POW = '^'
 * IDENT = [A-Z][A-Z0-9]*
 * C_INTEGER = [0-9]+
 * C_REAL = [0-9]+[.][0-9]*
 */

namespace BASIC
{
/*
const char sNOTOKENS[] PROGMEM = "NOTOKENS";  // 0
const char sOP_AND[] PROGMEM = "AND";         // 1
#if USE_DUMP
const char sARRAYS[] PROGMEM = "ARRAYS";      // 2
#endif
const char sBASE[] PROGMEM = "BASE";          // 3
#if USE_SAVE_LOAD
const char sCHAIN[] PROGMEM = "CHAIN";        // 4
#endif
const char sCLS[] PROGMEM = "CLS";            // 5
#if USESTOPCONT
const char sCONT[] PROGMEM = "CONT";          // 6
#endif
#if USE_MATRIX
const char sCON[] PROGMEM = "CON";            // 5
#endif
const char sDATA[] PROGMEM = "DATA";          // 6
const char sDEF[] PROGMEM = "DEF";            // 7
//const char sDELAY[] PROGMEM = "DELAY";        // 8
#if USE_MATRIX
const char sDET[] PROGMEM = "DET";            // 5
#endif
const char sDIM[] PROGMEM = "DIM";            // 9
#if USE_DOLOOP
const char sDO[] PROGMEM = "DO";              //
#endif
#if USE_DUMP
const char sDUMP[] PROGMEM = "DUMP";          // 10
#endif
const char sEND[] PROGMEM = "END";            // 11
const char sFALSE[] PROGMEM = "FALSE";        // 12
const char sFOR[] PROGMEM = "FOR";            // 13
const char sGOSUB[] PROGMEM = "GOSUB";        // 14
const char sGOTO[] PROGMEM = "GOTO";          // 15
const char sGO[] PROGMEM = "GO";              // 16
#if USE_MATRIX
const char sIDN[] PROGMEM = "IDN";
#endif
const char sIF[] PROGMEM = "IF";              // 17
const char sINPUT[] PROGMEM = "INPUT";        // 18
#if USE_MATRIX
const char sINV[] PROGMEM = "INV";	      // 18
#endif
const char sLET[] PROGMEM = "LET";            // 19
const char sLIST[] PROGMEM = "LIST";          // 20
#if USE_SAVE_LOAD
const char sLOAD[] PROGMEM = "LOAD";          // 21
#endif
#if USE_DOLOOP
const char sLOOP[] PROGMEM = "LOOP";
#endif
#if USE_MATRIX
const char sMAT[] PROGMEM = "MAT";
#endif
const char sNEW[] PROGMEM = "NEW";            // 22
const char sNEXT[] PROGMEM = "NEXT";          // 23
const char sOP_NOT[] PROGMEM = "NOT";
const char sON[] PROGMEM = "ON";              // 24
const char sOPTION[] PROGMEM = "OPTION";      // 25
const char sOP_OR[] PROGMEM = "OR";
const char sPRINT[] PROGMEM = "PRINT";        // 26
#if USE_RANDOM
const char sRANDOMIZE[] PROGMEM = "RANDOMIZE";// 27
#endif
const char sREAD[] PROGMEM = "READ";          // 28
const char sREM[] PROGMEM = "REM";            // 29
const char sRETURN[] PROGMEM = "RETURN";      // 30
const char sRUN[] PROGMEM = "RUN";
#if USE_SAVE_LOAD
const char sSAVE[] PROGMEM = "SAVE";
#endif
const char sSTEP[] PROGMEM = "STEP";
#if USESTOPCONT
const char sSTOP[] PROGMEM = "STOP";
#endif
const char sTAB[] PROGMEM = "TAB";
const char sTHEN[] PROGMEM = "THEN";
const char sTO[] PROGMEM = "TO";
#if USE_MATRIX
const char sTRN[] PROGMEM = "TRN";
#endif
const char sTRUE[] PROGMEM = "TRUE";
#if USE_DUMP
const char sVARS[] PROGMEM = "VARS";
#endif
#if USE_MATRIX
const char sZER[] PROGMEM = "ZER";
#endif*/
const char sSTAR[] PROGMEM = "*";
const char sSLASH[] PROGMEM = "/";
#if USE_REALS && USE_INTEGER_DIV
const char sBACK_SLASH[] PROGMEM = "\\";
#endif
const char sPLUS[] PROGMEM = "+";
const char sMINUS[] PROGMEM = "-";
const char sEQUALS[] PROGMEM = "=";
const char sCOLON[] PROGMEM = ":";
const char sSEMI[] PROGMEM = ";";
const char sLT[] PROGMEM = "<";
const char sGT[] PROGMEM = ">";
const char sLTE[] PROGMEM = "<=";
const char sGTE[] PROGMEM = ">=";
const char sNE[] PROGMEM = "<>";
#if CONF_USE_ALTERNATIVE_NE
const char sNEA[] PROGMEM = "><";
#endif
const char sCOMMA[] PROGMEM = ",";
const char sPOW[] PROGMEM = "^";
const char sLPAREN[] PROGMEM = "(";
const char sRPAREN[] PROGMEM = ")";

PGM_P const Lexer::tokenStrings[] PROGMEM = {
	sSTAR,
	sSLASH,
#if USE_REALS && USE_INTEGER_DIV
	sBACK_SLASH,
#endif
	sPLUS, sMINUS,

	sEQUALS,
	sCOLON, sSEMI,
	sLT, sGT,
	sLTE, sGTE,
	sNE,
#if CONF_USE_ALTERNATIVE_NE
	sNEA,
#endif
	sCOMMA,
	sPOW,
	sLPAREN, sRPAREN
};

static const uint8_t tokenTable[] PROGMEM = {
	0x80,
	'A', 'N', 'D'+0x80,
#if USE_DUMP
	'A', 'R', 'R', 'A', 'Y', 'S'+0x80, // 1
#endif
//	'B', 'A', 'S', 'E'+0x80,           // 2
#if USE_SAVE_LOAD
	'C', 'H', 'A', 'I', 'N'+0x80,      // 3
#endif
#if USE_TEXTATTRIBUTES
	'C', 'L', 'S'+0x80,                // 4
#endif
#if USESTOPCONT
	'C', 'O', 'N', 'T'+0x80,
#endif
#if USE_MATRIX
	'C', 'O', 'N'+0x80,
#endif
#if USE_DATA
	'D', 'A', 'T', 'A'+0x80,           // 5
#endif
#if USE_DEFFN
	'D', 'E', 'F'+0x80,                // 6
#endif
#if USE_DELAY
	'D', 'E', 'L', 'A', 'Y'+0x80,      // 7
#endif
#if USE_MATRIX
	'D', 'E', 'T'+0x80,
#endif
	'D', 'I', 'M'+0x80,                // 8
#if USE_DIV_KW
	'D', 'I', 'V'+0x80,
#endif
#if USE_DOLOOP
	'D', 'O'+0x80,
#endif
#if USE_DUMP
	'D', 'U', 'M', 'P'+0x80,           // 9
#endif
	'E', 'N', 'D'+0x80,                // 10
	'F', 'A', 'L', 'S', 'E'+0x80,      // 11
	'F', 'O', 'R'+0x80,                // 12
	'G', 'O', 'S', 'U', 'B'+0x80,      // 13
	'G', 'O', 'T', 'O'+0x80,           // 14
#if CONF_SEPARATE_GO_TO
	'G', 'O'+0x80,                     // 15
#endif
#if USE_MATRIX
	'I', 'D', 'N'+0x80,
#endif
	'I', 'F'+0x80,                     // 16
	'I', 'N', 'P', 'U', 'T'+0x80,      // 17
#if USE_MATRIX
	'I', 'N', 'V'+0x80,
#endif
	'L', 'E', 'T'+0x80,                // 18
	'L', 'I', 'S', 'T'+0x80,           // 19
#if USE_SAVE_LOAD
	'L', 'O', 'A', 'D'+0x80,           // 20
#endif
#if USE_TEXTATTRIBUTES
	'L', 'O', 'C', 'A', 'T', 'E'+0x80, // 21
#endif
#if USE_DOLOOP
	'L', 'O', 'O', 'P'+0x80,
#endif
#if USE_MATRIX
	'M', 'A', 'T'+0x80,
#endif
#if USE_INTEGER_DIV
	'M', 'O', 'D'+0x80,
#endif
	'N', 'E', 'W'+0x80,                // 21
	'N', 'E', 'X', 'T'+0x80,           // 22
	'N', 'O', 'T'+0x80,
	'O', 'N'+0x80,                     // 23
//	'O', 'P', 'T', 'I', 'O', 'N'+0x80, // 24
	'O', 'R'+0x80,
	'P', 'R', 'I', 'N', 'T'+0x80,      // 25
#if USE_RANDOM
	'R', 'A', 'N', 'D', 'O', 'M', 'I', 'Z', 'E'+0x80, //26
#endif
#if USE_DATA
	'R', 'E', 'A', 'D'+0x80,           // 27
#endif
	'R', 'E', 'M'+0x80,
#if USE_DATA
	'R', 'E', 'S', 'T', 'O', 'R', 'E'+0x80,
#endif
	'R', 'E', 'T', 'U', 'R', 'N'+0x80,
	'R', 'U', 'N'+0x80,
#if USE_SAVE_LOAD
	'S', 'A', 'V', 'E'+0x80,
#endif
#if CONF_USE_SPC_PRINT_COM
	'S', 'P', 'C'+0x80,
#endif
	'S', 'T', 'E', 'P'+0x80,
#if USESTOPCONT
	'S', 'T', 'O', 'P'+0x80,
#endif
#if USE_TEXTATTRIBUTES
	'T', 'A', 'B'+0x80,
#endif
	'T', 'H', 'E', 'N'+0x80,
	'T', 'O'+0x80,
#if USE_MATRIX
	'T', 'R', 'N'+0x80,
#endif
	'T', 'R', 'U', 'E'+0x80,
#if USE_DUMP
	'V', 'A', 'R', 'S'+0x80,
#endif
	'X', 'O', 'R'+0x80,
#if USE_MATRIX
	'Z', 'E', 'R'+0x80,
#endif
	0
};

#if ARDUINO_LOG

Logger&
operator<<(Logger &logger, Token tok)
{
	char buf[12];
	strcpy_P(buf, (PGM_P) pgm_read_word(&(Lexer::tokenStrings[uint8_t(tok)])));

	logger.log(buf);
	return (logger);
}
#endif

#define SYM (uint8_t(_string[_pointer]))

const uint8_t*
Lexer::getTokenString(Token t, uint8_t *buf)
{
	const uint8_t *result = tokenTable, *pointer = result;
	uint8_t c; uint8_t index = 0;

	do {
		c=pgm_read_byte(pointer++);
		if (c & 0x80) {
			if (index++ == uint8_t(t)) {
				pointer = result;
				result = buf;
				while (((c = pgm_read_byte(pointer++)) & 0x80) == 0)
					*(buf++) = c;
				*(buf++) = c&0x7F;
				*buf = 0;
				return result;
			} else
				result = pointer;
		}
	} while (c != 0);

	return nullptr;
}

void
Lexer::init(const char *string)
{
	LOG_TRACE;
	assert(string != nullptr);

	_pointer = 0, _string = string;
}

bool
Lexer::getNext()
{
	LOG_TRACE;

	_token = Token::NOTOKENS;
	_error = NO_ERROR;
	_valuePointer = 0;
	while (SYM > 0) {
		if (SYM >= 0x80) {
			_token = Token(SYM & 0x7F);
			next();
			if (_token == Token::C_INTEGER)
				binaryInteger();
#if USE_REALS
                        else if (_token == Token::C_REAL)
				binaryReal();
#endif
			return true;
		} else if (isdigit(SYM)) {
			decimalNumber();
			return true;
		} else if (isalpha(SYM)) {
			uint8_t index;
			uint8_t *pos = (uint8_t*)_string+_pointer;
			if ((pos = scanTable(pos, tokenTable, index)) != NULL) {
				_token = Token(index);
				if (_token == Token::KW_TRUE)
					_value = true, _token = Token::C_BOOLEAN;
				else if (_token == Token::KW_FALSE)
					_value = false, _token = Token::C_BOOLEAN;
				_pointer += uint8_t(pos - ((uint8_t*)_string+
				    _pointer));
				return true;
			} else {
				pushSYM();
				ident();
				return true;
			}
		}
		switch (SYM) {
		case '=':
			_token = Token::EQUALS;
			next();
			return true;
		case ';':
			_token = Token::SEMI;
			next();
			return true;
		case '.':
			decimalNumber();
			return (true);
		case ',':
			_token = Token::COMMA;
			next();
			return true;
		case ':':
			_token = Token::COLON;
			next();
			return true;
		case '<':
			fitst_LT();
			return true;
		case '>':
			fitst_GT();
			return true;
		case '(':
			_token = Token::LPAREN;
			next();
			return true;
		case ')':
			_token = Token::RPAREN;
			next();
			return true;
		case '+':
			_token = Token::PLUS;
			next();
			return true;
		case '-':
			_token = Token::MINUS;
			next();
			return true;
		case '*':
			_token = Token::STAR;
			next();
			return true;
		case '/':
			_token = Token::SLASH;
			next();
			return true;
#if USE_REALS && USE_INTEGER_DIV
		case '\\':
			_token = Token::BACK_SLASH;
			next();
			return true;
#endif
		case '^':
			_token = Token::POW;
			next();
			return true;
		case '"':
			next();
			stringConst();
			return true;
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			next();
			break;
		default:
			next();
			return true;
		}
	}
	return false;
}

void
Lexer::pushSYM()
{
	if (_valuePointer < STRINGSIZE - 1)
		_id[_valuePointer++] = SYM;
	else
		_error = STRING_OVERFLOW;
	next();
}

void
Lexer::next()
{
	++_pointer;
}

void
Lexer::fitst_LT()
{
	next();
#if OPT == OPT_SPEED
	switch (SYM) {
	case '=':
		_token = Token::LTE;
		break;
	case '>':
		_token = Token::NE;
		break;
	default:
		_token = Token::LT;
		return;
	}
#else
	if (SYM == '=')
		_token = Token::LTE;
	else if (SYM == '>')
		_token = Token::NE;
	else {
		_token = Token::LT;
		return;
	}
#endif
	next();
}

void
Lexer::fitst_GT()
{
	next();
#if OPT == OPT_SPEED
	switch (SYM) {
	case '=':
		_token = Token::GTE;
		break;
#if CONF_USE_ALTERNATIVE_NE
	case '<':
		_token = Token::NEA;
		break;
#endif // CONF_USE_ALTERNATIVE_NE
	default:
		_token = Token::GT;
		return;
	}
#else
	if (SYM == '=')
		_token = Token::GTE;
#if CONF_USE_ALTERNATIVE_NE
	else if (SYM == '<')
		_token = Token::NEA;
#endif // CONF_USE_ALTERNATIVE_NE
	else {
		_token = Token::GT;
		return;
	}
#endif
	next();
}

void
Lexer::decimalNumber()
{
	LOG_TRACE;

#if USE_LONGINT
	_value.type = Parser::Value::LONG_INTEGER;
	LongInteger *val = &_value.value.longInteger;
#else
	_value.type = Parser::Value::INTEGER;
	Integer *val = &_value.value.integer;
#endif // USE_LONGINT
#if USE_REALS
	if (SYM == '.')
		*val = 0;
	else
#endif
		*val = SYM - '0';
	while (SYM > 0) {
#if USE_REALS
		if (_value.type == Parser::Value::REAL) {
			next();
			if (isdigit(SYM)) {
				_value.value.real *= Real(10);
				_value.value.real += SYM - '0';
				continue;
			}
		} else if (SYM != '.') {

#endif
			next();
			if (isdigit(SYM)) {
#if USE_REALS
				if (*val > MAXINT/INT(10)) {
					_value.type = Parser::Value::REAL;
					_value.value.real = Real(*val);
					_value.value.real *= Real(10);
					_value.value.real += SYM - '0';
				} else {
#endif
					*val *= INT(10);
					*val += SYM - '0';
#if USE_REALS
				}
#endif
				continue;
			}
#if USE_REALS
		}
#endif
		
			
		switch (SYM) {
#if USE_REALS
		case '.':
		{
			if (_value.type != Parser::Value::REAL) {
				_value.type = Parser::Value::REAL;
				_value.value.real = Real(*val);
			}
			Real d = 1;
			while (true) {
				next();
				if (isdigit(SYM)) {
					d /= 10.f;
					_value.value.real += Real(SYM - '0') * d;
					continue;
				} else if (SYM == 0) {
					_token = Token::C_REAL;
					return;
				} else if (SYM == 'E' || SYM == 'e') {
					if (!numberScale())
						_token = Token::NOTOKENS;
					else
						_token = Token::C_REAL;
					return;
				} else {
					_token = Token::C_REAL;
					return;
				}
			}
		}
			break;
		case 'E':
		case 'e':
		{
			if (_value.type == Parser::Value::INTEGER
#if USE_LONGINT
			    || _value.type == Parser::Value::LONG_INTEGER
#endif
			    )
				_value = Real(_value);
			if (!numberScale()) {
				_token = Token::NOTOKENS;
				return;
			}
		}
#endif
		default:
			if (_value.type == Parser::Value::INTEGER
#if USE_LONGINT
			    || _value.type == Parser::Value::LONG_INTEGER
#endif
			    )
				_token = Token::C_INTEGER;
#if USE_REALS
			else
				_token = Token::C_REAL;
#endif
			return;
		}
	}
}

void
Lexer::binaryInteger()
{
#if USE_LONGINT
	_value.type = Parser::Value::LONG_INTEGER;
#else
	_value.type = Parser::Value::INTEGER;
#endif
	char buf[sizeof(INT)];
	for (uint8_t i=0; i<sizeof(INT); ++i) {
		buf[i] = SYM;
		next();
	}
#if USE_LONGINT
	_value.value.longInteger = *reinterpret_cast<const LongInteger*>(buf);
#else
	_value.value.integer = *reinterpret_cast<const Integer*>(buf);
#endif
}

#if USE_REALS
void
Lexer::binaryReal()
{
	_value.type = Parser::Value::REAL;
	char buf[sizeof(Real)];
	for (uint8_t i=0; i<sizeof(Real); ++i) {
		buf[i] = SYM;
		next();
	}
	_value.value.real = *reinterpret_cast<const Real*>(buf);
}

bool
Lexer::numberScale()
{
	Integer scale(0);
	bool sign = true;

	next();
	if (SYM == '-') {
		sign = false;
		next();
	} else if (SYM == '+')
		next();

	if (isdigit(SYM)) {
		scale += SYM - '0';
		next();
	} else
		return false;

	while (true) {
		if (isdigit(SYM)) {
			scale *= Integer(10);
			scale += SYM - '0';
			next();
			continue;
		} else {
			if (!sign)
				scale = -scale;
			Real pw = pow(Real(10), scale);
			_value *= pw;
			return true;
		}
	}
}
#endif // USE_REALS

void
Lexer::ident()
{
	while (isalnum(SYM)
#if ALLOW_UNDERSCORE_ID
	    || (SYM == '_')
#endif
	    ) {
		pushSYM();
	}
	if (SYM == '%') {
		pushSYM();
#if USE_LONGINT
		if (SYM == '%') {
			pushSYM();
			_token = Token::LONGINT_IDENT;
		} else
#endif
			_token = Token::INTEGER_IDENT;
	} else if (SYM == '$') {
		pushSYM();
		_token = Token::STRING_IDENT;
	} else if (SYM == '!') {
		pushSYM();
		_token = Token::BOOL_IDENT;
	} else
		_token = Token::REAL_IDENT;
	_value.type = Parser::Value::STRING;
	_id[_valuePointer] = 0;
}

void
Lexer::stringConst()
{
	while (SYM != 0) {
		if (SYM == '"') {
			next();
			_token = Token::C_STRING;
			_id[_valuePointer] = 0;
			return;
		}
		pushSYM();
	}
}

}
