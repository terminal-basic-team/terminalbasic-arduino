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
#define USE_REALS	1 // Real arithmetics
#define USE_LONGINT	0 // Long integer support

/**
 * Used modules
 */
#define USESD		0 // SDcard
#define USEMATH 	1 // Math (requires USE_REALS)
#define USEARDUINOIO	1 // ARduino IO

// Uыe TFT output
#define USEUTFT		0
// Use multiterminal mode
#define BASIC_MULTITERMINAL 0
// Use external memory
#define USE_EXTMEM	0
#define EXTMEM_ADDRESS 0x8000
#define EXTMEM_SIZE 32768

namespace BASIC
{
// Tokenize keywords in program text
const bool TOKENIZE = true;
// Max size of the program line
const uint8_t PROGSTRINGSIZE = 73;

// Number of bytes for program text, variables and stack
#if USE_EXTMEM
const size_t PROGRAMSIZE = EXTMEM_SIZE;
#elif defined (__AVR_ATmega1284__) || defined (__AVR_ATmega1284P__)
const size_t PROGRAMSIZE = 14848;
#elif defined (__AVR_ATmega2560__)
const size_t PROGRAMSIZE = 4096;
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega128A__)
const size_t PROGRAMSIZE = 3072;
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
const size_t PROGRAMSIZE = 1024;
#elif defined (__AVR_ATmega168__)
const size_t PROGRAMSIZE = 384;
#endif

// Max size of the string constants/variables
const uint8_t STRINGSIZE = 64;

// Number of characters in variable name
const uint8_t VARSIZE = 8;

}

#endif // CONFIG_ARDUINO_HPP
