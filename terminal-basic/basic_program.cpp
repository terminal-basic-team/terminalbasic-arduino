/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
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

#include "basic_program.hpp"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "basic_interpreter.hpp"

namespace BASIC
{

Program::Program(uint16_t progsize) :
#if USE_EXTMEM
_text(reinterpret_cast<char*> (EXTMEM_ADDRESS)),
#endif
programSize(progsize)
{
	assert(_text != nullptr);
	assert(progsize <= SINGLE_PROGSIZE);
}

Program::Line*
Program::getNextLine()
{
	if (_jumpFlag) {
		_current.index = _jump;
		_jumpFlag = false;
		return current(_current);
	}
	
	return getNextLine(_current);
}

Program::Line*
Program::getNextLine(Position &pos)
{
	Program::Line *result = current(pos);
	if (result != nullptr) {
		pos.index += result->size;
		pos.position = 0;
	}
	return result;
}

Program::Line*
Program::current(const Position &pos) const
{
	if (pos.index < _textEnd)
		return lineByIndex(pos.index);
	else
		return nullptr;
}

Program::Line*
Program::first() const
{
	return lineByIndex(0);
}

Program::Line*
Program::last() const
{
	return lineByIndex(_textEnd);
}

void
Program::jump(Pointer newVal)
{
	_jump = newVal;
	_jumpFlag = true;
}

Program::Line*
Program::lineByIndex(Pointer address) const
{
	return const_cast<Line*> (reinterpret_cast<const Line*> (
	    _text + address));
}

Program::Line*
Program::lineByNumber(uint16_t number, Pointer address)
{
	Program::Line *result = nullptr;

	if (address <= _textEnd) {
		_current.index = address;
		for (Line *cur = getNextLine(); cur != nullptr;
		    cur = getNextLine()) {
			if (cur->number == number) {
				result = cur;
				break;
			}
		}
	}
	return result;
}

uint8_t
Program::StackFrame::size(Type t)
{
#if OPT == OPT_SPEED
	switch (t) {
	case SUBPROGRAM_RETURN:
		return sizeof (Type) + sizeof (GosubReturn);
	case FOR_NEXT:
		return sizeof (Type) + sizeof (ForBody);
	case STRING:
		return sizeof (Type) + STRINGSIZE;
	case ARRAY_DIMENSION:
		return sizeof (Type) + sizeof (uint16_t);
	case ARRAY_DIMENSIONS:
		return sizeof (Type) + sizeof (uint8_t);
	case VALUE:
		return sizeof (Type) + sizeof (Parser::Value);
	case INPUT_OBJECT:
		return sizeof (Type) + sizeof (VariableBody);
	default:
		return 0;
	}
#else
	if (t == SUBPROGRAM_RETURN)
		return (sizeof (Type) + sizeof (GosubReturn));
	else if (t == FOR_NEXT)
		return (sizeof (Type) + sizeof (ForBody));
	else if (t == STRING)
		return (sizeof (Type) + STRINGSIZE);
	else if (t == ARRAY_DIMENSION)
		return (sizeof (Type) + sizeof (uint16_t));
	else if (t == ARRAY_DIMENSIONS)
		return (sizeof (Type) + sizeof (uint8_t));
	else if (t == VALUE)
		return (sizeof (Type) + sizeof (Parser::Value));
	else if (t == INPUT_OBJECT)
		return (sizeof (Type) + sizeof (VariableBody));
	else
		return 0;
#endif
}

void
Program::clearProg()
{
	_jumpFlag = false;
	_reset();
}

void
Program::moveData(Pointer dest)
{
	const int32_t diff = _textEnd-dest;
	memmove(_text+dest, _text+_textEnd, _arraysEnd-_textEnd);
	_variablesEnd -= diff;
	_arraysEnd -= diff;
	_textEnd = dest;
}

void
Program::newProg()
{
	clearProg();
	_textEnd = _variablesEnd = _arraysEnd = _jump = 0;
#if CLEAR_PROGRAM_MEMORY
	memset(_text, 0xFF, programSize);
#endif
}

VariableFrame*
Program::variableByName(const char *name)
{
	auto index = _textEnd;

	for (auto f = variableByIndex(index); f != nullptr;
	    f = variableByIndex(index)) {
		const int8_t res = strncmp(name, f->name, VARSIZE);
		if (res == 0) {
			return f;
		} else if (res < 0)
			break;
		index += f->size();
	}
	return nullptr;
}

Pointer
Program::objectIndex(const void *obj) const
{
	return reinterpret_cast<const char*>(obj) - _text;
}

Program::StackFrame*
Program::push(StackFrame::Type t)
{
	const uint8_t s = StackFrame::size(t);
	if ((_sp - s) < _arraysEnd)
		return nullptr;

	_sp -= StackFrame::size(t);
	StackFrame *f = stackFrameByIndex(_sp);
	if (f != nullptr)
		f->_type = t;
	return f;
}

void
Program::pop()
{
	const StackFrame *f = stackFrameByIndex(_sp);
	if (f != nullptr)
		_sp += StackFrame::size(f->_type);
}

void
Program::reverseLast(StackFrame::Type type)
{
	StackFrame *f = this->currentStackFrame();
	if (f != nullptr && f->_type == type) {
		char buf[sizeof (StackFrame)];
		StackFrame *fr = reinterpret_cast<StackFrame*> (buf);
		memcpy(fr, f, StackFrame::size(f->_type));
		this->pop();
		reverseLast(type);
		pushBottom(fr);
	}
}

void
Program::pushBottom(StackFrame *f)
{
	auto newFrame = this->currentStackFrame();
	if ((newFrame == nullptr) || (newFrame->_type != f->_type)) {
		newFrame = this->push(f->_type);
		memcpy(newFrame, f, StackFrame::size(f->_type));
	} else {
		char buf[sizeof (StackFrame)];
		auto fr = reinterpret_cast<StackFrame*> (buf);
		memcpy(fr, newFrame, StackFrame::size(f->_type));
		this->pop();
		pushBottom(f);
		f = this->push(newFrame->_type);
		memcpy(f, fr, StackFrame::size(newFrame->_type));
	}
}

Program::StackFrame*
Program::stackFrameByIndex(Pointer address)
{
	if ((address > 0) && (address < programSize))
		return reinterpret_cast<StackFrame*> (_text + address);
	else
		return nullptr;
}

Program::StackFrame*
Program::currentStackFrame()
{
	if (_sp < programSize)
		return stackFrameByIndex(_sp);
	else
		return nullptr;
}

ArrayFrame*
Program::arrayByName(const char *name)
{
	auto index = _variablesEnd;

	for (auto f = arrayByIndex(index); index < _arraysEnd;
	    index += f->size(),
	    f = arrayByIndex(index)) {
		const int8_t res = strcmp(name, f->name);
		if (res == 0) {
			return f;
		} else if (res < 0)
			break;
	}
	return nullptr;
}

VariableFrame*
Program::variableByIndex(Pointer index)
{
	if (index < _variablesEnd)
		return reinterpret_cast<VariableFrame*> (_text + index);
	else
		return nullptr;
}

ArrayFrame*
Program::arrayByIndex(Pointer index)
{
	return reinterpret_cast<ArrayFrame*> (_text + index);
}

bool
Program::addLine(uint16_t num, const char *line)
{
	uint16_t size;
	char tempBuffer[PROGSTRINGSIZE];

	Lexer _lexer;
	_lexer.init(line);
	uint8_t position = 0;
	uint8_t lexerPosition = _lexer.getPointer();

	while (_lexer.getNext()) {
		if (position >= (PROGSTRINGSIZE-1))
			return false;
		
		const Token tok = _lexer.getToken();
		const uint8_t t = uint8_t(0x80) + uint8_t(tok);
		if (tok < Token::RPAREN) { // One byte tokens
			tempBuffer[position++] = t;
			lexerPosition = _lexer.getPointer();
			if (tok == Token::KW_REM) { // Save rem text as is
				while (line[lexerPosition] == ' ' ||
				    line[lexerPosition] == '\t')
					++lexerPosition;
				const uint8_t remaining = strlen(line) - lexerPosition;
				memcpy(tempBuffer + position, line + lexerPosition,
				    remaining);
				position += remaining;
				break;
			}
		} else if (tok == Token::C_INTEGER) {
			tempBuffer[position++] = t;
			if ((position + sizeof(INT)) >= PROGSTRINGSIZE-1)
				return false;
			const INT v = INT(_lexer.getValue());
			*reinterpret_cast<INT*>(tempBuffer+position) = v;
			position += sizeof(INT);
			lexerPosition = _lexer.getPointer();
		}
#if USE_REALS
		 else if (tok == Token::C_REAL) {
			tempBuffer[position++] = t;
			if ((position + sizeof(Real)) >= PROGSTRINGSIZE-1)
				return false;
			const Real v = Real(_lexer.getValue());
			*reinterpret_cast<Real*>(tempBuffer+position) = v;
			position += sizeof(Real);
			lexerPosition = _lexer.getPointer();
		}
#endif // USE_REALS
		else { // Other tokens
			tempBuffer[position++] = ' ';
			while (line[lexerPosition] == ' ' ||
			    line[lexerPosition] == '\t')
				++lexerPosition;
			const uint8_t siz = _lexer.getPointer() - lexerPosition;
			if ((position + siz) >= PROGSTRINGSIZE-1)
				return false;
			memcpy(tempBuffer + position, line + lexerPosition, siz);
			position += siz;
			lexerPosition = _lexer.getPointer();
		}
	}
	tempBuffer[position++] = 0;
	size = position;
	line = tempBuffer;

	return addLine(num, line, size);
}

void
Program::removeLine(uint16_t num)
{
	const Line *line = this->lineByNumber(num, 0);
	if (line != nullptr) {
		const uint16_t index = objectIndex(line);
		assert(index < _textEnd);
		const uint16_t next = index+line->size;
		const uint16_t len = _arraysEnd-next;
		_textEnd -= line->size;
		_variablesEnd -= line->size;
		_arraysEnd -= line->size;
		memmove(_text+index, _text+next, len);
	}
}

bool
Program::addLine(uint16_t num, const char *text, uint16_t len)
{
	reset();

	if (_textEnd == 0) // First string insertion
		return insert(num, text, len);

	const uint16_t strLen = sizeof(Line) + len;
	// Iterate over
	Line *cur;
	for (cur = current(_current); _current.index < _textEnd;
	    cur = current(_current)) {
		if (num < cur->number) {
			break;
		} else if (num == cur->number) { // Replace string
			const uint16_t newSize = strLen;
			const uint16_t curSize = cur->size;
			const int16_t dist = long(newSize) - curSize;
			const uint16_t bytes2copy = _arraysEnd -
			    (_current.index + curSize);
			if ((_arraysEnd + dist) >= _sp)
				return (false);
			memmove(_text + _current.index + newSize,
			    _text + _current.index + curSize, bytes2copy);
			cur->number = num;
			cur->size = strLen;
			memcpy(cur->text, text, len);
			_textEnd += dist, _variablesEnd += dist,
			    _arraysEnd += dist;
			return true;
		}
		_current.index += cur->size;
	}
	return insert(num, text, len);
}

bool
Program::insert(uint16_t num, const char *text, uint8_t len)
{
	assert(len <= PROGSTRINGSIZE);
	const uint8_t strLen = sizeof(Line) + len;

	if (_arraysEnd + strLen >= _sp)
		return false;

	memmove(_text + _current.index + strLen, _text + _current.index,
	    _arraysEnd - _current.index);

	Line *cur = lineByIndex(_current.index);
	cur->number = num;
	cur->size = strLen;
	memcpy(cur->text, text, len);
	_textEnd += strLen, _variablesEnd += strLen, _arraysEnd += strLen;
	return true;
}

void
Program::reset(Pointer size)
{
	_reset();
	if (size > 0)
		_textEnd = _variablesEnd = _arraysEnd = size;
}

Pointer
Program::size() const
{
	return _textEnd;
}

void
Program::_reset()
{
	_current.index = _current.position = 0;
#if USE_DATA
	_dataCurrent.index = _dataCurrent.position = 0;
#endif
	_sp = programSize;
}

} // namespace BASIC
