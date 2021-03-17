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

#ifndef CONFIG_ARDUINO_HPP
#define CONFIG_ARDUINO_HPP

#include <stdint.h>

/**
 * Paraeters
 */
#define USE_REALS            1 // Real arithmetics
#define USE_LONGINT          0 // Long integer support
#define USE_DUMP             1 // DUMP command support
#define CLEAR_PROGRAM_MEMORY 1 // Clear program memory with 0xFF on NEW

#define USE_SAVE_LOAD        1 // SAVE, LOAD and CHAIN commands support
#if USE_SAVE_LOAD
#define SAVE_LOAD_CHECKSUM   0 // Compute checksums while SAVE, LOAD and CHAIN
#endif

/**
 * Used modules
 */
#define USESD                0 // SDcard module
#define USEARDUINOIO         1 // Arduino IO module
/*
 * Math module (requires USE_REALS)
 */
#define USEMATH                 1
#if USEMATH
#define M_TRIGONOMETRIC         1 // SIN COS TAN COT
#define M_REVERSE_TRIGONOMETRIC	0 // ACS ASN ATN
#endif

// Input variants
#define SERIAL_I 0 // Serial output

// Output variants
#define SERIAL_O 0 // Serial output
#define UTFT_O   1 // UTFT output
#define TVOUT_O  2 // TVout output

// Input select (SERIAL)
#define S_INPUT SERIAL_I

// Output select
#define S_OUTPUT SERIAL_O

#define USEUTFT		          0
#define USETVOUT	          0

#if S_OUTPUT == UTFT_O
#undef USEUTFT
#define USEUTFT		          1
#elif S_OUTPUT == TVOUT_O
#undef USETVOUT
#define USETVOUT	          1
#endif

// Use multiterminal mode
#define BASIC_MULTITERMINAL       0

// Use external memory
#define USE_EXTMEM                0
#if USE_EXTMEM
#define EXTMEM_ADDRESS 0x8000
#define EXTMEM_SIZE    32768
#endif

namespace BASIC
{
// Tokenize keywords in program text
const bool TOKENIZE = true;
// Max size of the program line
const uint8_t PROGSTRINGSIZE = 64;

// Number of bytes for program text, variables and stack
#if USE_EXTMEM
const size_t PROGRAMSIZE = EXTMEM_SIZE;
#elif defined (__AVR_ATmega1284__) || defined (__AVR_ATmega1284P__)
const size_t PROGRAMSIZE = 14848;
#elif defined (__AVR_ATmega2560__)
const size_t PROGRAMSIZE = 6144;
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega128A__)
const size_t PROGRAMSIZE = 3072;
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
const size_t PROGRAMSIZE = 1000;
#elif defined (__AVR_ATmega168__)
const size_t PROGRAMSIZE = 384;
#endif

// Max size of the string constants/variables
const uint8_t STRINGSIZE = 32;

// Number of characters in variable name
const uint8_t VARSIZE = 5;

}

#endif // CONFIG_ARDUINO_HPP