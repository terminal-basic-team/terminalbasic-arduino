/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016-2019 Andrey V. Skvortsov <starling13@mail.ru>
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

#ifndef BASIC_COMMON_HPP
#define BASIC_COMMON_HPP

#include <stdio.h>
#include <inttypes.h>

#include <Arduino.h>

#include "basic.hpp"

#ifdef ARDUINO
#include "config_arduino.hpp"
#elif defined __linux__
#include "config_linux.hpp"
#endif

#if S_INPUT == SERIAL_I
    #define SERIAL_PORT_I Serial
#elif S_INPUT == SERIAL1_I
    #define SERIAL_PORT_I Serial1
#elif S_INPUT == SERIAL2_I
    #define SERIAL_PORT_I Serial2
#elif S_INPUT == SERIAL3_I
    #define SERIAL_PORT_I Serial3
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
#elif S_OUTPUT == SERIAL2_O
#define SERIAL_PORT_O Serial2
#elif S_OUTPUT == SERIAL3_O
#define SERIAL_PORT_O Serial3
#elif S_OUTPUT == SERIALL_O
#define SERIAL_PORT_O SerialL
#elif S_OUTPUT == SERIALL1_O
#define SERIAL_PORT_O SerialL1
#elif S_OUTPUT == SERIALL2_O
#define SERIAL_PORT_O SerialL2
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
#endif // USE_LONGINT
// floating point type
#if USE_REALS == REAL_SINGLE
typedef float Real;
#elif USE_REALS == REAL_DOUBLE
typedef double Real;
#endif

// Number of characters in command/function identifier
const uint8_t IDSIZE = 8;

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
#endif // USE_DUMP
#if USESD
	S_DIR,
#endif // USESD
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
#endif // SET_PRINTZNES
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

} // namespace BASIC

#endif // BASIC_COMMON_HPP
