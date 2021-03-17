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

#ifndef BASIC_HPP
#define BASIC_HPP

#include <stdio.h>
#include <inttypes.h>

#include <Arduino.h>

#ifdef ARDUINO
#include "config_arduino.hpp"
#elif defined __linux__
#include "config_linux.hpp"
#endif

#if S_INPUT == SERIAL_I
    #define SERIAL_PORT_I Serial
#elif S_INPUT == SERIAL1_I
    #define SERIAL_PORT_I Serial1
#elif S_INPUT == SERIALL_I
    #define SERIAL_PORT_I SerialL
#elif S_INPUT == SERIALL1_I
    #define SERIAL_PORT_I SerialL1
#elif S_INPUT == SERIALL2_I
    #define SERIAL_PORT_I SerialL2
#elif S_INPUT == SERIALL3_I
    #define SERIAL_PORT_I SerialL3
#elif S_INPUT == PS2UARTKB_I
#undef USEPS2USARTKB
#define USEPS2USARTKB     1
#elif S_INPUT == SDL_I
    #define USE_SDL_ISTREAM 1
#endif
#if S_OUTPUT == SERIAL_O
#define SERIAL_PORT_O Serial
#elif S_OUTPUT == SERIAL1_O
#define SERIAL_PORT_O Serial1
#elif S_OUTPUT == SERIALL_O
#define SERIAL_PORT_O SerialL
#elif S_OUTPUT == SERIALL3_O
#define SERIAL_PORT_O SerialL3
#elif S_OUTPUT == UTFT_O
#undef USEUTFT
#define USEUTFT            1
#elif S_OUTPUT == TVOUT_O
#undef USETVOUT
#define USETVOUT           1
#elif S_OUTPUT == LIQCR_O
#undef USELIQUIDCRYSTAL
#define USELIQUIDCRYSTAL   1
#endif

#ifdef true
#undef true
#endif

#ifdef false
#undef false
#endif

/**
 * @brief Simple BASIC language interpreter package
 */
namespace BASIC
{
typedef uint16_t Pointer;
// integer type
typedef int16_t Integer;
const Integer MaxInteger = Integer(32767);
#if USE_LONGINT
// long integer type
typedef int32_t LongInteger;
const LongInteger MaxLongInteger = LongInteger(2147483647l);
typedef LongInteger INT;
#define MAXINT MaxLongInteger
#else
typedef Integer INT;
#define MAXINT MaxInteger
#endif
// floating point type
#if USE_REALS
typedef float Real;
#endif

// Number of characters in command/function identifier
const uint8_t IDSIZE = 8;

/**
 * @brief lexical tokens
 */
enum class Token : uint8_t
{
	NOTOKENS = 0,  // 0
	OP_AND,        // 1
#if USE_DUMP
	KW_ARRAYS,     // 2
#endif
//	KW_BASE,       // 3
#if USE_SAVE_LOAD
	COM_CHAIN,     // 4
#endif
#if USE_TEXTATTRIBUTES
	COM_CLS,       // 5
#endif
#if USESTOPCONT
	COM_CONT,      // 6
#endif
#if USE_MATRIX
	KW_CON,        // 7
#endif
#if USE_DATA
	KW_DATA,       // 8
#endif
#if USE_DEFFN
	KW_DEF,        // 9
#endif
#if USE_DELAY
	COM_DELAY,     // 10
#endif
#if USE_MATRIX
	KW_DET,        // 11
#endif
	KW_DIM,        // 12
#if USE_DIV_KW
	KW_DIV,        // 13
#endif
#if USE_DOLOOP
	KW_DO,         // 14
#endif
#if USE_DUMP
	COM_DUMP,      // 15
#endif
	KW_END,        // 16
	KW_FALSE,      // 17
#if USE_DEFFN
	KW_FN,         // 18
#endif
	KW_FOR,        // 19
	KW_GOSUB,      // 20
	KW_GOTO,       // 21
#if CONF_SEPARATE_GO_TO
	KW_GO,         // 22
#endif
#if USE_MATRIX
	KW_IDN,        // 22
#endif
	KW_IF,         // 23
	KW_INPUT,      // 24
#if USE_MATRIX
	KW_INV,        // 25
#endif
	KW_LET,        // 26
	COM_LIST,      // 27
#if USE_SAVE_LOAD
	COM_LOAD,      // 28
#endif
#if USE_TEXTATTRIBUTES
	COM_LOCATE,    // 29
#endif
#if USE_DOLOOP
	KW_LOOP,       // 30
#endif
#if USE_MATRIX
	KW_MAT,        // 31
#endif
#if USE_INTEGER_DIV
	KW_MOD,        // 32
#endif
	COM_NEW,       // 33
	KW_NEXT,       // 34
	OP_NOT,        // 35
	KW_ON,         // 36
//	KW_OPTION,     // 37
	OP_OR,         // 38
	KW_PRINT,      // 39
#if USE_RANDOM
	KW_RANDOMIZE,  // 40
#endif
#if USE_DATA
	KW_READ,       // 41
#endif
	KW_REM,        // 42
#if USE_DATA
	KW_RESTORE,    // 43
#endif
	KW_RETURN,     // 44
	COM_RUN,       // 45
#if USE_SAVE_LOAD
	COM_SAVE,      // 46
#endif
#if CONF_USE_SPC_PRINT_COM
	KW_SPC,        // 47
#endif
	KW_STEP,       // 48
#if USESTOPCONT
	KW_STOP,       // 49
#endif
#if USE_TEXTATTRIBUTES
	KW_TAB,        // 50
#endif
	KW_THEN,       // 51
	KW_TO,         // 52
#if USE_MATRIX
	KW_TRN,        // 53
#endif
	KW_TRUE,       // 54
#if USE_DUMP
	KW_VARS,       // 55
#endif
	OP_XOR,        // 56
#if USE_MATRIX
	KW_ZER,        // 57
#endif
	// *
	STAR,          // 58
	// /
	SLASH,         // 59
#if USE_REALS && USE_INTEGER_DIV
	BACK_SLASH,    // 60
#endif
	// +
	PLUS,          // 61
	// -
	MINUS,         // 62
	// =
	EQUALS,        // 63
	// :
	COLON,         // 64
	// ;
	SEMI,          // 65
	// <
	LT,            // 66
	// >
	GT,            // 67
	// <=
	LTE,           // 68
	// >=
	GTE,           // 69
	// <>
	NE,            // 70
#if CONF_USE_ALTERNATIVE_NE
	//  ><
	NEA,           // 71
#endif
	// ,
	COMMA,         // 71
	// ^
	POW,           // 72
	// (
	LPAREN,        // 73
	// )
	RPAREN,        // 74

	INTEGER_IDENT, // 75
	REAL_IDENT,    // 76
#if USE_LONGINT        // 77
	LONGINT_IDENT, // 78
#endif
	STRING_IDENT,  // 79
	BOOL_IDENT,    // 80

	C_INTEGER,     // 81
	C_REAL,        // 82
	C_BOOLEAN,     // 83
	C_STRING,      // 84

	NUM_TOKENS
};

enum class ProgMemStrings : uint8_t
{
	S_AT = 0,
	S_BYTES,
	S_STATIC,
	S_DYNAMIC,
	S_ERROR,
	S_SEMANTIC,
	S_SYNTAX,
	S_READY,
	AVAILABLE,
	TERMINAL,
	S_TERMINAL_BASIC,
	S_VERSION,
	S_TEXT,
	S_OF,
#if USE_DUMP
	S_VARS,
	S_ARRAYS,
	S_STACK,
#endif
#if USESD
	S_DIR,
#endif
	S_REALLY,
	S_END,
#if USE_TEXTATTRIBUTES
        VT100_ESCSEQ,
	VT100_CLS,
	VT100_NOATTR,
	VT100_BRIGHT,
	VT100_UNDERSCORE,
	VT100_REVERSE,
	VT100_LINEHOME,
#if SET_PRINTZNES
	VT100_CLEARZONES,
	VT100_SETZONE,
#endif
#if USE_COLORATTRIBUTES
	VT100_RED,
	VT100_GREEN,
	VT100_YELLOW,
	VT100_BLUE,
	VT100_MAGENTA,
	VT100_CYAN,
	VT100_WHITE,
#endif // USE_COLORATTRIBUTES
#endif // USE_TEXTATTRIBUTES
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
