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
#define USE_REALS            0 // Real arithmetics
#define USE_STRINGOPS        0 // Basic string operations (concatenation and comparision)
#define USE_LONGINT          0 // Long integer support
#define USE_DUMP             0 // DUMP command support
#define USE_RANDOM           0 // Use RND and RANDOMIZE
#define CLEAR_PROGRAM_MEMORY 0 // Clear program memory with 0xFF on NEW
#define USE_MATRIX           0 // Matrix operations
#define USE_TEXTATTRIBUTES   0 // Use vt100 text attributes
#if USE_TEXTATTRIBUTES
#define USE_COLORATTRIBUTES  0 // Use vt100 color attributes
#endif
#define USE_EXTEEPROM        0 // External EEPROM functions module
#if USE_EXTEEPROM
#define EXTEEPROM_SIZE    32768 // Size in bytes
#endif
#define USE_SAVE_LOAD        0 // SAVE, LOAD and CHAIN commands support
#if USE_SAVE_LOAD
#define SAVE_LOAD_CHECKSUM   0 // Compute checksums while SAVE, LOAD and CHAIN
#endif
#define USESTOPCONT          0
#define AUTOCAPITALIZE       0 // Convert all input to upper register
#define USE_GFX              0 // GFX module

/**
 * Used modules
 */
#define USESD                0 // SDcard module
#define USEARDUINOIO         0 // Arduino IO module
/*
 * Math module (requires USE_REALS)
 */
#define USEMATH                 0
#if USEMATH
#define M_TRIGONOMETRIC         0 // SIN COS TAN COT
#define M_REVERSE_TRIGONOMETRIC	0 // ACS ASN ATN
#endif

#define OPT_SPEED     1
#define OPT_SIZE      2
#define OPT           OPT_SIZE

/*
 * Input and output for single terminal mode
 */

// Input variants
#define SERIAL_I   0 // Serial input
#define SERIALL_I  1 // SerialL input
#define SERIALL1_I 2 // SerialL1 input
#define SERIALL2_I 3 // SerialL2 input
#define SERIALL3_I 4 // SerialL3 input

// Output variants
#define SERIAL_O   0 // SerialL output
#define SERIALL_O  1 // SerialL output
#define SERIALL1_O 2 // SerialL1 output
#define SERIALL2_O 3 // SerialL2 output
#define SERIALL3_O 4 // SerialL3 output
#define UTFT_O     5 // UTFT output
#define TVOUT_O    6 // TVout output
	#define TVOUT_HORIZ 240
	#define TVOUT_VERT 192

// Input select 
#define S_INPUT SERIALL_I

// Output select
#define S_OUTPUT SERIALL_O

#define USEUTFT		          0
#define USETVOUT	          0

// Use multiterminal mode
#define BASIC_MULTITERMINAL       0
#if BASIC_MULTITERMINAL
#define SERIAL_PORT1 SerialL1
#define SERIAL_PORT2 SerialL2
#define SERIAL_PORT3 SerialL3
#endif

// Use external memory
#define USE_EXTMEM                0
#if USE_EXTMEM
#define EXTMEM_ADDRESS 0x8000
#define EXTMEM_SIZE    32768
#endif

namespace BASIC
{

// Max size of the program line
const uint8_t PROGSTRINGSIZE = 65;

// Number of bytes for program text, variables and stack
#if USE_EXTMEM
const uint16_t PROGRAMSIZE = EXTMEM_SIZE;
#elif defined (__AVR_ATmega1284__) || defined (__AVR_ATmega1284P__)
const uint16_t PROGRAMSIZE = 14848;
#elif defined (__AVR_ATmega2560__)
const uint16_t PROGRAMSIZE = 6144;
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega128A__)
const uint16_t PROGRAMSIZE = 3072;
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
const uint16_t PROGRAMSIZE = 1024;
#elif defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__)
const uint16_t PROGRAMSIZE = 384;
#else
const uint16_t PROGRAMSIZE = 1024;
#endif // USE_EXTMEM

// Max size of the string constants/variables
const uint8_t STRINGSIZE = 34;

// Number of characters in variable name
const uint8_t VARSIZE = 5;

}

#endif // CONFIG_ARDUINO_HPP
