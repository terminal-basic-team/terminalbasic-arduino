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
	assert(_text != NULL);
	assert(progsize <= PROGRAMSIZE);
}

Program::String*
Program::getString()
{
	if (_jumpFlag) {
		_current = _jump;
		_jumpFlag = false;
		return current();
	}
	
	Program::String *result = current();
	if (result != nullptr) {
		_current += result->size;
		_textPosition = 0;
	}
	return result;
}

Program::String*
Program::current() const
{
	if (_current < _textEnd)
		return stringByIndex(_current);
	else
		return nullptr;
}

Program::String*
Program::first() const
{
	return stringByIndex(0);
}

Program::String*
Program::last() const
{
	return stringByIndex(_textEnd);
}

void
Program::jump(uint16_t newVal)
{
	_jump = newVal;
	_jumpFlag = true;
}

Program::String*
Program::stringByIndex(uint16_t index) const
{
	return const_cast<String*> (reinterpret_cast<const String*> (
	    _text + index));
}

Program::String*
Program::lineByNumber(uint16_t number, uint16_t index)
{
	Program::String *result = NULL;

	if (index <= _textEnd) {
		_current = index;
		for (String *cur = getString(); cur != nullptr;
		    cur = getString()) {
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
		return sizeof (Type) + sizeof (InputBody);
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
		return (sizeof (Type) + sizeof (InputBody));
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
Program::moveData(uint16_t dest)
{
	int32_t diff = _textEnd-dest;
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

VariableFrame *
Program::variableByName(const char *name)
{
	uint16_t index = _textEnd;

	for (VariableFrame *f = variableByIndex(index); f != NULL;
	    f = variableByIndex(index)) {
		int8_t res = strcmp(name, f->name);
		if (res == 0) {
			return f;
		} else if (res < 0)
			break;
		index += f->size();
	}
	return nullptr;
}

uint16_t
Program::stringIndex(const String *s) const
{
	return ((char*) s) - _text;
}

uint16_t
Program::variableIndex(VariableFrame *f) const
{
	return ((char*) f) - _text;
}

uint16_t
Program::arrayIndex(ArrayFrame *f) const
{
	return ((char*) f) - _text;
}

Program::StackFrame*
Program::push(StackFrame::Type t)
{
	const uint8_t s = StackFrame::size(t);
	if ((_sp - s) < _arraysEnd)
		return nullptr;

	_sp -= StackFrame::size(t);
	StackFrame *f = stackFrameByIndex(_sp);
	if (f != NULL)
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
	StackFrame *newFrame = this->currentStackFrame();
	if ((newFrame == NULL) || (newFrame->_type != f->_type)) {
		newFrame = this->push(f->_type);
		memcpy(newFrame, f, StackFrame::size(f->_type));
	} else {
		char buf[sizeof (StackFrame)];
		StackFrame *fr = reinterpret_cast<StackFrame*> (buf);
		memcpy(fr, newFrame, StackFrame::size(f->_type));
		this->pop();
		pushBottom(f);
		f = this->push(newFrame->_type);
		memcpy(f, fr, StackFrame::size(newFrame->_type));
	}
}

Program::StackFrame*
Program::stackFrameByIndex(uint16_t index)
{
	if ((index > 0) && (index < programSize))
		return reinterpret_cast<StackFrame*> (_text + index);
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
	uint16_t index = _variablesEnd;

	for (ArrayFrame *f = arrayByIndex(index); index < _arraysEnd;
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
Program::variableByIndex(uint16_t index)
{
	if (index < _variablesEnd)
		return reinterpret_cast<VariableFrame*> (_text + index);
	else
		return nullptr;
}

ArrayFrame*
Program::arrayByIndex(uint16_t index)
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
	uint8_t position = 0, lexerPosition = _lexer.getPointer();

	while (_lexer.getNext()) {
		if (position >= (PROGSTRINGSIZE-1))
			return false;
		uint8_t t = uint8_t(0x80) + uint8_t(_lexer.getToken());
		if (_lexer.getToken() < Token::STAR) { // One byte tokens
			tempBuffer[position++] = t;
			lexerPosition = _lexer.getPointer();
			if (_lexer.getToken() == Token::KW_REM) { // Save rem text as is
				while (line[lexerPosition] == ' ' ||
				    line[lexerPosition] == '\t')
					++lexerPosition;
				const uint8_t remaining = strlen(line) - lexerPosition;
				memcpy(tempBuffer + position, line + lexerPosition,
				    remaining);
				position += remaining;
				break;
			}
		} else if (_lexer.getToken() == Token::C_INTEGER) {
			tempBuffer[position++] = t;
#if USE_LONGINT
			if ((position + 4) >= PROGSTRINGSIZE-1)
				return false;
			const LongInteger v = LongInteger(_lexer.getValue());
			tempBuffer[position++] = v >> 24;
			tempBuffer[position++] = (v >> 16) & 0xFF;
			tempBuffer[position++] = (v >> 8) & 0xFF;
			tempBuffer[position++] = v & 0xFF;
#else
			if ((position + 2) >= PROGSTRINGSIZE-1)
				return false;
			const Integer v = Integer(_lexer.getValue());
			tempBuffer[position++] = (v >> 8) & 0xFF;
			tempBuffer[position++] = v & 0xFF;
#endif
			lexerPosition = _lexer.getPointer();
		} else { // Other tokens
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
	const String *line = this->lineByNumber(num, 0);
	if (line != nullptr) {
		const uint16_t index = stringIndex(line);
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

	const uint16_t strLen = sizeof (String) + len;
	// Iterate over
	String *cur;
	for (cur = current(); _current < _textEnd; cur = current()) {
		if (num < cur->number) {
			break;
		} else if (num == cur->number) { // Replace string
			const uint16_t newSize = strLen;
			const uint16_t curSize = cur->size;
			const int16_t dist = long(newSize) - curSize;
			const uint16_t bytes2copy = _arraysEnd -
			    (_current + curSize);
			if ((_arraysEnd + dist) >= _sp)
				return (false);
			memmove(_text + _current + newSize,
			    _text + _current + curSize, bytes2copy);
			cur->number = num;
			cur->size = strLen;
			memcpy(cur->text, text, len);
			_textEnd += dist, _variablesEnd += dist,
			    _arraysEnd += dist;
			return true;
		}
		_current += cur->size;
	}
	return insert(num, text, len);
}

bool
Program::insert(uint16_t num, const char *text, uint8_t len)
{
	assert(len <= PROGSTRINGSIZE);
	const uint8_t strLen = sizeof (String) + len;

	if (_arraysEnd + strLen >= _sp)
		return false;

	memmove(_text + _current + strLen, _text + _current,
	    _arraysEnd - _current);

	String *cur = stringByIndex(_current);
	cur->number = num;
	cur->size = strLen;
	memcpy(cur->text, text, len);
	_textEnd += strLen, _variablesEnd += strLen, _arraysEnd += strLen;
	return true;
}

void
Program::reset(uint16_t size)
{
	_reset();
	if (size > 0)
		_textEnd = _variablesEnd = _arraysEnd = size;
}

uint16_t
Program::size() const
{
	return _textEnd;
}

void
Program::_reset()
{
	_current = _textPosition = 0;
	_sp = programSize;
}

}
