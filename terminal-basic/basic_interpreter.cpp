/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2017-2019 Andrey V. Skvortsov <starling13@mail.ru>
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

#include <string.h>
#include <assert.h>

#include <stdbool.h>
#include <ctype.h>

#include "basic.hpp"

#if USE_SAVE_LOAD
#include <EEPROM.h>
#if SAVE_LOAD_CHECKSUM
#include <util/crc16.h>
#include <stdint.h>
#endif
#endif // USE_SAVE_LOAD

#include "helper.hpp"
#include "math.hpp"
#include "basic_interpreter.hpp"
#include "basic_program.hpp"
#include "basic_parser_value.hpp"
#include "arduino_logger.hpp"
#include "bytearray.hpp"
#include "version.h"
#include "ascii.hpp"
#if USE_DATA
#include "basic_dataparser.hpp"
#endif

#if USESD
#include "basic_sdfs.hpp"
extern BASIC::SDFSModule sdfs;
#endif

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

void
Interpreter::valueFromVar(Parser::Value &v, const char *varName)
{
	const auto f = getVariable(varName);
	if (f == nullptr)
		return;
	switch (f->type) {
	case Parser::Value::INTEGER:
		v = f->get<Integer>();
		break;
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
		v = f->get<LongInteger>();
		break;
#endif
#if USE_REALS
	case Parser::Value::REAL:
		v = f->get<Real>();
		break;
#if USE_LONG_REALS
	case Parser::Value::LONG_REAL:
		v = f->get<LongReal>();
		break;
#endif
#endif // USE_REALS
	case Parser::Value::LOGICAL:
		v = f->get<bool>();
		break;
	case Parser::Value::STRING:
	{
		v.setType(Parser::Value::STRING);
		auto fr = _program.push(Program::StackFrame::STRING);
		if (fr == nullptr) {
			raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
			return;
		}
		strcpy(fr->body.string, f->bytes);
	}
		break;
	}
}

bool
Interpreter::valueFromArray(Parser::Value &v, const char *name)
{
	const auto f = _program.arrayByName(name);
	if (f == nullptr) {
		raiseError(DYNAMIC_ERROR, NO_SUCH_ARRAY);
		return false;
	}

	uint16_t index;
	if (!arrayElementIndex(f, index)) {
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		return false;
	}

	if (!f->get(index, v)) {
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		return false;
	}
	return true;
}

#if BASIC_MULTITERMINAL
uint8_t Interpreter::_termnoGen = 0;
#endif

Interpreter::Interpreter(Stream &stream, Print &output, Pointer progSize) :
_program(progSize), _state(SHELL), _input(stream), _output(output),
_parser(_lexer, *this)
#if BASIC_MULTITERMINAL
,_termno(++_termnoGen)
#endif
#if USE_DATA
,_dataParserContinue(false)
#endif
{
	_input.setTimeout(10000L);
}

void
Interpreter::init()
{
	_parser.init();
	_program.newProg();
#if USE_TEXTATTRIBUTES
        cls();
#if SET_PRINTZNES
	printEsc(ProgMemStrings::VT100_CLEARZONES);
	char buf[PRINT_ZONE_WIDTH];
	memset(buf, ' ', PRINT_ZONE_WIDTH-1);
	buf[PRINT_ZONE_WIDTH-1] = '\0';
	for (uint8_t i=0; i<PRINT_ZONES_NUMBER; ++i) {
		print(buf);
		writePgm(ProgMemStrings::VT100_SETZONE);
	}
	cls();
#endif // SET_PRINTZNES
#endif // USE_TEXTATTRIBUTES
	print(ProgMemStrings::TERMINAL, VT100::BRIGHT);
	print(ProgMemStrings::S_TERMINAL_BASIC, VT100::BRIGHT);
	newline();
	print(ProgMemStrings::S_VERSION);
	print(VERSION, VT100::BRIGHT), newline();
#if BASIC_MULTITERMINAL
	print(ProgMemStrings::TERMINAL, VT100::NO_ATTR),
	    print(Integer(_termno), VT100::BRIGHT),
	    _output.print(':'), _output.print(' ');
#endif // BASIC_MULTITERMINAL
	print(long(_program.programSize - _program._arraysEnd), VT100::BRIGHT);
	print(ProgMemStrings::S_BYTES), print(ProgMemStrings::AVAILABLE), newline();
	_state = SHELL;
#if USESD
	sdfs.loadAutorun(*this);
#endif
}

void
Interpreter::step()
{
	LOG_TRACE;

	char c;

	switch (_state) {
#if USE_DELAY
	case DELAY:
		c = char(ASCII::NUL);
		if (_input.available() > 0) {
			c = _input.read();
			_inputBuffer[0] = c;
			if (c == char(ASCII::EOT))
				_state = SHELL;
		}
		if (millis() >= _delayTimeout)
			_state = _lastState;
		break;
#endif // USE_DELAY
	// waiting for user input command or program line
	case SHELL:
	{
		print(ProgMemStrings::S_READY, VT100::BRIGHT);
#if CLI_PROMPT_NEWLINE
		newline();
#endif // CLI_PROMPT_NEWLINE
	}
#if BASIC_MULTITERMINAL
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
#else
		// fall through
		// waiting for user input next program line
	case PROGRAM_INPUT:
		//_state = COLLECT_INPUT;
		_inputPosition = 0;
		memset(_inputBuffer, 0xFF, PROGSTRINGSIZE);
		while (!readInput());
		exec();
		break;
	case VAR_INPUT:
		while (nextInput()) {
			_output.print('?');
			_inputPosition = 0;
			memset(_inputBuffer, 0xFF, PROGSTRINGSIZE);
			while (!readInput());
			doInput();
		}
		_state = EXECUTE;
		break;
#endif // BASIC_MULTITERMINAL
	case EXECUTE: {
		c = char(ASCII::NUL);
		if (_input.available() > 0) {
			c = _input.read();
#if USE_INKEY
			_inputBuffer[0] = c;
#endif // USE_GET
		}
		Program::Line *s = _program.current(_program._current);
		if (s != nullptr && c != char(ASCII::EOT)) {
			bool res;
			if (!_parser.parse(s->text + _program._current.position,
			    res, true))
				_program.getNextLine();
			else
				_program._current.position += _lexer.getPointer();
			if (!res)
				raiseError(STATIC_ERROR);
		} else
			_state = SHELL;
	}
	// Fall through
	default:
		break;
	}
}

void
Interpreter::exec()
{
	_lexer.init(_inputBuffer, false);
	if (_inputPosition == 0 && _lexer.getNext() &&
	    (_lexer.getToken() == Token::C_INTEGER)) {
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
		bool res;
		while (_parser.parse(_inputBuffer+_inputPosition, res, false)) {
			if (!res) {
				raiseError(STATIC_ERROR);
				break;
			}
			_inputPosition += _lexer.getPointer();
		}
		if (_state == PROGRAM_INPUT)
			_state = SHELL;
#if BASIC_MULTITERMINAL
		if (_state == COLLECT_INPUT)
			_state = SHELL;
#endif // BASIC_MULTITERMINAL
	}
}

#if USE_DATA
void
Interpreter::restore()
{
	_program._dataCurrent.index = _program._dataCurrent.position = 0;
	_dataParserContinue = false;
}
#endif // USE_DATA

#if USESTOPCONT
void
Interpreter::cont()
{
	_state = EXECUTE;
}
#endif // USESTOPCONT

#if USE_TEXTATTRIBUTES
void
Interpreter::cls()
{
	printEsc(ProgMemStrings::VT100_CLS),
	printEsc("H");
}
#endif // USE_TEXTATTRIBUTES

void
Interpreter::list(uint16_t start, uint16_t stop)
{
#if LINE_NUM_INDENT
	uint8_t order = 0;
	_program.reset();
	for (auto s = _program.getNextLine(); s != nullptr;
	    s = _program.getNextLine()) {
		// Output onlyselected lines subrange
		if (s->number < start)
			continue;
		if (stop > 0 && s->number > stop)
			break;

		if (s->number > 9999)
			order = 4;
		else if (s->number > 999)
			order = 3;
		else if (s->number > 99)
			order = 2;
		else if (s->number > 9)
			order = 1;
	}
#endif // LINE_NUM_INDENT
	
	_program.reset();
#if LOOP_INDENT
	_loopIndent = 0;
#endif
	for (auto s = _program.getNextLine(); s != nullptr;
	    s = _program.getNextLine()) {
		// Output onlyselected lines subrange
		if (s->number < start)
			continue;
		if (stop > 0 && s->number > stop)
			break;

		// Output line number
#if LINE_NUM_INDENT
		uint8_t indent;
		if (s->number > 9999)
			indent = order - 4;
		else if (s->number > 999)
			indent = order - 3;
		else if (s->number > 99)
			indent = order - 2;
		else if (s->number > 9)
			indent = order - 1;
		else
			indent = order;
		
		while (indent-- > 0)
			_output.print(char(ASCII::SPACE));
#endif // LINE_NUM_INDENT
		print(long(s->number), VT100::C_YELLOW);
		
		Lexer lex;
#if LOOP_INDENT
                lex.init(s->text, true);
		int8_t diff = 0;
                while (lex.getNext()) {
			if (lex.getToken() == Token::KW_REM)
				break;
			else if (lex.getToken() == Token::KW_FOR)
				++diff;
			else if (lex.getToken() == Token::KW_NEXT)
				--diff;
		}
		if (diff < 0) {
			if (_loopIndent > -diff)
				_loopIndent += diff;
			else
				_loopIndent = 0;
		}
		//If enabled, do indention of the operators string
		for (uint8_t i=0; i<_loopIndent; ++i) {
			_output.print(char(ASCII::SPACE)),
			_output.print(char(ASCII::SPACE));
		}
		if (diff > 0)
			_loopIndent += diff;
#endif // LOOP_INDENT
		lex.init(s->text, true);
		while (lex.getNext()) {
			print(lex);
			if (lex.getToken() == Token::KW_REM) {
				print((char*)s->text + lex.getPointer());
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

#if USE_INKEY
uint8_t
Interpreter::lastKey()
{
	const uint8_t c = _inputBuffer[0];
	_inputBuffer[0] = 0;
	return c;
}
#endif // USE_GET

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
		auto index = _program._textEnd;
		for (auto f = _program.variableByIndex(index);
		    (f != nullptr) && (_program.objectIndex(f) <
		    _program._variablesEnd); f = _program.variableByIndex(
		    _program.objectIndex(f) + f->size())) {
#if USE_DEFFN
			if (f->type & TYPE_DEFFN)
				continue;
#endif
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
		auto index = _program._variablesEnd;
		ArrayFrame* f;
		while (f = _program.arrayByIndex(index)) {
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
			
			index += f->size();
		}
	}
		break;
	}
}
#endif // USE_DUMP

void
Interpreter::print(const Parser::Value &v, VT100::TextAttr attr)
{
	AttrKeeper keeper(*this, attr);

	switch (v.type()) {
	case Parser::Value::LOGICAL:
#if USE_REALS
	case Parser::Value::REAL:
#if USE_LONG_REALS
	case Parser::Value::LONG_REAL:
#endif
#endif // USE_REALS
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
#endif
	case Parser::Value::INTEGER:
		v.printTo(_output), _output.write(' ');
		break;
	case Parser::Value::STRING:
	{
		Program::StackFrame *f =
		    _program.stackFrameByIndex(_program._sp);
		if (f == nullptr || f->_type != Program::StackFrame::STRING) {
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

#if USE_DELAY
void
Interpreter::delay(uint16_t ms)
{
	_delayTimeout = millis() + ms;
	_lastState = _state == EXECUTE ? EXECUTE : SHELL;
	_state = DELAY;
}
#endif // USE_DELAY

#if USE_TEXTATTRIBUTES
void
Interpreter::locate(Integer x, Integer y)
{
	writePgm(ProgMemStrings::VT100_ESCSEQ), _output.print(x),
	    _output.print(char(ASCII::SEMICOLON)), _output.print(y),
	    _output.print('f');
}
#endif // USE_TEXTATTRIBUTES

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
Interpreter::execCommand(FunctionBlock::command c)
{
	if (!(*c)(*this))
		raiseError(DYNAMIC_ERROR, COMMAND_FAILED);
}

#if USE_PEEK_POKE
void
Interpreter::poke(Pointer a, Integer d)
{
	*((volatile uint8_t*)(a)) = d;
}
#endif // USE_PEEK_POKE

void
Interpreter::print(Lexer &l)
{
	Token t = l.getToken();
	if (t <= Token::RPAREN)
		print(t);
	else {
#if OPT == OPT_SPEED
		switch (t) {
		case Token::C_INTEGER:
		case Token::C_REAL:
		case Token::C_BOOLEAN:
			print(l.getValue(), VT100::C_CYAN);
			_output.print(char(ASCII::SPACE));
			break;
		case Token::C_STRING:
		{
			AttrKeeper a(*this, VT100::C_MAGENTA);
			_output.print(char(ASCII::QUMARK));
			_output.print(l.id());
			_output.print(char(ASCII::QUMARK));
		}
			break;
		case Token::REAL_IDENT:
		case Token::INTEGER_IDENT:
#if USE_LONGINT
		case Token::LONGINT_IDENT:
#endif
		case Token::STRING_IDENT:
		case Token::BOOL_IDENT:
			print(l.id(), VT100::C_BLUE);
			break;
		default:
			_output.print('?');
		}
#else
		if (t >= Token::C_INTEGER && t <= Token::C_BOOLEAN)
			print(l.getValue(), VT100::C_CYAN);
		else if (t == Token::C_STRING) {
			AttrKeeper a(*this, VT100::C_MAGENTA);
			_output.print(char(ASCII::QUMARK));
			_output.print(l.id());
			_output.print(char(ASCII::QUMARK));
                        _output.print(char(ASCII::SPACE));
		} else if (t >= Token::INTEGER_IDENT && t <= Token::BOOL_IDENT)
			print(l.id(), VT100::C_BLUE);
		else
			_output.print(char(ASCII::QMARK));
#endif
	}
}

void
Interpreter::run()
{
	_program.reset(_program._textEnd);
	_state = EXECUTE;
#if USE_INKEY
	_inputBuffer[0] = 0;
#endif
#if USE_DATA
	_dataParserContinue = false;
#endif
}

void
Interpreter::gotoLine(const Parser::Value &l)
{
	if (l.type() != Parser::Value::INTEGER
#if USE_LONGINT
	&& l.type() != Parser::Value::LONG_INTEGER
#endif
	) {
		raiseError(DYNAMIC_ERROR, INTEGER_EXPRESSION_EXPECTED);
		return;
	}
	Program::Line *s = _program.lineByNumber(Integer(l));
	if (s != nullptr)
		_program.jump(_program.objectIndex(s));
	else
		raiseError(DYNAMIC_ERROR, NO_SUCH_STRING);
}

void
Interpreter::newProgram()
{
	_program.newProg();
}

void
Interpreter::pushReturnAddress()
{
	const uint8_t textPosition = _lexer.getPointer();
	auto f = _program.push(Program::StackFrame::SUBPROGRAM_RETURN);
	if (f != nullptr) {
		f->body.gosubReturn.calleeIndex = _program._current.index;
		f->body.gosubReturn.textPosition = _program._current.position +
		    textPosition;
	} else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
}

void
Interpreter::returnFromSub()
{
	while (true) {
		const auto f = _program.currentStackFrame();
		if (f == nullptr)
			break;
		if (f->_type == Program::StackFrame::SUBPROGRAM_RETURN) {
			_program.jump(f->body.gosubReturn.calleeIndex);
			_program._current.position = f->body.gosubReturn.textPosition;
			_program.pop();
			return;
		}
		_program.pop();
	}
	raiseError(DYNAMIC_ERROR, RETURN_WO_GOSUB);
}

Program::StackFrame*
Interpreter::pushForLoop(const char *varName, uint8_t textPosition,
    const Parser::Value &v, const Parser::Value &vStep)
{
	// push new FOR .. NEXT stack frame
	auto f = _program.push(Program::StackFrame::FOR_NEXT);
	if (f != nullptr) {
	    	auto &fBody = f->body.forFrame;
		fBody.calleeIndex = _program._current.index;
		fBody.textPosition = _program._current.position +
		    textPosition;
		fBody.finalvalue = v;
		fBody.stepValue = vStep;
		strcpy(fBody.varName, varName);
	} else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
	
	return f;
}

bool
Interpreter::pushValue(const Parser::Value &v)
{
	auto f = _program.push(Program::StackFrame::VALUE);
	if (f != nullptr) {
		f->body.value = v;
		return true;
	} else {
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
		return false;
	}
}

void
Interpreter::pushInputObject(const char *varName)
{
	auto f = _program.push(Program::StackFrame::INPUT_OBJECT);
	if (f != nullptr)
		strcpy(f->body.inputObject.name, varName);
	else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
}

bool
Interpreter::popValue(Parser::Value &v)
{
	const auto f = _program.currentStackFrame();
	if ((f != nullptr) && (f->_type == Program::StackFrame::VALUE)) {
		v = f->body.value;
		_program.pop();
		return true;
	} else {
//		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return false;
	}
}

bool
Interpreter::popString(const char *&str)
{
	const auto f = _program.currentStackFrame();
	if ((f != nullptr) && (f->_type == Program::StackFrame::STRING)) {
		str = f->body.string;
		_program.pop();
		return true;
	} else {
		raiseError(DYNAMIC_ERROR, STRING_FRAME_SEARCH);
		return false;
	}
}

void
Interpreter::randomize()
{
	::randomSeed(millis());
}

#if USE_DEFFN
void
Interpreter::execFn(const char *name)
{
	auto vf = _program.functionByName(name);
	if (vf == nullptr || ((vf->type & TYPE_DEFFN) == 0)) {
		raiseError(DYNAMIC_ERROR, NO_SUCH_FUNCION);
		return;
	}
	
	auto ff = reinterpret_cast<FunctionFrame*>(vf+1);
	_program._current.index = ff->lineNumber;
	_program._current.position = ff->linePosition;
	Program::Line *s = _program.current(_program._current);
	if (s != nullptr)
		_lexer.init(s->text + _program._current.position, true);
	else
		raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
}

void
Interpreter::setFnVars()
{
	Pointer sp = _program._sp;
	Pointer paramPtr;
	// Set old function variables
	uint8_t numberOfParameters = 0;
	while (true) {
		const auto f = _program.currentStackFrame();
		if (f->_type == Program::StackFrame::INPUT_OBJECT) {
			_program.pop();
			const auto ff = _program.currentStackFrame();
			if (ff->_type == Program::StackFrame::VALUE) {
				++numberOfParameters;
				_program.pop();
				continue;
			}
		} else if (f->_type == Program::StackFrame::SUBPROGRAM_RETURN) {
			_program.pop();
			paramPtr = _program._sp;
			break;
		}
		raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
		return;
	}
	_program._sp = sp;
	for (uint8_t i=0; i<numberOfParameters; ++i) {
		const auto f = _program.currentStackFrame();
		_program.pop();
		_program.pop();
		const Pointer ret = _program._sp;
		_program._sp = paramPtr;
		const auto fff = _program.currentStackFrame();
		if (fff->_type == Program::StackFrame::VALUE) {
			setVariable(f->body.inputObject.name,
			    fff->body.value);
		} else {
			raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
			break;
		}
		_program.pop();
		paramPtr = _program._sp;
		_program._sp = ret;
	}
	_program._sp = sp;
}

void
Interpreter::returnFromFn()
{
	uint8_t numParameters = 0;
	// Restore variables
	while (true) {
		const auto *f = _program.currentStackFrame();
		if (f->_type == Program::StackFrame::INPUT_OBJECT) {
			_program.pop();
			const auto *ff = _program.currentStackFrame();
			if (ff->_type == Program::StackFrame::VALUE) {
				setVariable(f->body.inputObject.name,
				    ff->body.value);
				_program.pop();
				++numParameters;
				continue;
			} else {
				raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
				return;
			}
		} else
			break;
	}
	
	const auto f = _program.currentStackFrame();
	if ((f != nullptr) && (f->_type == Program::StackFrame::SUBPROGRAM_RETURN)) {
		_program._current.index = f->body.gosubReturn.calleeIndex;
		_program._current.position = f->body.gosubReturn.textPosition;
		_program.pop();
	} else {
		raiseError(DYNAMIC_ERROR, RETURN_WO_GOSUB);
		return;
	}
	Program::Line *s = _program.current(_program._current);
	if (s != nullptr)
		_lexer.init(s->text + _program._current.position, true);
	else {
		raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
		return;
	}
	for (uint8_t i=0; i<numParameters; ++i) {
		const auto f=_program.currentStackFrame();
		if (f->_type == Program::StackFrame::VALUE)
			_program.pop();
		else {
			raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
			return;
		}
	}
}
#endif // USE_DEFFN

bool
Interpreter::next(const char *varName)
{
        do {
		Program::StackFrame *f = _program.currentStackFrame();
		if ((f != nullptr) && (f->_type == Program::StackFrame::FOR_NEXT)) { // Correct frame
			if (strcmp(f->body.forFrame.varName, varName) == 0) {
				Parser::Value v;
				valueFromVar(v, varName);
				v += f->body.forFrame.stepValue;
				setVariable(varName, v);
				return testFor(*f);
			} else
				_program.pop();
		} else { // Incorrect frame
			raiseError(DYNAMIC_ERROR, INVALID_NEXT);
			return false;
		}
	} while (true);
}

bool
Interpreter::testFor(Program::StackFrame &f)
{
	assert(f._type == Program::StackFrame::FOR_NEXT);
	
	auto &fBody = f.body.forFrame;
	Parser::Value v;
	valueFromVar(v, fBody.varName);
	if (fBody.stepValue > Parser::Value(Integer(0))) {
		if (v > fBody.finalvalue) {
			_program.pop();
			return true;
		}
	} else if (v < fBody.finalvalue) {
		_program.pop();
		return true;
	}
	_program.jump(fBody.calleeIndex);
	_program._current.position = fBody.textPosition;
	return false;
}

#if USE_SAVE_LOAD
void
Interpreter::save()
{
	EEpromHeader_t h = {
		// Program text buffer length
		.len = _program._textEnd,
		.magic_FFFFminuslen = Pointer(0xFFFFu) - _program._textEnd,
		// Checksum
		.crc16 = 0
	};
#if SAVE_LOAD_CHECKSUM
	// Compute program checksum
	for (Pointer p = 0; p < _program._textEnd; ++p)
		h.crc16 = _crc16_update(h.crc16, _program._text[p]);
#endif
	{
		EEPROMClass e;
		// Write program to EEPROM
		for (Pointer p = 0; p < _program._textEnd; ++p) {
			e.update(p + sizeof (EEpromHeader_t), _program._text[p]);
			_output.print('.');
		}
	}
	newline();
#if SAVE_LOAD_CHECKSUM
	// Compute checksum
	const uint16_t crc = eepromProgramChecksum(h.len);

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
	Pointer len;
	_program.newProg();
	if (!checkText(len))
		return;

	loadText(len);
	_program._textEnd = _program._variablesEnd = _program._arraysEnd = len;
}

void
Interpreter::chain()
{
	Pointer len;
	if (!checkText(len))
		return;

	_program.clearProg();
	_program.moveData(len);
	// Load program memory without progress
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

	if ((h.len > SINGLE_PROGSIZE) ||
	    (h.magic_FFFFminuslen != uint16_t(0xFFFF) - h.len)) {
		raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
		return false;
	}
#if SAVE_LOAD_CHECKSUM
	const uint16_t crc = eepromProgramChecksum(h.len);
	if (h.crc16 != crc) {
		raiseError(DYNAMIC_ERROR, BAD_CHECKSUM);
		return false;
	}
#endif // SAVE_LOAD_CHECKSUM
	len = h.len;
	return true;
}

void
Interpreter::loadText(uint16_t len, bool showProgress)
{
	EEPROMClass e;

	for (Pointer p = 0; p < len; ++p) {
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
	const auto f = _program.currentStackFrame();
	if (f != nullptr && f->_type == Program::StackFrame::INPUT_OBJECT) {
		strcpy(_inputVarName, f->body.inputObject.name);
		_program.pop();
		return true;
	} else
		return false;
}

void
Interpreter::doInput()
{
	Lexer l;
	l.init(_inputBuffer, false);
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
			v.switchSign();
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
VariableFrame::size() const
{
	return size(type);
}

uint8_t
VariableFrame::size(Parser::Value::Type t)
{
#if USE_DEFFN
	if (t & TYPE_DEFFN)
		return sizeof(VariableFrame) + sizeof(FunctionFrame);
#endif
#if OPT == OPT_SPEED
	switch (t) {
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
		return sizeof(VariableFrame) + sizeof (LongInteger);
#endif
	case Parser::Value::INTEGER:
		return sizeof(VariableFrame) + sizeof (Integer);
#if USE_REALS
	case Parser::Value::REAL:
		return sizeof(VariableFrame) + sizeof (Real);
#endif
	case Parser::Value::LOGICAL:
		return sizeof(VariableFrame) + sizeof (bool);
	case Parser::Value::STRING:
		return sizeof(VariableFrame) + STRING_SIZE;
	default:
		return sizeof(VariableFrame);
	}
#else // OPT != OPT_SPEED
	uint8_t res = sizeof(VariableFrame);
	
#if USE_LONGINT
	if (t == Parser::Value::LONG_INTEGER)
		res += sizeof(LongInteger);
	else
#endif
		if (t == Parser::Value::INTEGER)
			res += sizeof(Integer);
#if USE_REALS
	else if (t == Parser::Value::REAL)
		res += sizeof(Real);
#if USE_LONG_REALS
	else if (t == Parser::Value::LONG_REAL)
		res += sizeof(LongReal);
#endif
#endif // USE_REALS
	else if (t == Parser::Value::LOGICAL)
		res += sizeof(bool);
	else if (t == Parser::Value::STRING)
		res += STRING_SIZE;

	return res;
#endif // OPT
}

void
Interpreter::set(VariableFrame &f, const Parser::Value &v)
{
	switch (f.type) {
	case Parser::Value::LOGICAL:
	{
		union
		{
			char *b;
			bool *i;
		} U;
		U.b = f.bytes;
		*U.i = bool(v);
	}
		break;
	case Parser::Value::INTEGER:
	{
		union
		{
			char *b;
			Integer *i;
		} U;
		U.b = f.bytes;
		*U.i = Integer(v);
	}
		break;
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
	{
		union
		{
			char *b;
			LongInteger *i;
		} U;
		U.b = f.bytes;
		*U.i = LongInteger(v);
	}
		break;
#endif // USE_LONGINT
#if USE_REALS
	case Parser::Value::REAL:
	{
		union
		{
			char *b;
			Real *r;
		} U;
		U.b = f.bytes;
		*U.r = Real(v);
	}
		break;
#if USE_LONG_REALS
	case Parser::Value::LONG_REAL:
	{
		union
		{
			char *b;
			LongReal *r;
		} U;
		U.b = f.bytes;
		*U.r = LongReal(v);
	}
		break;	
#endif
#endif // USE_REALS
	case Parser::Value::STRING:
	{
		auto fr = _program.currentStackFrame();
		if (fr == nullptr || fr->_type != Program::StackFrame::STRING) {
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

bool
Interpreter::readInput()
{
	int a = _input.available();
	if (a <= 0)
		return false;

	const uint8_t availableSize = PROGSTRINGSIZE - 1 - _inputPosition;
	a = min(a, availableSize);

	const size_t read = _input.readBytes(_inputBuffer + _inputPosition, a);
	assert(read <= availableSize);
	uint8_t end = _inputPosition + read;
	for (uint8_t i = _inputPosition; i < end; ++i) {
		char c = _inputBuffer[i];
		switch (c) {
		case char(ASCII::HT):
                        break;
		case char(ASCII::BS):
		case char(ASCII::DEL):
			if (_inputPosition > 0) {
				--_inputPosition;
				_output.print(char(ASCII::BS));
				_output.print(char(ASCII::SPACE));
				_output.print(char(ASCII::BS));
			}
			break;
		case char(ASCII::CR):
			_output.println();
			_inputBuffer[i] = '\0';
			_inputPosition = 0;
			return true;
#if PROCESS_LF
		case char(ASCII::LF) :
#if PROCESS_LF == LF_NEWLINE
			_output.println();
			_inputBuffer[i] = '\0';
			_inputPosition = 0;
#endif
			return true;
#endif // PROCESS_LF
		default:
#if AUTOCAPITALIZE
			c = toupper(c);
			_inputBuffer[i] = c;
#endif // AUTOCAPITALIZE
			// Only acept character if there is room for upcoming
			// control one (line end or del/bs)
			if (availableSize > 1) {
				++_inputPosition;
				_output.print(c);
			}
		}
	}
	return false;
}

void
Interpreter::print(const char *text, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(text), _output.print(char(ASCII::SPACE));
}

void
Interpreter::print(ProgMemStrings index, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	writePgm(index), _output.print(char(ASCII::SPACE));
}

void
Interpreter::writePgm(ProgMemStrings index)
{
	writePgm(progmemString(index));
}

void
Interpreter::writePgm(PGM_P str)
{
	char buf[STRING_SIZE];
	strcpy_P(buf, str);

	_output.print(buf);
}

#if USE_DEFFN
void
Interpreter::newFunction(const char *fname, uint8_t pos)
{
	Pointer index = _program._textEnd;

	VariableFrame *f;
	for (f = _program.variableByIndex(index); f != nullptr; index += f->size(),
	    f = _program.variableByIndex(index)) {
		if (!(f->type & TYPE_DEFFN))
			continue;
		int res = strcmp(fname, f->name);
		if (res == 0) {
			raiseError(DYNAMIC_ERROR,
			    FUNCTION_DUPLICATE);
			return;
		} else if (res < 0)
			break;
	}

	if (f == nullptr)
		f = reinterpret_cast<VariableFrame*> (_program._text + index);

	const Parser::Value::Type t = Parser::Value::Type(
	    uint8_t(typeFromName(fname)) | TYPE_DEFFN);
	const uint16_t dist = VariableFrame::size(t);
	if ((_program._arraysEnd + dist)>= _program._sp) {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return;
	}
	memmove(_program._text + index + dist, _program._text + index,
	    _program._arraysEnd - index);
	f->type = t;
	strncpy(f->name, fname, VARSIZE);
	FunctionFrame *ff = reinterpret_cast<FunctionFrame*>(f->bytes);
	ff->lineNumber = _program._current.index;
	ff->linePosition = _program._current.position+pos;
	_program._variablesEnd += dist;
	_program._arraysEnd += dist;
}
#endif // USE_DEFFN

void
Interpreter::print(Token t)
{
	char buf[10];
	
	const bool res = Lexer::getTokenString(t,
	    reinterpret_cast<uint8_t*>(buf));
	if (res)
		print(buf, VT100::TextAttr(uint8_t(VT100::BRIGHT) |
		    uint8_t(VT100::C_GREEN)));
	else
		print(ProgMemStrings::S_ERROR, VT100::TextAttr(uint8_t(VT100::BRIGHT) |
		    uint8_t(VT100::C_RED)));
}

#if USE_DATA
bool
Interpreter::read(Parser::Value &value)
{
	DataParser dparser(*this);
	if (_dataParserContinue) {
		const Program::Line *l = _program.current(_program._dataCurrent);
		if (l == nullptr)
			return false;
		const bool result = dparser.read(l->text+_program._dataCurrent.
		    position,value);
		_program._dataCurrent.position += dparser.lexer().getPointer();
		if (result)
			return true;
		else {
			_dataParserContinue = false;
			_program.getNextLine(_program._dataCurrent);
		}
	}
	for (const Program::Line *l = _program.current(_program._dataCurrent);
	    l != nullptr; l = _program.current(_program._dataCurrent)) {
		const bool result = dparser.searchData(l->text+_program.
		    _dataCurrent.position, value);
		_program._dataCurrent.position += dparser.lexer().getPointer();
		if (result) {
			_dataParserContinue = true;
			return true;
		} else
			_program.getNextLine(_program._dataCurrent);
			
	}
	return false;
}
#endif // USE_DATA

void
Interpreter::print(Integer i, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(i), _output.print(char(ASCII::SPACE));
}

#if USE_TEXTATTRIBUTES
void
Interpreter::printEsc(const char *str)
{
	writePgm(ProgMemStrings::VT100_ESCSEQ), _output.print(str);
}

void
Interpreter::printEsc(ProgMemStrings index)
{
	writePgm(ProgMemStrings::VT100_ESCSEQ);
	writePgm(index);
}

void
Interpreter::printTab(const Parser::Value &v, bool flag)
{
	Integer tabs;
#if USE_REALS
	if (v.type() == Parser::Value::REAL)
		tabs = math<Real>::round(Real(v));
	else
#endif
		tabs = Integer(v);
	if (tabs > 0) {
		if (tabs == 1)
			return;
		writePgm(ProgMemStrings::VT100_ESCSEQ);
		if (flag) {
			writePgm(ProgMemStrings::VT100_LINEHOME);
			writePgm(ProgMemStrings::VT100_ESCSEQ);
                        --tabs;
		}
		_output.print(tabs), _output.print('C');
	} else
		raiseError(DYNAMIC_ERROR, INVALID_TAB_VALUE, false);
}
#endif // USE_TEXTATTRIBUTES

void
Interpreter::print(long i, VT100::TextAttr attr)
{
	AttrKeeper _a(*this, attr);

	_output.print(i), _output.print(char(ASCII::SPACE));
}

void
Interpreter::raiseError(ErrorType type, ErrorCodes errorCode, bool fatal)
{
	// Output Program line number if running program
	const Program::Line *l = _program.current(_program._current);
	if ((_state == EXECUTE) && (l != nullptr)) {
		print(long(l->number), VT100::C_YELLOW);
		_output.print(':');
	}
	if (type == DYNAMIC_ERROR)
		print(ProgMemStrings::S_SEMANTIC);
	else // STATIC_ERROR
		print(ProgMemStrings::S_SYNTAX);
	//print(ProgMemStrings::S_SEMANTIC);
	print(ProgMemStrings::S_ERROR, VT100::C_RED);
	if (type == DYNAMIC_ERROR)
		print(Integer(errorCode));
	else {// STATIC_ERROR
		print(Integer(_parser.getError()));
#if CONF_ERROR_STRINGS
		writePgm((PGM_P)pgm_read_ptr(
		    &_parser.errorStrings[Integer(_parser.getError())] ));
#endif
	}
	newline();
	
	if (fatal)
		_state = SHELL;
}

bool
Interpreter::arrayElementIndex(ArrayFrame *f, uint16_t &index)
{
	index = 0;
	uint8_t dim = f->numDimensions, mul = 1;
	while (dim-- > 0) {
		Program::StackFrame *sf = _program.currentStackFrame();
		if (sf == nullptr ||
		    sf->_type != Program::StackFrame::ARRAY_DIMENSION ||
		    sf->body.arrayDimension > f->dimension[dim]) {
			return false;
		}
		index += mul * sf->body.arrayDimension;
		mul *= f->dimension[dim] + 1;
		_program.pop();
	};
	return true;
}

VariableFrame*
Interpreter::setVariable(const char *name, const Parser::Value &v)
{
	Pointer index = _program._textEnd;

	VariableFrame *f;
	for (f = _program.variableByIndex(index); f != nullptr; index += f->size(),
	    f = _program.variableByIndex(index)) {
#if USE_DEFFN
		if (f->type & TYPE_DEFFN)
			continue;
#endif
		const auto res = strcmp(name, f->name);
		if (res == 0) {
			set(*f, v);
			return f;
		} else if (res < 0)
			break;
	}

	if (f == nullptr)
		f = reinterpret_cast<VariableFrame*> (_program._text + index);

	const Parser::Value::Type t = typeFromName(name);
	const uint16_t dist = VariableFrame::size(t);
	if (_program._arraysEnd+dist >= _program._sp) {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return nullptr;
	}
	memmove(_program._text + index + dist, _program._text + index,
	    _program._arraysEnd - index);
	f->type = t;
	strncpy(f->name, name, VARSIZE);
	_program._variablesEnd += dist;
	_program._arraysEnd += dist;
	set(*f, v);

	return f;
}

void
Interpreter::setArrayElement(const char *name, const Parser::Value &v)
{
	ArrayFrame *f = _program.arrayByName(name);
	if (f == nullptr) {
#if OPT_IMPLICIT_ARRAYS
		uint8_t dims = 0;
		Program::StackFrame* sf;
		Pointer index = _program._sp;
		while (((sf = _program.stackFrameByIndex(index)) != nullptr) &&
		    (sf->_type == Program::StackFrame::ARRAY_DIMENSION)) {
			++dims;
			pushDimension(10);
			index += sf->size();
		}
		pushDimensions(dims);
		newArray(name);
		f = _program.arrayByName(name);
		if (f == nullptr) {
#endif
			raiseError(DYNAMIC_ERROR, NO_SUCH_ARRAY);
			return;
#if OPT_IMPLICIT_ARRAYS
		}
#endif
	}

	uint16_t index;
	if (!arrayElementIndex(f, index)) {
		raiseError(DYNAMIC_ERROR, INVALID_VALUE_TYPE);
		return;
	}
	f->set(index, v);
}

void
Interpreter::newArray(const char *name)
{
	auto f = _program.stackFrameByIndex(_program._sp);
	if (f != nullptr && f->_type == Program::StackFrame::ARRAY_DIMENSIONS) {
		uint8_t dimensions = f->body.arrayDimensions;
		_program.pop();
		uint16_t size = 1;
		auto sp = _program._sp; // go on stack frames, containing dimesions
		for (uint8_t dim = 0; dim < dimensions; ++dim) {
			f = _program.stackFrameByIndex(sp);
			if (f != nullptr && f->_type ==
			    Program::StackFrame::ARRAY_DIMENSION) {
				size *= f->body.arrayDimension + 1;
				sp += f->size(Program::StackFrame::ARRAY_DIMENSION);
			} else {
				raiseError(DYNAMIC_ERROR, INTERNAL_ERROR);
				return;
			}
		}
		auto array = addArray(name, dimensions, size);
		if (array != nullptr) { // go on stack frames, containong dimesions once more
			// now popping
			for (uint8_t dim = dimensions; dim-- > 0;) {
				f = _program.currentStackFrame();
				array->dimension[dim] = f->body.arrayDimension;
				_program.pop();
			}
		}
	}
}

const VariableFrame*
Interpreter::getVariable(const char *name)
{
	const VariableFrame *f = _program.variableByName(name);
	if (f == nullptr) {
		const Parser::Value v(Integer(0));
		f = setVariable(name, v);
	}
	return f;
}

void
Interpreter::pushString(const char *str)
{
	auto f = _program.push(Program::StackFrame::STRING);
	if (f == nullptr) {
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
		return;
	}
	strcpy(f->body.string, str);
}

void
Interpreter::pushDimension(uint16_t dim)
{
	Program::StackFrame *f =
	    _program.push(Program::StackFrame::ARRAY_DIMENSION);
	if (f == nullptr)
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
	else
		f->body.arrayDimension = dim;
}

void
Interpreter::pushDimensions(uint8_t dim)
{
	Program::StackFrame *f =
	    _program.push(Program::StackFrame::ARRAY_DIMENSIONS);
	if (f != nullptr)
		f->body.arrayDimensions = dim;
	else
		raiseError(DYNAMIC_ERROR, STACK_FRAME_ALLOCATION);
}

bool
Interpreter::pushResult()
{
	return pushValue(_result);
}

bool
Interpreter::confirm()
{
	bool result = false;
	do {
		print(ProgMemStrings::S_REALLY);
		_output.print(char(ASCII::QMARK));
		newline();
		while (_input.available() <= 0);
		char c = _input.read();
		_output.print(c);
		while (_input.available() <= 0);
		const char eol = _input.read();
		if (eol != char(ASCII::CR) &&
		    eol != char(ASCII::LF)) {
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
	return result;
}

#if USE_STRINGOPS
void
Interpreter::strConcat()
{
	const char *str1;
	if (popString(str1)) {
		Program::StackFrame *ff = _program.currentStackFrame();
		if ((ff != nullptr) && (ff->_type == Program::StackFrame::STRING)) {
			uint8_t l1 = strlen(ff->body.string);
			uint8_t l2 = strlen(str1);
			if (l1 + l2 >= STRING_SIZE)
				l2 = STRING_SIZE - l1 - 1;
			strncpy(ff->body.string + l1, str1, l2);
			ff->body.string[l1 + l2] = 0;
			return;
		}
	}
}

bool
Interpreter::strCmp()
{
	const char *str1;
	if (popString(str1)) {
		const char *str2;
		if (popString(str2))
			return strncmp(str1, str2, STRING_SIZE) == 0;
	}
	return false;
}
#endif // USE_STRINGOPS

void
Interpreter::end()
{
	_state = SHELL;
}

uint16_t
ArrayFrame::size() const
{
	// Header with dimensions vector
	uint16_t result = sizeof (ArrayFrame) + numDimensions *
	    sizeof(uint16_t);
	
	uint16_t mul = dataSize();
	result += mul;

	return result;
}

uint16_t
ArrayFrame::dataSize() const
{
	uint16_t mul = numElements();

	switch (type) {
	case Parser::Value::INTEGER:
		mul *= sizeof (Integer);
		break;
#if USE_LONGINT
	case Parser::Value::LONG_INTEGER:
		mul *= sizeof (LongInteger);
		break;
#endif
#if USE_REALS
#if USE_LONG_REALS
	case Parser::Value::LONG_REAL:
		mul *= sizeof (LongReal);
		break;	
#endif // USE_LONG_REALS
	case Parser::Value::REAL:
		mul *= sizeof (Real);
		break;
#endif // USE_REALS
	case Parser::Value::LOGICAL:
	{
		uint16_t s = mul / 8;
		if ((mul % 8) != 0)
			++s;
		mul = s;
		break;
	}
	default:
		break;
	}
	return mul;
}

bool
ArrayFrame::get(uint16_t index, Parser::Value& v) const
{
	assert(index < numElements());
	if (index < numElements()) {
		switch (type) {
		case Parser::Value::INTEGER:
			v = get<Integer>(index);
			return true;
#if USE_LONGINT
		case Parser::Value::LONG_INTEGER:
			v = get<LongInteger>(index);
			return true;
#endif
#if USE_REALS
#if USE_LONG_REALS
		case Parser::Value::LONG_REAL:
			v = get<LongReal>(index);
			return true;	
#endif // USE_LONG_REALS
		case Parser::Value::REAL:
			v = get<Real>(index);
			return true;
#endif
		case Parser::Value::LOGICAL:
		{
			const uint8_t _byte = data()[index / 8];
			v = bool((_byte >> (index % 8)) & 1);
			return true;
		}
		default:
			return false;
		}
	}
	return false;
}

bool
ArrayFrame::set(uint16_t index, const Parser::Value &v)
{
	assert(index < numElements());
	if (index < numElements()) {
		switch (type) {
		case Parser::Value::INTEGER:
			set(index, Integer(v));
			return true;
#if USE_LONGINT
		case Parser::Value::LONG_INTEGER:
			set(index, LongInteger(v));
			return true;
#endif
#if USE_REALS
#if USE_LONG_REALS
		case Parser::Value::LONG_REAL:
			set(index, LongReal(v));
			return true;	
#endif
		case Parser::Value::REAL:
			set(index, Real(v));
			return true;
#endif // USE_REALS
		case Parser::Value::LOGICAL:
		{
			uint8_t &_byte = data()[index / uint8_t(8)];
			const bool _v = bool(v);
			if (_v)
				_byte |= uint8_t(1) << (index % uint8_t(8));
			else
				_byte &= ~(uint8_t(1) << (index % uint8_t(8)));
			return true;
		}
		default:
			return false;
		}
	}
	return false;
}

uint16_t
ArrayFrame::numElements() const
{
	uint16_t mul = 1;
	
	// Every dimension is from 0 to dimension[i], thats why 
	// it is increased by 1
	for (uint8_t i=0u; i<numDimensions; ++i)
		mul *= dimension[i] + 1;
	
	return mul;
}

ArrayFrame*
Interpreter::addArray(const char *name, uint8_t dim, uint16_t num)
{
	Pointer index = _program._variablesEnd;
	ArrayFrame *f;
	while (f = _program.arrayByIndex(index)) {
		int res = strcmp(name, f->name);
		if (res == 0) {
			raiseError(DYNAMIC_ERROR, REDIMED_ARRAY);
			return nullptr;
		} else if (res < 0)
			break;
		
		index += f->size();
	}

	if (f == nullptr)
		f = reinterpret_cast<ArrayFrame*> (_program._text + index);

	Parser::Value::Type t;
#if USE_LONGINT
	if (endsWith(name, "%!")) {
		t = Parser::Value::LONG_INTEGER;
		num *= sizeof (LongInteger);
	} else
#endif  
	if (endsWith(name, '%')) {
		t = Parser::Value::INTEGER;
		num *= sizeof (Integer);
	} else if (endsWith(name, '@')) {
		uint16_t s = num / 8;
		if ((num % 8) != 0)
			++s;
		t = Parser::Value::LOGICAL;
		num = s;
	} else { // real
#if USE_REALS
#if USE_LONG_REALS
		if (endsWith(name, '!')) {
			t = Parser::Value::LONG_REAL;
			num *= sizeof (LongReal);
		} else {
#endif // USE_LONG_REALS
			t = Parser::Value::REAL;
			num *= sizeof (Real);
#if USE_LONG_REALS
		}
#endif
#else  // Integer
			t = Parser::Value::INTEGER;
			num *= sizeof (Integer);
#endif // USE_REALS
	}

	const uint16_t dist = sizeof (ArrayFrame) + sizeof (uint16_t) * dim + num;
	if (_program._arraysEnd + dist >= _program._sp) {
		raiseError(DYNAMIC_ERROR, OUTTA_MEMORY);
		return nullptr;
	}
	memmove(_program._text + index + dist, _program._text + index,
	    _program._arraysEnd - index);
	f->type = t;
	f->numDimensions = dim;
	strcpy(f->name, name);
	memset(f->data(), 0, num);
	_program._arraysEnd += dist;

	return f;
}

Parser::Value::Type
Interpreter::typeFromName(const char *fname)
{
	Parser::Value::Type t;
#if USE_LONGINT
	if (endsWith(fname, "%!")) {
		t = Parser::Value::LONG_INTEGER;
	} else
#endif // USE_LONGINT
		if (endsWith(fname, '%')) {
		t = Parser::Value::INTEGER;
	} else if (endsWith(fname, '@')) {
		t = Parser::Value::LOGICAL;
	} else if (endsWith(fname, '$')) {
		t = Parser::Value::STRING;
	}
#if USE_REALS
#if USE_LONG_REALS
         else if (endsWith(fname, '!')) {
		t = Parser::Value::LONG_REAL;
	}
#endif // USE_LONG_REALS
         else {
		t = Parser::Value::REAL;
#else
	 else {
		t = Parser::Value::INTEGER;
#endif // USE_REALS
	}
	return t;
}

} // namespace BASIC
