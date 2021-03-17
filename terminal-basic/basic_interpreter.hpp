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

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "basic.hpp"
#include "basic_lexer.hpp"
#include "basic_parser.hpp"
#include "helper.hpp"
#include "vt100.hpp"

namespace BASIC
{

/**
 * @brief Interpreter context object
 */
class Interpreter
{
public:
	/**
	 * @brief BASIC program memory
	 */
	class CPS_PACKED Program;

	/**
	 * Variable type
	 */
	enum Type : uint8_t
	{
		VF_INTEGER = 0,
#if USE_LONGINT
		VF_LONG_INTEGER,
#endif
#if USE_REALS
		VF_REAL,
#endif
		VF_BOOLEAN,
		VF_STRING
	};

	/**
	 * Dynamic (runtime error codes)
	 */
	enum ErrorCodes : uint8_t
	{
		NO_ERROR = 0,		// Ok
		OUTTA_MEMORY,		// Out of memory
		REDIMED_ARRAY,		// Attempt to define existing array
		STACK_FRAME_ALLOCATION,	// Unable to allocate stack frame
		ARRAY_DECLARATION,	// 
		STRING_FRAME_SEARCH,	// Missing string frame
		INVALID_NEXT,		// 
		RETURN_WO_GOSUB,
		NO_SUCH_STRING,
		INVALID_VALUE_TYPE,
		NO_SUCH_ARRAY,
		INTEGER_EXPRESSION_EXPECTED, // Integer expression expected
		BAD_CHECKSUM,		// Bad program checksum
		INVALID_TAB_VALUE,
		INTERNAL_ERROR = 255
	};

	/**
	 * Type of the occured error
	 */
	enum ErrorType : uint8_t
	{
		STATIC_ERROR, // syntax
		DYNAMIC_ERROR // runtime
	};

	/**
	 * @brief variable memory frame
	 */
	struct CPS_PACKED VariableFrame
	{
		/**
		 * @brief size of the initialized frame
		 * @return size in bytes
		 */
		uint8_t size() const;

		/**
		 * @brief getValue from Variable frame
		 * @param T value type
		 * @return value
		 */
		template <typename T>
		T get() const
		{
			union
			{
				const char *b;
				const T *i;
			} _U;
			_U.b = bytes;
			return *_U.i;
		}

		// Variable name
		char name[VARSIZE];
		// Variable type
		Type type;
		// Frame body
		char bytes[];
	};

	/**
	 * Array memory frame
	 */
	struct ArrayFrame
	{
		/**
		 * @brief get frame size in bytes
		 * @return size
		 */
		size_t size() const;

		/**
		 * @brief get array raw data pointer
		 * @return pointer
		 */
		uint8_t *data()
		{
			return (reinterpret_cast<uint8_t*> (this+1) +
			    sizeof (size_t) * numDimensions);
		}

		/**
		 * @brief Overloaded version
		 */
		const uint8_t *data() const
		{
			return (reinterpret_cast<const uint8_t*> (this+1) +
			    sizeof (size_t) * numDimensions);
		}

		/**
		 * @brief get array value by raw index
		 * @param index shift in array data
		 * @return value
		 */
		template <typename T>
		T get(size_t index) const
		{
			union
			{
				const uint8_t *b;
				const T *i;
			} _U;
			_U.b = this->data();
			return _U.i[index];
		}

		// Array data
		char name[VARSIZE];
		// Array type
		Type type;
		// Number of dimensions
		uint8_t numDimensions;
		// Actual dimensions values
		size_t dimension[];
	};
	// Interpreter FSM state

	enum State : uint8_t
	{
		SHELL,		// Wait for user input of line or command
		PROGRAM_INPUT,	// 
		COLLECT_INPUT,	//
		EXECUTE,	// Runniong the program
		VAR_INPUT,	// Input of the variable value
		GET_VAR_VALUE,
		CONFIRM_INPUT	// Input of the confirmation
	};
#if USE_DUMP
	// Memory dump modes
	enum DumpMode : uint8_t
	{
		MEMORY, VARS, ARRAYS
	};
#endif
	
	/**
	 * @brief constructor
	 * @param stream Boundary output object
	 * @param print Boundary input object
	 * @param program Program object
	 */
	explicit Interpreter(Stream&, Print&, Program&);
	
	/**
	 * [re]initialize interpreter object
	 */
	void init();
	// Interpreter cycle: request a string or execute one operator
	void step();
	// Execute entered command (command or inputed program line)
	void exec();
	// Clear screen
	void cls();
	// Output program memory
	void list(uint16_t = 1, uint16_t = 0);
#if USE_DUMP
	// Dump program memory
	void dump(DumpMode);
#endif
	// Add module on tail of the modules list
	void addModule(FunctionBlock*);

	// New print line
	void newline();
	void print(char);
#if USE_REALS
	void print(Real);
#endif

	void print(Integer, VT100::TextAttr = VT100::NO_ATTR);
	void printTab(Integer);
	void print(long, VT100::TextAttr = VT100::NO_ATTR);
	void print(ProgMemStrings, VT100::TextAttr = VT100::NO_ATTR);
	void print(Token);
	void print(const char *, VT100::TextAttr = VT100::NO_ATTR);
	// print value
	void print(const Parser::Value&, VT100::TextAttr = VT100::NO_ATTR);

	// run program
	void run();
	// goto new line
	void gotoLine(const Parser::Value&);
	// CLear program memory
	void newProgram();
	/**
	 * save current line on stack
	 * @param text position
	 */
	void pushReturnAddress(uint8_t);
	// return from subprogram
	void returnFromSub();
	// save for loop
	void pushForLoop(const char*, uint8_t, const Parser::Value&,
	    const Parser::Value&);
	void pushValue(const Parser::Value&);
	void pushInputObject(const char*);
	bool popValue(Parser::Value&);
	bool popString(const char*&);
	void randomize();
	/**
	 * @brief iterate over loop
	 * @param varName loop variable name
	 * @return loop end flag
	 */
	bool next(const char*);

	// Internal EEPROM commands
#if USE_SAVE_LOAD

	struct EEpromHeader_t
	{
		uint16_t len;
		uint16_t magic_FFFFminuslen;
		uint16_t crc16;
	};
	void save();
	void load();
	void chain();
#endif // USE_SAVE_LOAD
	/**
	 * @breif Input variables
	 */
	void input();

	void end();
	/**
	 * @brief set value to initialized object
	 * @param f frame to set to
	 * @param v value to set
	 */
	void set(VariableFrame&, const Parser::Value&);
	void set(ArrayFrame&, size_t, const Parser::Value&);
	/**
	 * @brief set a new value and possibly create new variable
	 * @param name variable name
	 * @param v value to assign
	 */
	Interpreter::VariableFrame *setVariable(const char*,
	    const Parser::Value&);
	/**
	 * @brief setarray element a given value with indexes on the stack
	 * @param name array name
	 * @param value value to set
	 */
	void setArrayElement(const char*, const Parser::Value&);
	/**
	 * @brief create array
	 * @param name array name
	 */
	void newArray(const char*);
	/**
	 * @brief get variable frame pointer (or create new one)
	 * @param name variable name
	 * @return frame pointer
	 */
	const VariableFrame *getVariable(const char*);

	void valueFromVar(Parser::Value&, const char*);

	bool valueFromArray(Parser::Value&, const char*);

	/**
	 * @brief push string constant on the stack
	 */
	void pushString(const char*);
	/**
	 * @brief push the next array dimesion on the stack
	 * @param 
	 * @return 
	 */
	size_t pushDimension(size_t);
	/**
	 * @brief push the number of array dimesions on the stack
	 * @param num number of dimensions
	 */
	void pushDimensions(uint8_t);

	void strConcat(Parser::Value&, Parser::Value&);
	/**
	 * @brief request user confirmation
	 * @return 
	 */
	bool confirm();

	void stop()
	{
		_parser.stop();
	}

	Program &_program;
private:
	class AttrKeeper;
	// Get next input object from stack
	bool nextInput();
	// Place input values to objects
	void doInput();

	void print(Lexer&);

	void raiseError(ErrorType, ErrorCodes = NO_ERROR);
	/**
	 * @brief read and buffer one symbol
	 * @return input finished flag
	 */
	bool readInput();
	/**
	 * @brief Add new array frame
	 * @param name name of the array (also defines type of the elements)
	 * @param dim number of dimensions
	 * @param num overall elements number
	 * @return 
	 */
	ArrayFrame *addArray(const char*, uint8_t, uint32_t);

	bool arrayElementIndex(ArrayFrame*, size_t&);
#if USE_SAVE_LOAD
	/**
	 * @brief Check program text
	 * @param len Length of the program
	 * @return Flag of success
	 */
	bool checkText(uint16_t&);
	/**
	 * @brief load program memory from eeprom
	 * @param len Length of program
	 * @param showPogress show loading progress
	 */
	void loadText(uint16_t, bool=true);
#if SAVE_LOAD_CHECKSUM
	uint16_t eepromProgramChecksum(uint16_t);
#endif
#endif // USE_SAVE_LOAD
	// Interpreter FSM state
	State			 _state;
	// Input oject
	Stream			&_input;
	// Output object
	Print			&_output;
	// Lexical analyzer object
	Lexer			 _lexer;
	// Syntactical analyzer object
	Parser			 _parser;
	//Input buffer
	char			 _inputBuffer[PROGSTRINGSIZE];
	// Position in the input buffer
	uint8_t			 _inputPosition;
	// Input variable name string;
	char			 _inputVarName[VARSIZE];
	static uint8_t		 _termnoGen;
	uint8_t			 _termno;
};

}

#endif
