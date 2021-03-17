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

#include "math.hpp"
#include "basic_math.hpp"
#include <string.h>
#include <assert.h>

#include <EEPROM.h>
#include <stdbool.h>

#include "helper.hpp"
#include <avr/crc16.h>

#include "basic_interpreter.hpp"
#include "basic_interpreter_program.hpp"
#include "basic_parser_value.hpp"
#include "arduino_logger.hpp"
#include "bytearray.hpp"
#include "version.h"
#include "ascii.hpp"
#ifdef ARDUINO
#include "config_arduino.hpp"
#else
#include "config_linux.hpp"
#endif

namespace BASIC
{

class Interpreter::AttrKeeper
{
public:

	explicit AttrKeeper(Interpreter &i, TextAttr a) :
	_i(i), _a(a)
	{
		if (_a == NO_ATTR)
			return;
		if ((uint8_t(a) & uint8_t(BRIGHT)) != uint8_t(NO_ATTR))
			_i._output.print("\x1B[1m");
		if ((uint8_t(a) & uint8_t(UNDERSCORE)) != uint8_t(NO_ATTR))
			_i._output.print("\x1B[4m");
		if ((uint8_t(a) & uint8_t(REVERSE)) != uint8_t(NO_ATTR))
			_i._output.print("\x1B[7m");
		if ((uint8_t(a) & 0xF0) == C_YELLOW)
			_i._output.print("\x1B[33m");
		else if ((uint8_t(a) & 0xF0) == C_GREEN)
			_i._output.print("\x1B[32m");
		else if ((uint8_t(a) & 0xF0) == C_RED)
			_i._output.print("\x1B[31m");
		else if ((uint8_t(a) & 0xF0) == C_BLUE)
			_i._output.print("\x1B[34m");
		else if ((uint8_t(a) & 0xF0) == C_MAGENTA)
			_i._output.print("\x1B[35m");
		else if ((uint8_t(a) & 0xF0) == C_CYAN)
			_i._output.print("\x1B[36m");
		else if ((uint8_t(a) & 0xF0) == C_WHITE)
			_i._output.print("\x1B[37m");
	}

	~AttrKeeper()
	{
		if (_a == NO_ATTR)
			return;
		_i._output.print("\x1B[0m");
	}
private:
	Interpreter &_i;
	TextAttr _a;
};

static const char strStatic[] PROGMEM = "STATIC";
static const char strDynamic[] PROGMEM = "DYNAMIC";
static const char strError[] PROGMEM = "ERROR";
static const char strSemantic[] PROGMEM = "SEMANTIC";
static const char strReady[] PROGMEM = "READY";
static const char strBytes[] PROGMEM = "BYTES";
static const char strAvailable[] PROGMEM = "AVAILABLE";
static const char strucBASIC[] PROGMEM = "ucBASIC";
static const char strVERSION[] PROGMEM = "VERSION";
static const char strTEXT[] PROGMEM = "TEXT";
static const char strOF[] PROGMEM = "OF";
static const char strVARS[] PROGMEM = "VARS";
static const char strARRAYS[] PROGMEM = "ARRAYS";
static const char strSTACK[] PROGMEM = "STACK";
static const char strDIR[] PROGMEM = "DIR";
static const char strREALLY[] PROGMEM = "REALLY";

PGM_P const Interpreter::_progmemStrings[NUM_STRINGS] PROGMEM = {
	strStatic, // STATIC
	strDynamic, // DYNAMAIC
	strError, // ERROR
	strSemantic, // SEMANTIC
	strReady, // READY
	strBytes, // BYTES
	strAvailable, // AVAILABLE
	strucBASIC, // ucBASIC
	strVERSION, // VERSION
	strTEXT, // TEXT
	strOF, // OF
	strVARS, // VARS
	strARRAYS, // ARRAYS
	strSTACK, // STACK
	strDIR,	// DIR
	strREALLY
};

#define ESTRING(en) (_progmemStrings[en])

void
Interpreter::valueFromVar(Parser::Value &v, const char *varName)
{
	const Interpreter::VariableFrame *f = getVariable(varName);
	if (f == NULL)
		return;

	switch (f->type) {
	case VF_INTEGER:
		v = f->get<Integer>();
		break;
#if USE_LONGINT
	case VF_LONG_INTEGER:
		v = f->get<LongInteger>();
		break;
#endif
	case VF_REAL:
		v = f->get<Real>();
		break;
	case VF_BOOLEAN:
		v = f->get<bool>();
		break;
	case VF_STRING:
	{
		v.type = Parser::Value::STRING;
		Program::StackFrame *fr =
		    _program.push(Program::StackFrame::STRING);
		if (fr == NULL) {
			raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
			return;
		}
		strcpy(fr->body.string, f->bytes);
		break;
	}
	}
}

bool
Interpreter::valueFromArray(Parser::Value &v, const char *name)
{
	ArrayFrame *f = _program.arrayByName(name);
	if (f == NULL) {
		raiseError(DYNAMIC_ERROR, NO_SUCH_ARRAY);
		return false;
	}

	size_t index;
	if (!arrayElementIndex(f, index)) {
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		return false;
	}

	switch (f->type) {
	case VF_BOOLEAN:
		v.type = Parser::Value::BOOLEAN;
		v.value.boolean = f->get<bool>(index);
		break;
	case VF_INTEGER:
		v.type = Parser::Value::INTEGER;
		v.value.integer = f->get<Integer>(index);
		break;
#if USE_LONGINT
	case VF_LONG_INTEGER:
		v.type = Parser::Value::LONG_INTEGER;
		v.value.longInteger = f->get<LongInteger>(index);
		break;
#endif
	case VF_REAL:
		v.type = Parser::Value::REAL;
		v.value.real = f->get<Real>(index);
		break;
	default:
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		return false;
	}
	return true;
}

Interpreter::Interpreter(Stream &stream, Print &output, Program &program,
    FunctionBlock *first) :
    _program(program), _state(SHELL), _input(stream), _output(output),
    _parser(_lexer, *this, first)
{
	_input.setTimeout(10000L);
}

void
Interpreter::init()
{
	_parser.init();
	print(ucBASIC, BRIGHT);

	print(S_VERSION), print(VERSION, BRIGHT), newline();
	print(long(_program.programSize - _program._arraysEnd), BRIGHT);
	print(BYTES), print(AVAILABLE), newline();
	//_stream.print("\x1B[c");
}

void
Interpreter::step()
{
	LOG_TRACE;

	switch (_state) {
		// waiting for user input command or program line
	case SHELL:
	{
		print(READY, BRIGHT);
		newline();
	}
		// fall through
		// waiting for user input next program line
	case PROGRAM_INPUT:
		_state = COLLECT_INPUT;
		_inputPosition = 0;
		memset(_inputBuffer, 0xFF, PROGSTRINGSIZE);
		break;
		// collection input buffer
	case COLLECT_INPUT:
		if (readInput())
			exec();
		break;
	case VAR_INPUT:
		if (readInput()) {
			doInput();
			_state = EXECUTE;
		}
		break;
	case EXECUTE:
		char c = char(ASCII::NUL);
#ifdef ARDUINO
		if (_input.available() > 0)
			c = _input.read();
#endif
		if (_program._current < _program._textEnd && c != char(ASCII::EOT)) {
			Program::String *s = _program.current();
			if (!_parser.parse(s->text + _program._textPosition))
				raiseError(STATIC_ERROR);
			_program.getString();
		} else
			_state = SHELL;
	}
}

void
Interpreter::exec()
{
	const char *pString;
	
	_lexer.init(_inputBuffer);
	if (_lexer.getNext() && (_lexer.getToken() == Token::C_INTEGER)) {
		Integer pLine = Integer(_lexer.getValue());
		if (TOKENIZE) {
			tokenize();
			pString = _inputBuffer;
		} else {
			pString = _inputBuffer + _lexer.getPointer();
			_inputPosition -= _lexer.getPointer();
		}
		if (!_program.addLine(pLine, pString, _inputPosition)) {
			raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
			_state = SHELL;
			return;
		} else
			_state = PROGRAM_INPUT;
	} else {
		_state = SHELL;
		if (!_parser.parse(_inputBuffer))
			raiseError(STATIC_ERROR);
	}
}

void
Interpreter::tokenize()
{
	char tempBuffer[PROGSTRINGSIZE];
	size_t position = 0, lexerPosition = _lexer.getPointer();
	while (_lexer.getNext()) {
		uint8_t t = uint8_t(0x80) + uint8_t(_lexer.getToken());
		if (_lexer.getToken() <= Token::OP_NOT) { // One byte tokens
			tempBuffer[position++] = t;
			lexerPosition = _lexer.getPointer();
			if (_lexer.getToken() == Token::KW_REM) { // Save rem text as is
				while (_inputBuffer[lexerPosition] == ' ' ||
				    _inputBuffer[lexerPosition] == '\t')
					++lexerPosition;
				uint8_t remaining = strlen(_inputBuffer)-lexerPosition;
				memcpy(tempBuffer+position, _inputBuffer+lexerPosition,
				    remaining);
				position += remaining;
				break;
			}
		} else if (_lexer.getToken() == Token::C_INTEGER) {
			tempBuffer[position++] = t;
#if USE_LONGINT
			LongInteger v = LongInteger(_lexer.getValue());
			tempBuffer[position++] = v >> 24;
			tempBuffer[position++] = (v >> 16) & 0xFF;
			tempBuffer[position++] = (v >> 8) & 0xFF;
			tempBuffer[position++] = v & 0xFF;
#else
			Integer v = Integer(_lexer.getValue());
			tempBuffer[position++] = (v >> 8) & 0xFF;
			tempBuffer[position++] = v & 0xFF;
#endif
			lexerPosition = _lexer.getPointer();
		} else { // Other tokens
			while (_inputBuffer[lexerPosition] == ' ' ||
			    _inputBuffer[lexerPosition] == '\t')
				++lexerPosition;
			memcpy(tempBuffer+position, _inputBuffer+lexerPosition,
			    _lexer.getPointer()-lexerPosition);
			position += _lexer.getPointer()-lexerPosition;
			lexerPosition = _lexer.getPointer();
		}
	}
	memcpy(_inputBuffer, tempBuffer, position);
	_inputBuffer[position] = 0;
}

void
Interpreter::cls()
{
	_output.print("\x1B[2J"), _input.print("\x1B[H");
}

void
Interpreter::doInput()
{
	Lexer l;
	l.init(_inputBuffer);
	Parser::Value v(Integer(0));
	bool neg = false;
n:
	if (l.getNext()) {
		switch (l.getToken()) {
		case Token::MINUS:
			neg = true;
			goto n;
		case Token::PLUS:
			neg = false;
			goto n;
		case Token::C_INTEGER:
		case Token::C_REAL:
			v = l.getValue();
			break;
		case Token::C_STRING:
		{
			v = l.getValue();
			pushString(l.id());
		}
			break;
		default:
			raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		}
	}
	if (neg)
		v = -v;
	setVariable(_inputVarName, v);
}

void
Interpreter::list(uint16_t start, uint16_t stop)
{
	_program.reset();
	for (Program::String *s = _program.getString(); s != NULL;
	    s = _program.getString()) {
		if (s->number < start)
			continue;
		if (stop > 0 && s->number > stop)
			break;
		
		print(long(s->number), C_YELLOW);
		
		Lexer lex;
		lex.init(s->text);
		while (lex.getNext()) {
			print(lex);
			if (lex.getToken() == Token::KW_REM) {
				print(s->text+lex.getPointer());
				break;
			}
		}
		newline();
	}
}

void
Interpreter::dump(DumpMode mode)
{
	switch (mode) {
	case MEMORY:
	{
		ByteArray ba((uint8_t*) _program._text, _program.programSize);
		_input.println(ba);
		print(Token::KW_END), print(S_OF), print(S_TEXT), _input.print('\t');
		_input.println(unsigned(_program._textEnd), HEX);
		print(Token::KW_END), print(S_OF), print(S_VARS), _input.print('\t');
		_input.println(unsigned(_program._variablesEnd), HEX);
		print(Token::KW_END), print(S_OF), print(S_ARRAYS), _input.print('\t');
		_input.println(unsigned(_program._arraysEnd), HEX);
		print(S_STACK), _input.print('\t');
		_input.println(unsigned(_program._sp), HEX);
	}
		break;
	case VARS:
	{
		size_t index = _program._textEnd;
		for (VariableFrame *f = _program.variableByIndex(index);
		    (f != NULL) && (_program.variableIndex(f) <
		    _program._variablesEnd); f = _program.variableByIndex(
		    _program.variableIndex(f) + f->size())) {
			_input.print(f->name);
			_input.print(":\t");
			Parser::Value v;
			valueFromVar(v, f->name);
			print(v);
			_input.println();
		}
	}
		break;
	case ARRAYS:
	{
		size_t index = _program._variablesEnd;
		for (ArrayFrame *f = _program.arrayByIndex(index);
		    _program.arrayIndex(f) < _program._arraysEnd;
		    f = _program.arrayByIndex(_program.arrayIndex(f) + f->size())) {
			_input.print(f->name);
			_input.print('(');
			_input.print(f->dimension[0]);
			for (uint8_t i = 1; i < f->numDimensions; ++i) {
				_input.print(',');
				_input.print(f->dimension[i]);
			}
			_input.print(')');
			_input.print(":\t");
			_input.println();
		}
	}
	}
}

void
Interpreter::print(const Parser::Value &v, TextAttr attr)
{
	AttrKeeper keeper(*this, attr);

	switch (v.type) {
	case Parser::Value::BOOLEAN:
		if (v.value.boolean)
			print(Token::KW_TRUE);
		else
			print(Token::KW_FALSE);
		break;
	case Parser::Value::REAL:
		this->print(v.value.real);
		break;
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
		_output.print(v.value.longInteger);
		break;
#endif
	case Parser::Value::INTEGER:
		_output.print(v.value.integer);
		break;
	case Parser::Value::STRING:
	{
		Program::StackFrame *f =
		    _program.stackFrameByIndex(_program._sp);
		if (f == NULL || f->_type != Program::StackFrame::STRING) {
			raiseError(DYNAMIC_ERROR, STRING_FRAME_SEARCH);
			return;
		}
		_output.print(f->body.string);
		_program.pop();
		break;
	}
	default:
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		break;
	}
	_output.print(' ');
}

void
Interpreter::newline()
{
	_output.println();
}

void
Interpreter::print(char v)
{
	_output.print(v);
}

void
Interpreter::print(Real number)
{
	char buf[17];
#ifdef ARDUINO
	::dtostrf(number, 12, 9, buf);
#else
	::sprintf(buf, "% .7G", number);
#endif
	print(buf);
}

void
Interpreter::print(Lexer &l)
{
	Token t = l.getToken();
	if (t <= Token::RPAREN)
		print(t);
	else {
		switch (t) {
		case Token::C_INTEGER:
		case Token::C_REAL:
		case Token::C_BOOLEAN:
			print(l.getValue(), C_CYAN); break;
		case Token::C_STRING:
		{
			AttrKeeper a(*this, C_MAGENTA);
			_output.write("\"");
			_output.print(l.id());
			_output.write("\" ");
		} break;
		case Token::REAL_IDENT:
		case Token::INTEGER_IDENT:
		case Token::BOOL_IDENT:
			print(l.id(), C_BLUE); break;
		default:
			print('?');
		}
	}
}

void
Interpreter::run()
{
	_program.reset(_program._textEnd);
	_state = EXECUTE;
}

void
Interpreter::gotoLine(const Parser::Value &l)
{
#if USE_LONGINT
	if (l.type != Parser::Value::INTEGER &&
	    l.type != Parser::Value::LONG_INTEGER) {
#else
	if (l.type != Parser::Value::INTEGER) {
#endif
		raiseError(DYNAMIC_ERROR, INTEGER_EXPRESSION_EXPECTED);
		return;
	}
	Program::String *s = _program.stringByNumber(Integer(l));
	if (s != NULL)
		_program.jump(_program.stringIndex(s));
	else
		raiseError(DYNAMIC_ERROR, NO_SUCH_STRING);
}

void
Interpreter::newProgram()
{
	_program.newProg();
}

void
Interpreter::pushReturnAddress(uint8_t textPosition)
{
	Program::StackFrame *f = _program.push(Program::StackFrame::
	    SUBPROGRAM_RETURN);
	if (f == NULL)
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
	f->body.gosubReturn.calleeIndex = _program._current;
	f->body.gosubReturn.textPosition = _program._textPosition + textPosition;
}

void
Interpreter::returnFromSub()
{
	Program::StackFrame *f = _program.stackFrameByIndex(_program._sp);
	if ((f != NULL) && (f->_type == Program::StackFrame::SUBPROGRAM_RETURN)) {
		_program.jump(f->body.gosubReturn.calleeIndex);
		_program._textPosition = f->body.gosubReturn.textPosition;
		_program.pop();
	} else
		raiseError(DYNAMIC_ERROR, RETURN_WO_GOSUB);
}

void
Interpreter::pushForLoop(const char *varName, uint8_t textPosition,
    const Parser::Value &v, const Parser::Value &vStep)
{
	Program::StackFrame *f = _program.push(Program::StackFrame::FOR_NEXT);
	if (f == NULL)
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
	f->body.forFrame.calleeIndex = _program._current;
	f->body.forFrame.textPosition = _program._textPosition + textPosition;
	f->body.forFrame.finalv = v;
	f->body.forFrame.step = vStep;

	valueFromVar(f->body.forFrame.current, varName);
	strcpy(f->body.forFrame.varName, varName);
	setVariable(varName, f->body.forFrame.current);
}

void
Interpreter::pushValue(const Parser::Value &v)
{
	Program::StackFrame *f = _program.push(Program::StackFrame::
	    VALUE);
	if (f == NULL)
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
	f->body.value = v;
}

bool
Interpreter::popValue(Parser::Value &v)
{
	Program::StackFrame *f = _program.stackFrameByIndex(_program._sp);
	if ((f != NULL) && (f->_type == Program::StackFrame::VALUE)) {
		v = f->body.value;
		_program.pop();
		return true;
	} else {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return false;
	}
}

bool
Interpreter::popString(const char *&str)
{
	Program::StackFrame *f = _program.stackFrameByIndex(_program._sp);
	if ((f != NULL) && (f->_type == Program::StackFrame::STRING)) {
		str = f->body.string;
		_program.pop();
		return true;
	} else {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return false;
	}
}

void
Interpreter::randomize()
{
	::randomSeed(millis());
}

bool
Interpreter::next(const char *varName)
{
	Program::StackFrame *f = _program.stackFrameByIndex(_program._sp);
	if ((f != NULL) && (f->_type == Program::StackFrame::FOR_NEXT) &&
	    (strcmp(f->body.forFrame.varName, varName) == 0)) {
		f->body.forFrame.current += f->body.forFrame.step;
		if (f->body.forFrame.step > Parser::Value(Integer(0))) {
			if (f->body.forFrame.current >
			    f->body.forFrame.finalv) {
				_program.pop();
				return true;
			}
		} else if (f->body.forFrame.current < f->body.forFrame.finalv) {
			_program.pop();
			return true;
		}
		_program.jump(f->body.forFrame.calleeIndex);
		_program._textPosition = f->body.forFrame.textPosition;
		setVariable(f->body.forFrame.varName, f->body.forFrame.current);
	} else
		raiseError(DYNAMIC_ERROR, INVALID_NEXT);

	return false;
}

void
Interpreter::save()
{
	/**
	 * struct SavedProgram_t
	 * {
	 *	uint16_t	length;
	 *	uint8_t		data[length];
	 *	uint16_t	crc;
	 * };
	 */
	
	// Program text buffer length
	size_t len = _program._textEnd;
	uint16_t crc = 0;
	
	EEPROMClass e;
	// First 2 bytes is program length
	e.update(0, (len << 8) >> 8);
	e.update(1, len >> 8);
	size_t p;
	for (p = 0; p < _program._textEnd; ++p) {
		e.update(p + 2, _program._text[p]);
		crc = _crc16_update(crc, _program._text[p]);
		_output.print('.');
	}
	e.update(p + 2, (crc << 8) >> 8);
	e.update(p + 3, crc >> 8);
	newline();
}

void Interpreter::load()
{
	_program.newProg();
	EEPROMClass e;
	
	uint16_t crc = 0;
	size_t len = size_t(e.read(0));
	len |= size_t(e.read(1)) << 8;
	size_t p;
	for (p = 0; p < len; ++p) {
		_program._text[p] = e.read(p + 2);
		crc = _crc16_update(crc, _program._text[p]);
		_output.print('.');
	}
	uint16_t pCrc = uint16_t(e.read(p+2));
	pCrc |= size_t(e.read(p+3)) << 8;
	if (pCrc != crc)
		newline(), raiseError(DYNAMIC_ERROR, BAD_CHECKSUM);
	newline();
	_program._textEnd = _program._variablesEnd = _program._arraysEnd = len;
}

void
Interpreter::input(const char *varName)
{
	_output.print('?');

	strcpy(_inputVarName, varName);

	//_state = VAR_INPUT;
	_program._textPosition += _lexer.getPointer();

	_inputPosition = 0;
	
	memset(_inputBuffer, 0xFF, PROGSTRINGSIZE);
	
	while (!readInput());
	doInput();
}

uint8_t
Interpreter::VariableFrame::size() const
{
	switch (type) {
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
		return sizeof (VariableFrame) + sizeof (LongInteger);
#endif
	case Parser::Value::INTEGER:
		return sizeof (VariableFrame) + sizeof (Integer);
	case Parser::Value::REAL:
		return sizeof (VariableFrame) + sizeof (float);
	case Parser::Value::BOOLEAN:
		return sizeof (VariableFrame) + sizeof (bool);
	case Parser::Value::STRING:
		return sizeof (VariableFrame) + STRINGSIZE;
	default:
		return sizeof (VariableFrame);
	}
}

void
Interpreter::set(VariableFrame &f, const Parser::Value &v)
{
	switch (f.type) {
	case VF_BOOLEAN:
	{
		union
		{
			char *b;
			bool *i;
		} _U;
		_U.b = f.bytes;
		*_U.i = bool(v);
	}
		break;
	case VF_INTEGER:
	{
		union
		{
			char *b;
			Integer *i;
		} _U;
		_U.b = f.bytes;
		*_U.i = Integer(v);
	}
		break;
#if USE_LONGINT
	case VF_LONG_INTEGER:
	{
		union
		{
			char *b;
			LongInteger *i;
		} _U;
		_U.b = f.bytes;
		*_U.i = LongInteger(v);
	}
		break;
#endif
	case VF_REAL:
	{
		union
		{
			char *b;
			Real *r;
		} _U;
		_U.b = f.bytes;
		*_U.r = Real(v);
	}
		break;
	case VF_STRING:
	{
		Program::StackFrame *fr = _program.currentStackFrame();
		if (fr == NULL || fr->_type != Program::StackFrame::STRING) {
			raiseError(DYNAMIC_ERROR, STRING_FRAME_SEARCH);
			return;
		}
		strcpy(f.bytes, fr->body.string);
		_program.pop();
	}
		break;
	default:
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
	}
}

void
Interpreter::set(ArrayFrame &f, size_t index, const Parser::Value &v)
{
	switch (f.type) {
	case VF_BOOLEAN:
	{
		union
		{
			uint8_t *b;
			bool *i;
		} _U;
		_U.b = f.data();
		_U.i[index] = bool(v);
	}
		break;
	case VF_INTEGER:
	{
		union
		{
			uint8_t *b;
			Integer *i;
		} _U;
		_U.b = f.data();
		_U.i[index] = Integer(v);
	}
		break;
#if USE_LONGINT
	case VF_LONG_INTEGER:
	{
		union
		{
			uint8_t *b;
			LongInteger *i;
		} _U;
		_U.b = f.data();
		_U.i[index] = LongInteger(v);
	}
		break;
#endif
	case VF_REAL:
	{
		union
		{
			uint8_t *b;
			Real *r;
		} _U;
		_U.b = f.data();
		_U.r[index] = Real(v);
	}
		break;
	default:
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
	};
}

bool
Interpreter::readInput()
{
	int a = _input.available();
	if (a <= 0)
		return (false);

	const uint8_t availableSize = PROGSTRINGSIZE - 1 - _inputPosition;
	a = min(a, availableSize);

	size_t read = _input.readBytes(_inputBuffer + _inputPosition, a);
	assert(read <= availableSize);
	uint8_t end = _inputPosition + read;
	for (uint8_t i = _inputPosition; i < end; ++i) {
		char c = _inputBuffer[i];
		_output.write(c);
		switch (c) {
		case char(ASCII::BS):
			if (_inputPosition > 0)
				--_inputPosition;
			break;
		case char(ASCII::CR):
			_output.write(char(ASCII::LF));
			_inputBuffer[i] = 0;
			return (true);
		default:
			++_inputPosition;
		}
	}
	return (false);
}

void
Interpreter::print(const char *text, TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(text), _output.print(' ');
}

void
Interpreter::print(ProgMemStrings index, TextAttr attr)
{
	char buf[16];
	strcpy_P(buf, (PGM_P) pgm_read_word(&(ESTRING(index))));

	AttrKeeper _a(*this, attr);

	_output.print(buf), _output.print(' ');
}

void
Interpreter::print(Token t)
{
	char buf[16];
	strcpy_P(buf, (PGM_P) pgm_read_word(&(Lexer::tokenStrings[
	    uint8_t(t)])));
	if ( t <= Token::KW_VARS)
		print(buf, TextAttr(uint8_t(BRIGHT) |
		    uint8_t(C_GREEN)));
	else
		print(buf);
}

void
Interpreter::print(Integer i, TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(i), _output.print(' ');
}

void
Interpreter::print(long i, TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(i), _output.print(' ');
}

void
Interpreter::raiseError(ErrorType type, ErrorCodes errorCode)
{
	char buf[16];
	if (type == DYNAMIC_ERROR)
		strcpy_P(buf, (PGM_P) pgm_read_word(&(ESTRING(S_DYNAMIC))));
	else // STATIC_ERROR
		strcpy_P(buf, (PGM_P) pgm_read_word(&(ESTRING(S_STATIC))));
	_output.print(buf);
	_output.print(' ');
	strcpy_P(buf, (PGM_P) pgm_read_word(&(ESTRING(S_SEMANTIC))));
	_output.print(buf);
	_output.print(' ');
	strcpy_P(buf, (PGM_P) pgm_read_word(&(ESTRING(S_ERROR))));
	_output.print(buf);
	_output.print(':');
	if (type == DYNAMIC_ERROR) {
		_output.println(uint8_t(errorCode));
	} else { // STATIC_ERROR
		_output.println(int(_parser.getError()));
	}
	_state = SHELL;
}

bool
Interpreter::arrayElementIndex(ArrayFrame *f, size_t &index)
{
	index = 0;
	size_t dim = f->numDimensions, mul = 1;
	while (dim-- > 0) {
		Program::StackFrame *sf = _program.currentStackFrame();
		if (sf == NULL ||
		    sf->_type != Program::StackFrame::ARRAY_DIMENSION ||
		    sf->body.arrayDimension > f->dimension[dim]) {
			return false;
		}
		index += mul * sf->body.arrayDimension;
		mul *= f->dimension[dim] + 1;
		_program.pop();
	};
	return (true);
}

Interpreter::VariableFrame*
Interpreter::setVariable(const char *name, const Parser::Value &v)
{
	size_t index = _program._textEnd;

	VariableFrame *f;
	for (f = _program.variableByIndex(index); f != NULL; index += f->size(),
	    f = _program.variableByIndex(index)) {
		int res = strcmp(name, f->name);
		if (res == 0) {
			set(*f, v);
			return (f);
		} else if (res < 0) {
			break;
		}
	}

	if (f == NULL)
		f = reinterpret_cast<VariableFrame*> (_program._text + index);

	size_t dist = sizeof (VariableFrame);
	Type t;
#if USE_LONGINT
	if (endsWith(name, "%%")) {
		t = VF_LONG_INTEGER;
		dist += sizeof (LongInteger);
	} else
#endif
	if (endsWith(name, '%')) {
		t = VF_INTEGER;
		dist += sizeof (Integer);
	} else if (endsWith(name, '!')) {
		t = VF_BOOLEAN;
		dist += sizeof (bool);
	} else if (endsWith(name, '$')) {
		t = VF_STRING;
		dist += STRINGSIZE;
	} else {
		t = VF_REAL;
		dist += sizeof (Real);
	}
	if (_program._arraysEnd >= _program._sp) {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return NULL;
	}
	memmove(_program._text + index + dist, _program._text + index,
	    _program._arraysEnd - index);
	f->type = t;
	strcpy(f->name, name);
	_program._variablesEnd += f->size();
	_program._arraysEnd += f->size();
	set(*f, v);

	return (f);
}

void
Interpreter::setArrayElement(const char *name, const Parser::Value &v)
{
	ArrayFrame *f = _program.arrayByName(name);
	if (f == NULL) {
		raiseError(DYNAMIC_ERROR, NO_SUCH_ARRAY);
		return;
	}

	size_t index;
	if (!arrayElementIndex(f, index)) {
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		return;
	}

	set(*f, index, v);
}

void
Interpreter::newArray(const char *name)
{
	Program::StackFrame *f = _program.stackFrameByIndex(_program._sp);
	if (f != NULL && f->_type == Program::StackFrame::ARRAY_DIMENSIONS) {
		uint8_t dimensions = f->body.arrayDimensions;
		_program.pop();
		size_t size = 1;
		size_t sp = _program._sp; // go on stack frames, containong dimesions
		for (uint8_t dim = 0; dim < dimensions; ++dim) {
			f = _program.stackFrameByIndex(sp);
			if (f != NULL && f->_type ==
			    Program::StackFrame::ARRAY_DIMENSION) {
				size *= f->body.arrayDimension + 1;
				sp += f->size(Program::StackFrame::ARRAY_DIMENSION);
			} else {
				raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
				return;
			}
		}
		ArrayFrame *array = addArray(name, dimensions, size);
		if (array != NULL) { // go on stack frames, containong dimesions once more
			// now popping
			for (uint8_t dim = dimensions; dim-- > 0;) {
				f = _program.stackFrameByIndex(_program._sp);
				array->dimension[dim] = f->body.arrayDimension;
				_program.pop();
			}
		} else
			return;
	};
	return;
}

const Interpreter::VariableFrame*
Interpreter::getVariable(const char *name)
{
	const VariableFrame *f = _program.variableByName(name);
	if (f == NULL) {
		Parser::Value v(Integer(0));
		f = setVariable(name, v);
	}
	return (f);
}

void
Interpreter::pushString(const char *str)
{
	Program::StackFrame *f = _program.push(Program::StackFrame::STRING);
	if (f == NULL) {
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
		return;
	}
	strcpy(f->body.string, str);
}

size_t
Interpreter::pushDimension(size_t dim)
{
	Program::StackFrame *f =
	    _program.push(Program::StackFrame::ARRAY_DIMENSION);
	if (f == NULL) {
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
		return 0;
	}
	f->body.arrayDimension = dim;
	return (_program._sp);
}

void
Interpreter::pushDimensions(uint8_t dim)
{
	Program::StackFrame *f =
	    _program.push(Program::StackFrame::ARRAY_DIMENSIONS);
	if (f == NULL) {
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
		return;
	}
	f->body.arrayDimensions = dim;
}

bool
Interpreter::confirm()
{
	bool result = false;
	do {
		print(S_REALLY); print('?'); newline();
		while (_input.available() <= 0);
		char c = _input.read();
		_output.write(c);
		while (_input.available() <= 0);
		if (_input.read() != int(ASCII::CR)) {
			newline();
			continue;
		}
		if (c == 'Y' || c == 'y') {
			result = true;
		}
		else if (c == 'N' || c == 'n') {
			result = false;
		} else
			continue;
		newline();
		break;
	} while (true);
	return result;
}

void
Interpreter::strConcat(Parser::Value &v1, Parser::Value &v2)
{
	Program::StackFrame *f = _program.currentStackFrame();
	if (f != NULL || f->_type == Program::StackFrame::STRING) {
		_program.pop();
		Program::StackFrame *ff = _program.currentStackFrame();
		if (ff != NULL || ff->_type == Program::StackFrame::STRING) {
			uint8_t l1 = strlen(ff->body.string);
			uint8_t l2 = strlen(f->body.string);
			if (l1 + l2 >= STRINGSIZE)
				l2 = STRINGSIZE - l1 - 1;
			strncpy(ff->body.string + l1, f->body.string, l2);
			ff->body.string[l1 + l2] = 0;
		} else
			raiseError(DYNAMIC_ERROR, STRING_FRAME_SEARCH);
	} else
		raiseError(DYNAMIC_ERROR, STRING_FRAME_SEARCH);
}

void
Interpreter::end()
{
	_state = SHELL;
}

size_t
Interpreter::ArrayFrame::size() const
{
	size_t result = sizeof (Interpreter::ArrayFrame) +
	    numDimensions * sizeof (size_t);

	size_t mul = 1;

	for (uint8_t i = 0; i < numDimensions; ++i)
		mul *= dimension[i] + 1;

	switch (type) {
	case VF_INTEGER:
		mul *= sizeof (Integer);
		break;
#if USE_LONGINT
	case VF_LONG_INTEGER:
		mul *= sizeof (LongInteger);
		break;
#endif
	case VF_REAL:
		mul *= sizeof (Real);
		break;
	case VF_BOOLEAN:
		mul *= sizeof (bool);
		break;
	default:
		mul = 1;
	}
	result += mul;

	return result;
}

Interpreter::ArrayFrame*
Interpreter::addArray(const char *name, uint8_t dim,
    uint32_t num)
{
	size_t index = _program._variablesEnd;
	ArrayFrame *f;
	for (f = _program.arrayByIndex(index); index < _program._arraysEnd;
	    index += f->size(), f = _program.arrayByIndex(index)) {
		int res = strcmp(name, f->name);
		if (res == 0) {
			raiseError(DYNAMIC_ERROR, REDIMED_ARRAY);
			return (NULL);
		} else if (res < 0)
			break;
	}

	if (f == NULL)
		f = reinterpret_cast<ArrayFrame*> (_program._text + index);

	Type t;
#if USE_LONGINT
	if (endsWith(name, "%%")) {
		t = VF_LONG_INTEGER;
		num *= sizeof (LongInteger);
	} else
#endif
	if (endsWith(name, '%')) {
		t = VF_INTEGER;
		num *= sizeof (Integer);
	} else if (endsWith(name, '!')) {
		t = VF_BOOLEAN;
		num *= sizeof (bool);
	} else { // real
		t = VF_REAL;
		num *= sizeof (Real);
	}
	size_t dist = sizeof (ArrayFrame) + sizeof (size_t) * dim + num;
	if (_program._arraysEnd + dist >= _program._sp) {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return NULL;
	}
	memmove(_program._text + index + dist, _program._text + index,
	    _program._arraysEnd - index);
	f->type = t;
	f->numDimensions = dim;
	strcpy(f->name, name);
	memset(f->data(), 0, num);
	_program._arraysEnd += dist;

	return (f);
}

}
