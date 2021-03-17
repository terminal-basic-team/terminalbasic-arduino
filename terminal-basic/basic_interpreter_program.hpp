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

#ifndef BASIC_INTERPRETER_PROGRAM_HPP
#define BASIC_INTERPRETER_PROGRAM_HPP

#include "basic_interpreter.hpp"

namespace BASIC
{

class CPS_PACKED Interpreter::Program
{
	friend class Interpreter;
public:

	/**
	 * BASIC string
	 * @return 
	 */
	struct CPS_PACKED String
	{
		uint16_t number;
		uint8_t size;
		char text[];
	};

	struct CPS_PACKED StackFrame
	{
		enum Type : uint8_t
		{
			SUBPROGRAM_RETURN, FOR_NEXT, STRING, ARRAY_DIMENSION,
			ARRAY_DIMENSIONS, VALUE
		};

		struct CPS_PACKED ForBody
		{
			size_t calleeIndex;
			uint8_t textPosition;
			char varName[VARSIZE];
			Parser::Value current;
			Parser::Value step;
			Parser::Value finalv;
		};
		static_assert (sizeof (ForBody) <= UINT8_MAX, "bad size");
		
		struct CPS_PACKED GosubReturn
		{
			size_t calleeIndex;
			uint8_t textPosition;
		};

		static uint8_t size(Type);

		Type _type;

		union CPS_PACKED
		{
			GosubReturn	gosubReturn;
			uint8_t		arrayDimensions;
			size_t		arrayDimension;
			ForBody		forFrame;
			char		string[STRINGSIZE];
			Parser::Value	value;
		} body;
	};

	Program(size_t=PROGRAMSIZE);
	/**
	 * @brief Clear program memory
	 */
	void newProg();
	/**
	 * @brief reset program 
	 */
	void reset(size_t=0);
	
	size_t size() const;

	String *getString();

	String *current() const;
	String *first() const;
	String *last() const;
	
	void jump(size_t newVal) { _jump = newVal; _jumpFlag = true; }
	/**
	 * @brief program string at given index
	 * @param index
	 * @return string pointer or NULL if not exists
	 */
	String *stringByIndex(size_t) const;
	/**
	 * @brief program string of given number
	 * @param number
	 * @param index
	 * @return string pointer or NULL if not found
	 */
	String *stringByNumber(size_t, size_t = 0);
	/**
	 * @brief 
	 * @param string pointer
	 * @return index
	 */
	size_t stringIndex(const String*) const;
	/**
	 * @brief get variable frame at a given index
	 * @param index basic memory address
	 * @return pointer
	 */
	VariableFrame *variableByIndex(size_t);
	VariableFrame *variableByName(const char*);
	size_t variableIndex(VariableFrame*) const;
	
	ArrayFrame *arrayByIndex(size_t);
	ArrayFrame *arrayByName(const char*);
	size_t arrayIndex(ArrayFrame*) const;

	StackFrame *stackFrameByIndex(size_t index);
	StackFrame *currentStackFrame();
	
	/**
	 * @brief create new stack frame of given type and get its pointer
	 * @param type
	 */
	StackFrame *push(StackFrame::Type);
	void pop();
	/**
	 * @brief Add new Program line
	 * @param number decimal line number
	 * @param text
	 */
	bool addLine(uint16_t, const char*);
	/**
	 * @brief Insert line at current position
	 * @param num line number
	 * @param text line text
	 */
	bool insert(uint16_t, const char*);
	char *_text;
	const size_t programSize;
private:
	// End of program text
	size_t _textEnd;
	size_t _current, _variablesEnd, _arraysEnd, _sp, _jump;
	bool _jumpFlag;
	uint8_t _textPosition;
};

}

#endif
