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
	KW_DIV,
#endif
#if USE_DOLOOP
	KW_DO,         // 13
#endif
#if USE_DUMP
	COM_DUMP,      // 14
#endif
	KW_END,        // 15
	KW_FALSE,      // 16
	KW_FOR,        // 17
	KW_GOSUB,      // 18
	KW_GOTO,       // 19
#if CONF_SEPARATE_GO_TO
	KW_GO,         // 20
#endif
#if USE_MATRIX
	KW_IDN,        // 21
#endif
	KW_IF,         // 22
	KW_INPUT,      // 23
#if USE_MATRIX
	KW_INV,        // 24
#endif
	KW_LET,        // 25
	COM_LIST,      // 26
#if USE_SAVE_LOAD
	COM_LOAD,      // 27
#endif
#if USE_TEXTATTRIBUTES
	COM_LOCATE,
#endif
#if USE_DOLOOP
	KW_LOOP,       // 28
#endif
#if USE_MATRIX
	KW_MAT,        // 28
#endif
#if USE_INTEGER_DIV
	KW_MOD,        // 29
#endif
	COM_NEW,       // 29
	KW_NEXT,       // 30
	OP_NOT,        // 31
//	KW_ON,         // 32
//	KW_OPTION,     // 33
	OP_OR,         // 34
	KW_PRINT,      // 35
#if USE_RANDOM
	KW_RANDOMIZE,  // 36
#endif
#if USE_DATA
	KW_READ,       // 37
#endif
	KW_REM,        // 38
#if USE_DATA
	KW_RESTORE,    // 39
#endif
	KW_RETURN,     // 39
	COM_RUN,       // 40
#if USE_SAVE_LOAD
	COM_SAVE,      // 41
#endif
#if CONF_USE_SPC_PRINT_COM
	KW_SPC,        // 42
#endif
	KW_STEP,       // 43
#if USESTOPCONT
	KW_STOP,       // 44
#endif
#if USE_TEXTATTRIBUTES
	KW_TAB,        // 44
#endif
	KW_THEN,       // 45
	KW_TO,         // 46
#if USE_MATRIX
	KW_TRN,        // 47
#endif
	KW_TRUE,       // 48
#if USE_DUMP
	KW_VARS,       // 49
#endif
#if USE_MATRIX
	KW_ZER,        // 50
#endif

	// *
	STAR,          // 51
	// /
	SLASH,         // 52
#if USE_REALS && USE_INTEGER_DIV
	BACK_SLASH,    // 53
#endif
	// +
	PLUS,          // 54
	// -
	MINUS,         // 55
	// =
	EQUALS,        // 56
	// :
	COLON,         // 57
	// ;
	SEMI,          // 58
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
#if CONF_USE_ALTERNATIVE_NE
	//  ><
	NEA,
#endif
	// ,
	COMMA,
	// ^
	POW,
	// (
	LPAREN,
	// )
	RPAREN,

	INTEGER_IDENT,
	REAL_IDENT,
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
