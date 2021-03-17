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

#ifndef CONFIG_ARDUINO_HPP
#define CONFIG_ARDUINO_HPP

#include <stdint.h>

namespace BASIC
{

// Use SDCARD module
#define USESD 0
// Uыe TFT output
#define USEUTFT 0
// Use multiterminal mode
#define BASIC_MULTITERMINAL 0
// Use long integer
#define USE_LONGINT 1
// Tokenize keywords in program text
const bool TOKENIZE = true;
// Max size of the program line
const uint8_t PROGSTRINGSIZE = 72;

// Number of bytes for program text, variables and stack
#if defined (__AVR_ATmega1284__) || defined (__AVR_ATmega1284P__)
const size_t PROGRAMSIZE = 14848;
#elif defined (__AVR_ATmega2560__)
const size_t PROGRAMSIZE = 6144;
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega128A__)
const size_t PROGRAMSIZE = 3072;
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
const size_t PROGRAMSIZE = 1024;	
#endif

// Max size of the string constants/variables
const uint8_t STRINGSIZE = 32;

// Number of characters in variable name
const uint8_t VARSIZE = 8;

}

#endif
