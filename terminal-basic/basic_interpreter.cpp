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

#include "math.hpp"
#include <string.h>
#include <assert.h>

#include <EEPROM.h>
#include <stdbool.h>

#include "helper.hpp"
#include <util/crc16.h>

#include "basic_interpreter.hpp"
#include "basic_program.hpp"
#include "basic_parser_value.hpp"
#include "arduino_logger.hpp"
#include "bytearray.hpp"
#include "version.h"
#include "ascii.hpp"

namespace BASIC
{

class Interpreter::AttrKeeper
{
public:

	explicit AttrKeeper(Interpreter &i, VT100::TextAttr a) :
	_i(i), _a(a)
	{
#if USE_TEXTATTRIBUTES
		if (_a == VT100::NO_ATTR)
			return;
		if ((uint8_t(a) & uint8_t(VT100::BRIGHT)) != uint8_t(VT100::NO_ATTR))
			_i.printEsc(ProgMemStrings::VT100_BRIGHT);
		if ((uint8_t(a) & uint8_t(VT100::UNDERSCORE)) != uint8_t(VT100::NO_ATTR))
			_i.printEsc(ProgMemStrings::VT100_UNDERSCORE);
		if ((uint8_t(a) & uint8_t(VT100::REVERSE)) != uint8_t(VT100::NO_ATTR))
			_i.printEsc(ProgMemStrings::VT100_REVERSE);
#if USE_COLORATTRIBUTES
		if ((uint8_t(a) & 0xF0) == VT100::C_YELLOW)
			_i.printEsc(ProgMemStrings::VT100_YELLOW);
		else if ((uint8_t(a) & 0xF0) == VT100::C_GREEN)
			_i.printEsc(ProgMemStrings::VT100_GREEN);
		else if ((uint8_t(a) & 0xF0) == VT100::C_RED)
			_i.printEsc(ProgMemStrings::VT100_RED);
		else if ((uint8_t(a) & 0xF0) == VT100::C_BLUE)
			_i.printEsc(ProgMemStrings::VT100_BLUE);
		else if ((uint8_t(a) & 0xF0) == VT100::C_MAGENTA)
			_i.printEsc(ProgMemStrings::VT100_MAGENTA);
		else if ((uint8_t(a) & 0xF0) == VT100::C_CYAN)
			_i.printEsc(ProgMemStrings::VT100_CYAN);
		else if ((uint8_t(a) & 0xF0) == VT100::C_WHITE)
			_i.printEsc(ProgMemStrings::VT100_WHITE);
#endif
#endif // USE_TEXTATTRIBUTES
	}

	~AttrKeeper()
	{
#if USE_TEXTATTRIBUTES
		if (_a == VT100::NO_ATTR)
			return;
		_i.printEsc(ProgMemStrings::VT100_NOATTR);
#endif
	}
private:

	Interpreter &_i;
	VT100::TextAttr _a;
};

uint8_t Interpreter::_termnoGen = 0;

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
#if USE_REALS
	case VF_REAL:
		v = f->get<Real>();
		break;
#endif
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
#if USE_REALS
	case VF_REAL:
		v.type = Parser::Value::REAL;
		v.value.real = f->get<Real>(index);
		break;
#endif
	default:
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		return false;
	}
	return true;
}

Interpreter::Interpreter(Stream &stream, Print &output, Program &program) :
_program(program), _state(SHELL), _input(stream), _output(output),
_parser(_lexer, *this), _termno(++_termnoGen)
{
	_input.setTimeout(10000L);
}

void
Interpreter::init()
{
	_parser.init();
	_program.newProg();

	print(ProgMemStrings::TERMINAL, VT100::BRIGHT);
	print(ProgMemStrings::S_TERMINAL_BASIC, VT100::BRIGHT);
	print(ProgMemStrings::S_VERSION);
	print(VERSION, VT100::BRIGHT), newline();
#if BASIC_MULTITERMINAL
	print(ProgMemStrings::TERMINAL, NO_ATTR), print(Integer(_termno), BRIGHT),
	    _output.print(':'), _output.print(' ');
#endif
	print(long(_program.programSize - _program._arraysEnd), VT100::BRIGHT);
	print(ProgMemStrings::BYTES), print(ProgMemStrings::AVAILABLE), newline();
	_state = SHELL;
}

void
Interpreter::step()
{
	LOG_TRACE;

	char c;

	switch (_state) {
		// waiting for user input command or program line
	case SHELL:
	{
		print(ProgMemStrings::S_READY, VT100::BRIGHT);
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
		if (nextInput()) {
			_output.print('?');
			_inputPosition = 0;
			memset(_inputBuffer, 0xFF, PROGSTRINGSIZE);
			_state = GET_VAR_VALUE;
		} else
			_state = EXECUTE;
		break;
	case GET_VAR_VALUE:
		if (readInput()) {
			doInput();
			_state = VAR_INPUT;
		}
		break;
	case EXECUTE:
		c = char(ASCII::NUL);
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
	default:
		break;
	}
}

void
Interpreter::exec()
{
	_lexer.init(_inputBuffer);
	if (_lexer.getNext() && (_lexer.getToken() == Token::C_INTEGER)) {
		Integer pLine = Integer(_lexer.getValue());
		uint8_t position = _lexer.getPointer();
		_lexer.getNext();
		if (_lexer.getToken() != Token::NOTOKENS) {
			if (!_program.addLine(pLine, _inputBuffer + position)) {
				raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
				_state = SHELL;
			} else
				_state = PROGRAM_INPUT;
		} else {
			_program.removeLine(pLine);
			_state = PROGRAM_INPUT;
		}
	} else {
		_state = SHELL;
		if (!_parser.parse(_inputBuffer))
			raiseError(STATIC_ERROR);
	}
}

void
Interpreter::cls()
{
	printEsc(ProgMemStrings::VT100_CLS),
	printEsc("H");
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

		print(long(s->number), VT100::C_YELLOW);

		Lexer lex;
		lex.init(s->text);
		while (lex.getNext()) {
			print(lex);
			if (lex.getToken() == Token::KW_REM) {
				print(s->text + lex.getPointer());
				break;
			}
		}
		newline();
	}
}

void
Interpreter::addModule(FunctionBlock *module)
{
	_parser.addModule(module);
}

#if USE_DUMP

void
Interpreter::dump(DumpMode mode)
{
	switch (mode) {
	case MEMORY:
	{
		ByteArray ba((uint8_t*) _program._text, _program.programSize);
		_output.println(ba);
		print(ProgMemStrings::S_END), print(ProgMemStrings::S_OF);
		print(ProgMemStrings::S_TEXT), _output.print('\t');
		_output.println(unsigned(_program._textEnd), HEX);
		print(ProgMemStrings::S_END), print(ProgMemStrings::S_OF);
		print(ProgMemStrings::S_VARS), _output.print('\t');
		_output.println(unsigned(_program._variablesEnd), HEX);
		print(ProgMemStrings::S_END), print(ProgMemStrings::S_OF);
		print(ProgMemStrings::S_ARRAYS), _output.print('\t');
		_output.println(unsigned(_program._arraysEnd), HEX);
		print(ProgMemStrings::S_STACK), _output.print('\t');
		_output.println(unsigned(_program._sp), HEX);
	}
		break;
	case VARS:
	{
		size_t index = _program._textEnd;
		for (VariableFrame *f = _program.variableByIndex(index);
		    (f != NULL) && (_program.variableIndex(f) <
		    _program._variablesEnd); f = _program.variableByIndex(
		    _program.variableIndex(f) + f->size())) {
			_output.print(f->name);
			_output.print(":\t");
			Parser::Value v;
			valueFromVar(v, f->name);
			print(v);
			_output.println();
		}
	}
		break;
	case ARRAYS:
	{
		size_t index = _program._variablesEnd;
		for (ArrayFrame *f = _program.arrayByIndex(index);
		    _program.arrayIndex(f) < _program._arraysEnd;
		    f = _program.arrayByIndex(_program.arrayIndex(f) + f->size())) {
			_output.print(f->name);
			_output.print('(');
			_output.print(f->dimension[0]);
			for (uint8_t i = 1; i < f->numDimensions; ++i) {
				_output.print(',');
				_output.print(f->dimension[i]);
			}
			_output.print(')');
			_output.print(":\t");
			_output.println();
		}
	}
		break;
	}
}
#endif

void
Interpreter::print(const Parser::Value &v, VT100::TextAttr attr)
{
	AttrKeeper keeper(*this, attr);

	switch (v.type) {
	case Parser::Value::BOOLEAN:
#if USE_REALS
	case Parser::Value::REAL:
#endif
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
#endif
	case Parser::Value::INTEGER:
		_output.print(v);
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
			print(l.getValue(), VT100::C_CYAN);
			break;
		case Token::C_STRING:
		{
			AttrKeeper a(*this, VT100::C_MAGENTA);
			_output.write(uint8_t(ASCII::QUMARK));
			_output.print(l.id());
			_output.write(uint8_t(ASCII::QUMARK));
		}
			break;
		case Token::REAL_IDENT:
		case Token::INTEGER_IDENT:
#if USE_LONGINT
		case Token::LONGINT_IDENT:
#endif
		case Token::BOOL_IDENT:
		case Token::STRING_IDENT:
			print(l.id(), VT100::C_BLUE);
			break;
		default:
			_output.print('?');
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
	if (l.type != Parser::Value::INTEGER
#if USE_LONGINT
	&& l.type != Parser::Value::LONG_INTEGER
#endif
	) {
		raiseError(DYNAMIC_ERROR, INTEGER_EXPRESSION_EXPECTED);
		return;
	}
	Program::String *s = _program.lineByNumber(Integer(l));
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
	if (f != NULL) {
		f->body.gosubReturn.calleeIndex = _program._current;
		f->body.gosubReturn.textPosition = _program._textPosition +
		    textPosition;
	} else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
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
	if (f != NULL) {
		f->body.forFrame.calleeIndex = _program._current;
		f->body.forFrame.textPosition = _program._textPosition +
		    textPosition;
		f->body.forFrame.finalvalue = v;
		f->body.forFrame.stepValue = vStep;

		valueFromVar(f->body.forFrame.currentValue, varName);
		strcpy(f->body.forFrame.varName, varName);
		setVariable(varName, f->body.forFrame.currentValue);
	} else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
}

void
Interpreter::pushValue(const Parser::Value &v)
{
	Program::StackFrame *f = _program.push(Program::StackFrame::
	    VALUE);
	if (f != NULL)
		f->body.value = v;
	else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
}

void
Interpreter::pushInputObject(const char *varName)
{
	Program::StackFrame *f = _program.push(Program::StackFrame::
	    INPUT_OBJECT);
	if (f != NULL)
		strcpy(f->body.inputObject.name, varName);
	else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
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
	    (strcmp(f->body.forFrame.varName, varName) == 0)) { // Correct frame
		f->body.forFrame.currentValue += f->body.forFrame.stepValue;
		if (f->body.forFrame.stepValue > Parser::Value(Integer(0))) {
			if (f->body.forFrame.currentValue >
			    f->body.forFrame.finalvalue) {
				_program.pop();
				return true;
			}
		} else if (f->body.forFrame.currentValue < f->body.forFrame.finalvalue) {
			_program.pop();
			return true;
		}
		_program.jump(f->body.forFrame.calleeIndex);
		_program._textPosition = f->body.forFrame.textPosition;
		setVariable(f->body.forFrame.varName, f->body.forFrame.currentValue);
	} else // Incorrect frame
		raiseError(DYNAMIC_ERROR, INVALID_NEXT);

	return (false);
}

#if USE_SAVE_LOAD
void
Interpreter::save()
{
	EEpromHeader_t h = {
		// Program text buffer length
		.len = _program._textEnd,
		.magic_FFFFminuslen = uint16_t(0xFFFFu) - _program._textEnd,
		// Checksum
		.crc16 = 0
	};
#if SAVE_LOAD_CHECKSUM
	// Compute program checksum
	for (size_t p = 0; p < _program._textEnd; ++p)
		h.crc16 = _crc16_update(h.crc16, _program._text[p]);
#endif
	{
		EEPROMClass e;
		// Write program to EEPROM
		for (size_t p = 0; p < _program._textEnd; ++p) {
			e.update(p + sizeof (EEpromHeader_t), _program._text[p]);
			_output.print('.');
		}
	}
	newline();
#if SAVE_LOAD_CHECKSUM
	// Compute checksum
	uint16_t crc = eepromProgramChecksum(h.len);

	if (crc == h.crc16) {
#endif
		EEPROMClass e;
		e.put(0, h);
#if SAVE_LOAD_CHECKSUM
	} else
		raiseError(DYNAMIC_ERROR, BAD_CHECKSUM);
#endif
}

void
Interpreter::load()
{
	uint16_t len;
	_program.newProg();
	if (!checkText(len))
		return;

	loadText(len);
	_program._textEnd = _program._variablesEnd = _program._arraysEnd = len;
}

void
Interpreter::chain()
{
	uint16_t len;
	if (!checkText(len))
		return;

	_program.clearProg();
	_program.moveData(len);
	// Load programm memory without progress
	loadText(len, false);
	_program.jump(0);
	_parser.stop();
}

#if SAVE_LOAD_CHECKSUM

uint16_t
Interpreter::eepromProgramChecksum(uint16_t len)
{
	EEPROMClass e;
	// Compute checksum
	uint16_t crc = 0, p;
	for (p = sizeof (EEpromHeader_t); p < len + sizeof (EEpromHeader_t);
	    ++p) {
		uint8_t b = e.read(p);
		crc = _crc16_update(crc, b);
		_output.print('.');
	}
	newline();
	return crc;
}
#endif // SAVE_LOAD_CHECKSUM

bool
Interpreter::checkText(uint16_t &len)
{
	EEpromHeader_t h;

	{
		EEPROMClass e;
		e.get(0, h);
	}

	if ((h.len > PROGRAMSIZE) ||
	    (h.magic_FFFFminuslen != uint16_t(0xFFFF) - h.len)) {
		raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
		return false;
	}
#if SAVE_LOAD_CHECKSUM
	uint16_t crc = eepromProgramChecksum(h.len);
	if (h.crc16 != crc) {
		raiseError(DYNAMIC_ERROR, BAD_CHECKSUM);
		return false;
	}
#endif
	len = h.len;
	return true;
}

void
Interpreter::loadText(uint16_t len, bool showProgress)
{
	EEPROMClass e;

	for (uint16_t p = 0; p < len; ++p) {
		_program._text[p] = e.read(p + sizeof (EEpromHeader_t));
		if (showProgress)
			_output.print('.');
	}
	newline();
}
#endif // USE_SAVE_LOAD

void
Interpreter::input()
{
	_program.reverseLast(Program::StackFrame::INPUT_OBJECT);
	_state = VAR_INPUT;
}

bool
Interpreter::nextInput()
{
	const Program::StackFrame *f = _program.currentStackFrame();
	if (f != NULL && f->_type == Program::StackFrame::INPUT_OBJECT) {
		_program.pop();
		strcpy(_inputVarName, f->body.inputObject.name);
		return (true);
	} else
		return (false);
}

void
Interpreter::doInput()
{
	Lexer l;
	l.init(_inputBuffer);
	Parser::Value v(Integer(0));
	bool neg = false;

	do {
		if (l.getNext()) {
			switch (l.getToken()) {
			case Token::MINUS:
				neg = !neg;
				continue;
			case Token::PLUS:
				continue;
			case Token::C_INTEGER:
			case Token::C_REAL:
				v = l.getValue();
				break;
			case Token::C_STRING:
			case Token::REAL_IDENT:
			case Token::INTEGER_IDENT:
#if USE_LONGINT
			case Token::LONGINT_IDENT:
#endif
			case Token::STRING_IDENT:
			case Token::BOOL_IDENT:
			{
				v = l.getValue();
				pushString(l.id());
			}
				break;
			default:
				raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
				return;
			}
		}
		if (neg)
			v = -v;
		setVariable(_inputVarName, v);
		if (l.getNext()) {
			if (l.getToken() == Token::COMMA)
				if (!nextInput())
					break;
			neg = false;
		} else
			break;
	} while (true);
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
#if USE_REALS
	case Parser::Value::REAL:
		return sizeof (VariableFrame) + sizeof (Real);
#endif
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
#if USE_REALS
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
#endif
	case VF_STRING:
	{
		Program::StackFrame *fr = _program.currentStackFrame();
		if (fr == NULL || fr->_type != Program::StackFrame::STRING) {
			f.bytes[0] = 0;
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
#if USE_REALS
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
#endif
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
		switch (c) {
		case char(ASCII::BS):
		case char(ASCII::DEL):
			if (_inputPosition > 0) {
				--_inputPosition;
				_output.write(char(ASCII::BS));
				_output.write(char(ASCII::SPACE));
				_output.write(char(ASCII::BS));
			}
			break;
		case char(ASCII::CR):
			_output.println();
			_inputBuffer[i] = 0;
			return (true);
		default:
			// Only acept character if there is room for upcoming
			// control one (line end or del/bs)
			if (availableSize > 1) {
				++_inputPosition;
				_output.write(c);
			}
		}
	}
	return (false);
}

void
Interpreter::print(const char *text, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(text), _output.print(' ');
}

void
Interpreter::print(ProgMemStrings index, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	write(index), _output.print(' ');
}

void
Interpreter::write(ProgMemStrings index)
{
	char buf[16];
	strcpy_P(buf, progmemString(index));

	_output.print(buf);
}

void
Interpreter::print(Token t)
{
	char buf[16];
	strcpy_P(buf, (PGM_P) pgm_read_word(&(Lexer::tokenStrings[
	    uint8_t(t)])));
	if (t < Token::STAR)
		print(buf, VT100::TextAttr(uint8_t(VT100::BRIGHT) |
	    uint8_t(VT100::C_GREEN)));
	else
		print(buf);
}

void
Interpreter::print(Integer i, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(i), _output.print(' ');
}

void
Interpreter::printEsc(const char *str)
{
	write(ProgMemStrings::VT100_ESCSEQ), _output.print(str);
}

void
Interpreter::printEsc(ProgMemStrings index)
{
	write(ProgMemStrings::VT100_ESCSEQ);
	write(index);
}

void
Interpreter::printTab(const Parser::Value &v)
{
	Integer tabs;
#if USE_REALS
	if (v.type == Parser::Value::REAL)
		tabs = math<Real>::round(v.value.real);
	else
#endif
		tabs = Integer(v);
	if (tabs > 0)
		write(ProgMemStrings::VT100_ESCSEQ), _output.print(tabs - 1),
		    _output.print('C');
	else
		raiseError(DYNAMIC_ERROR, INVALID_TAB_VALUE, false);
}

void
Interpreter::print(long i, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(i), _output.print(' ');
}

void
Interpreter::raiseError(ErrorType type, ErrorCodes errorCode, bool fatal)
{
	// Output Program line number if running program
	if ((_state == EXECUTE) && (_program.current() != NULL))
		print(long(_program.current()->number), VT100::C_YELLOW);
	_output.print(':');
	if (type == DYNAMIC_ERROR)
		print(ProgMemStrings::S_DYNAMIC);
	else // STATIC_ERROR
		print(ProgMemStrings::S_STATIC);
	print(ProgMemStrings::S_SEMANTIC);
	print(ProgMemStrings::S_ERROR, VT100::C_RED);
	if (type == DYNAMIC_ERROR)
		print(Integer(errorCode));
	else // STATIC_ERROR
		print(Integer(_parser.getError()));
	newline();
	
	if (fatal)
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

Interpreter::VariableFrame *
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
		} else if (res < 0)
			break;
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
#if USE_REALS
		t = VF_REAL;
		dist += sizeof (Real);
#else
		t = VF_INTEGER;
		dist += sizeof (Integer);
#endif
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
		}
	}
}

const Interpreter::VariableFrame *
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

uint16_t
Interpreter::pushDimension(uint16_t dim)
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
	if (f != NULL)
		f->body.arrayDimensions = dim;
	else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
}

bool
Interpreter::confirm()
{
	bool result = false;
	do {
		print(ProgMemStrings::S_REALLY);
		print('?');
		newline();
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
		} else if (c == 'N' || c == 'n') {
			result = false;
		} else
			continue;
		newline();
		break;
	} while (true);
	return (result);
}

#if USE_STRINGOPS
void
Interpreter::strConcat()
{
	const Program::StackFrame *f = _program.currentStackFrame();
	if (f != NULL && f->_type == Program::StackFrame::STRING) {
		_program.pop();
		Program::StackFrame *ff = _program.currentStackFrame();
		if (ff != NULL || ff->_type == Program::StackFrame::STRING) {
			uint8_t l1 = strlen(ff->body.string);
			uint8_t l2 = strlen(f->body.string);
			if (l1 + l2 >= STRINGSIZE)
				l2 = STRINGSIZE - l1 - 1;
			strncpy(ff->body.string + l1, f->body.string, l2);
			ff->body.string[l1 + l2] = 0;
			return;
		}
	}
	raiseError(DYNAMIC_ERROR, STRING_FRAME_SEARCH);
}

bool
Interpreter::strCmp()
{
	const Program::StackFrame *f = _program.currentStackFrame();
	if (f != NULL && f->_type == Program::StackFrame::STRING) {
		_program.pop();
		Program::StackFrame *ff = _program.currentStackFrame();
		if (ff != NULL || ff->_type == Program::StackFrame::STRING) {
			_program.pop();
			return strncmp(ff->body.string, f->body.string,
			    STRINGSIZE) == 0;
		}
	}
	raiseError(DYNAMIC_ERROR, STRING_FRAME_SEARCH);
	return false;
}
#endif // USE_STRINGOPS

void
Interpreter::end()
{
	_state = SHELL;
}

uint16_t
Interpreter::ArrayFrame::size() const
{
	uint16_t result = sizeof (Interpreter::ArrayFrame) +
	    numDimensions * sizeof (uint16_t);

	uint16_t mul = 1;

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
#if USE_REALS
	case VF_REAL:
		mul *= sizeof (Real);
		break;
#endif
	case VF_BOOLEAN:
		mul *= sizeof (bool);
		break;
	default:
		mul = 1;
	}
	result += mul;

	return (result);
}

Interpreter::ArrayFrame *
Interpreter::addArray(const char *name, uint8_t dim,
    uint32_t num)
{
	uint16_t index = _program._variablesEnd;
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
#if USE_REALS
		t = VF_REAL;
		num *= sizeof (Real);
#else  // Integer
		t = VF_INTEGER;
		num *= sizeof (Integer);
#endif
	}

	const uint16_t dist = sizeof (ArrayFrame) + sizeof (size_t) * dim + num;
	if (_program._arraysEnd + dist >= _program._sp) {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return (NULL);
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
