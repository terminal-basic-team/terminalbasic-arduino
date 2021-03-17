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

#ifndef BASIC_HPP
#define BASIC_HPP

#include <stdio.h>
#include <inttypes.h>

#include "helper.hpp"

#ifdef ARDUINO
#include "config_arduino.hpp"
#elif defined __linux__
#include "config_linux.hpp"
#endif

/**
 * @brief Simple BASIC language interpreter package
 */
namespace BASIC
{

// integer type
typedef int16_t Integer;
#if USE_LONGINT
// long integer type
typedef int32_t LongInteger;
typedef LongInteger INT;
#else
typedef Integer INT;
#endif
// floating point type
#if USE_REALS
typedef float Real;
#endif

/**
 * @brief lexical tokens
 */
enum class Token : uint8_t
{
	NOTOKENS = 0,
	OP_AND,        // 1
#if USE_DUMP
	KW_ARRAYS,     // 1
#endif
	KW_BASE,       // 2
#if USE_SAVE_LOAD
	COM_CHAIN,     // 3
#endif
	COM_CLS,       // 4
	KW_DATA,       // 5
	KW_DEF,        // 6
//	COM_DELAY,     // 7
	KW_DIM,        // 8
#if USE_DUMP
	COM_DUMP,      // 9
#endif
	KW_END,        // 10
	KW_FALSE,      // 11
	KW_FOR,        // 12
	KW_GOSUB,      // 13
	KW_GOTO,       // 14
	KW_GO,         // 15
#if USE_MATRIX
	KW_IDN,
#endif
	KW_IF,         // 16
	KW_INPUT,      // 17
	KW_LET,        // 18
	COM_LIST,      // 19
#if USE_SAVE_LOAD
	COM_LOAD,      // 20
#endif
#if USE_MATRIX
	KW_MAT,
#endif
	COM_NEW,       // 21
	KW_NEXT,
	OP_NOT,
	KW_ON,
	KW_OPTION,
	OP_OR,
	KW_PRINT,
#if USE_RANDOM
	KW_RANDOMIZE,
#endif
	KW_READ,
	KW_REM,
	KW_RETURN,
	COM_RUN,
#if USE_SAVE_LOAD
	COM_SAVE,
#endif
	KW_STEP,
	KW_STOP,
	KW_TAB,
	KW_THEN,
	KW_TO,
	KW_TRUE,
#if USE_DUMP
	KW_VARS,
#endif
#if USE_MATRIX
	KW_ZER,
#endif

	// *
	STAR,
	// /
	SLASH,
	// +
	PLUS,
	// -
	MINUS,
	// =
	EQUALS,
	// :
	COLON,
	// ;
	SEMI,
	// <
	LT,
	// >
	GT,
	// <=
	LTE,
	// >=
	GTE,
	// <>
	NE,
	//  ><
	NEA,
	// ,
	COMMA,
	// ^
	POW,
	// (
	LPAREN,
	// )
	RPAREN,

	REAL_IDENT,
	INTEGER_IDENT,
#if USE_LONGINT
	LONGINT_IDENT,
#endif
	STRING_IDENT,
	BOOL_IDENT,

	C_INTEGER,
	C_REAL,
	C_BOOLEAN,
	C_STRING,

	NUM_TOKENS
};

enum class ProgMemStrings : uint8_t
{
	S_STATIC = 0,
	S_DYNAMIC,
	S_ERROR,
	S_SEMANTIC,
	S_AT,
	S_READY,
	BYTES,
	AVAILABLE,
	TERMINAL,
	S_TERMINAL_BASIC,
	S_VERSION,
	S_TEXT,
	S_OF,
	S_VARS,
	S_ARRAYS,
	S_STACK,
	S_DIR,
	S_REALLY,
	S_END,
        VT100_ESCSEQ,
	VT100_CLS,
	VT100_NOATTR,
	VT100_BRIGHT,
	VT100_UNDERSCORE,
	VT100_REVERSE,
#if USE_COLORATTRIBUTES
	VT100_RED,
	VT100_GREEN,
	VT100_YELLOW,
	VT100_BLUE,
	VT100_MAGENTA,
	VT100_CYAN,
	VT100_WHITE,
#endif
	NUM_STRINGS
};

// Static text strings
extern PGM_P progmemString(ProgMemStrings);

/**
 * @brief Scan token table
 * @param token
 * @param table
 * @param index
 * @return find flag
 */
uint8_t *scanTable(const uint8_t*, const uint8_t[], uint8_t&);

}

#endif // BASIC_HPP
