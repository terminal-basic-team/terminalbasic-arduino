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

#ifndef BASIC_INTERPRETER_PROGRAM_HPP
#define BASIC_INTERPRETER_PROGRAM_HPP

#include "arduinoext.hpp"
#include "basic.hpp"
#include "basic_parser_value.hpp"

namespace BASIC
{
	
class VariableFrame;
class ArrayFrame;
class Interpreter;

/**
 * @brief BASIC program in memory
 */
class Program
{
	friend class Interpreter;
public:

	/**
	 * @brief BASIC program string object
	 */
	struct EXT_PACKED Line
	{
		// string decimal number (label)
		uint16_t number;
		// size in bytes
		uint8_t size;
		// string body
		char text[];
	};

	/**
	 * @program stack frame object
	 */
	struct EXT_PACKED StackFrame
	{
		/**
		 * @brief Stack frame type
		 */
		enum Type : uint8_t
		{
			// Subprogram return address
			SUBPROGRAM_RETURN,
			// FOR-loop state frame
			FOR_NEXT,
			// String frame
			STRING,
			// One array dimension
			ARRAY_DIMENSION,
			// Number of array dimensions
			ARRAY_DIMENSIONS,
			// Value frame
			VALUE,
			// Input object frame
			INPUT_OBJECT
		};

		/**
		 * @brief FOR-loop state frame body
		 */
		struct EXT_PACKED ForBody
		{
			// Program counter on loop begin
			uint16_t	calleeIndex;
			// Loop begin position in the program string
			uint8_t		textPosition;
			// Loop variable name
			char		varName[VARSIZE];
			// Current value of the loop variable
			Parser::Value	currentValue;
			// Loop step value
			Parser::Value	stepValue;
			// Loop final value
			Parser::Value	finalvalue;
		};
		static_assert (sizeof (ForBody) <= UINT8_MAX, "bad size");
		
		/**
		 * @brief Subprogram return address frame body
		 */
		struct EXT_PACKED GosubReturn
		{
			// Program counter of the colee string
			uint16_t calleeIndex;
			// Position in the program string
			uint8_t	textPosition;
		};
		
		/**
		 * @brief Input object frame body
		 */
		struct EXT_PACKED InputBody
		{
			enum Type : uint8_t
			{
				INPUT_VAR, INPUT_ARR_ELM
			};
			// Program counter of the colee string
			Type	type;
			char	name[VARSIZE];
		};

		static uint8_t size(Type);
		
		uint8_t size() const { return size(this->_type); }

		Type _type;

		union EXT_PACKED Body
		{
			GosubReturn	gosubReturn;
			uint8_t		arrayDimensions;
			uint16_t	arrayDimension;
			ForBody		forFrame;
			InputBody	inputObject;
			char		string[STRINGSIZE];
			Parser::Value	value;
		};
		Body body;
	};

	Program(uint16_t = PROGRAMSIZE);
	/**
	 * @brief Clear program text, but not vars and arrays
	 */
	void clearProg();
	/**
	 * @brief Move program state (variables and arrays) to dest
	 * @param dest New Data start
	 */
	void moveData(uint16_t);
	/**
	 * @brief Clear program memory
	 */
	void newProg();
	/**
	 * @brief reset program 
	 * @brief newSize 0 if use existing program, text size if clear vars
	 *   and arrays
	 */
	void reset(uint16_t = 0);
	/**
	 * @brief get actual size of stored program in bytes
	 * @return program size
	 */
	uint16_t size() const;
	/**
	 * @brief get next program line
	 * @return line object or nullptr if beyond last line
	 */
	Line *getString();
	/**
	 * @brief Get current program line to be executed
	 * @return pointer to current program line
	 */
	Line *current() const;
	/**
	 * @brief Get first stored program line
	 * @return 
	 */
	Line *first() const;
	
	Line *last() const;
	
	void jump(uint16_t newVal);
	/**
	 * @brief program line at given index
	 * @param index
	 * @return string pointer or nullptr if not exists
	 */
	Line *lineByIndex(uint16_t) const;
	/**
	 * @brief program line of given number
	 * @param number Program line number to get
	 * @param index start of the search
	 * @return string pointer or NULL if not found
	 */
	Line *lineByNumber(uint16_t, uint16_t = 0);
	/**
	 * @brief 
	 * @param string pointer
	 * @return index
	 */
	uint16_t stringIndex(const Line*) const;
	/**
	 * @brief get variable frame at a given index
	 * @param index basic memory address
	 * @return pointer
	 */
	VariableFrame *variableByIndex(uint16_t);
	VariableFrame *variableByName(const char*);
	uint16_t variableIndex(VariableFrame*) const;
	
	ArrayFrame *arrayByIndex(uint16_t);
	ArrayFrame *arrayByName(const char*);
	uint16_t arrayIndex(ArrayFrame*) const;

	StackFrame *stackFrameByIndex(uint16_t index);
	StackFrame *currentStackFrame();
	/**
	 * @brief create new stack frame of given type and get its pointer
	 * @param type
	 */
	StackFrame *push(StackFrame::Type);
	/**
	 * @brief pop top value
	 */
	void pop();
	/**
	 * @brief reverse order of last same type elements
	 */
	void reverseLast(StackFrame::Type);
	/**
	 * @brief Add new Program line
	 * @param number decimal line number
	 * @param text
	 * @return flag of success
	 */
	bool addLine(uint16_t, const char*);
	/**
	 * @brief Remove program line
	 * @param num line number
	 */
	void removeLine(uint16_t);
	/**
	 * @brief Insert line at current position
	 * @param num line number
	 * @param text line text
	 */
	bool insert(uint16_t, const char*, uint8_t);
	
	uint16_t textEnd() const { return _textEnd; }
	void setTextEnd(uint16_t newVal)
	{
		_textEnd = newVal;
	}
	
	uint16_t varsEnd() const { return _variablesEnd; }
	void setVarsEnd(uint16_t newVal)
	{
		_variablesEnd = newVal;
	}
	
	uint16_t arraysEnd() const { return _arraysEnd; }
	void setArraysEnd(uint16_t newVal)
	{
		_arraysEnd = newVal;
	}
	
	uint16_t sp() const { return _sp; }
	void setSP(uint16_t newVal)
	{
		_sp = newVal;
	}
	
#if USE_EXTMEM
	char *_text;
#else
	char _text[PROGRAMSIZE];
#endif
	const uint16_t programSize;
private:
	
	void _reset();
	
	void pushBottom(StackFrame*);
	/**
	 * @brief Add tokenized program line
	 * @param num line number
	 * @param text line body w/o number
	 * @param len line body length
	 * @return flag of success
	 */
	bool addLine(uint16_t, const char*, uint16_t);
	// End of program text
	uint16_t _textEnd;
	// End of variables area
	uint16_t _variablesEnd;
	uint16_t _arraysEnd, _sp;
	// Current line index
	uint16_t _current;
	// Position in current line
	uint8_t _textPosition;
	// Jump flag
	bool _jumpFlag;
	// Jump pointer
	uint16_t _jump;
};

}

#endif
