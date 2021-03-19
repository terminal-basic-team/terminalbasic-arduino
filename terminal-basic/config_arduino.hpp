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

/**
 * @file config_arduino.hpp
 * @brief Configuration parameters, specific for Arduino builds
 */

#ifndef CONFIG_ARDUINO_HPP
#define CONFIG_ARDUINO_HPP

#include "config.hpp"
#include "seriallight.hpp"

/**
 * Parameters
 */

#if BASIC_MULTITERMINAL
#define NUM_TERMINALS 1
#ifdef HAVE_HWSERIAL1
#define SERIAL_PORT1 SerialL1
#undef NUM_TERMINALS
#define NUM_TERMINALS 2
#endif
#ifdef HAVE_HWSERIAL2
#define SERIAL_PORT2 SerialL2
#undef NUM_TERMINALS
#define NUM_TERMINALS 3
#endif
#ifdef HAVE_HWSERIAL3
#define SERIAL_PORT3 SerialL3
#undef NUM_TERMINALS
#define NUM_TERMINALS 4
#endif
#endif // BASIC_MULTITERMINAL

// Use external memory
#define USE_EXTMEM                0
#if USE_EXTMEM
#define EXTMEM_ADDRESS 0x8000
#define EXTMEM_SIZE    32768
#endif

namespace BASIC
{
// Number of bytes for program text, variables and stack
#if USE_EXTMEM
const uint16_t PROGRAMSIZE = EXTMEM_SIZE;
#elif defined (__AVR_ATmega1284__) || defined (__AVR_ATmega1284P__)
#if (S_OUTPUT != TVOUT_O)
const uint16_t PROGRAMSIZE = 14336;
#else
const uint16_t PROGRAMSIZE = 8192;
#endif
#elif defined (__AVR_ATmega2560__)
#if (S_OUTPUT != TVOUT_O) && (!USE_EXTMEM) && (!USESD)
const uint16_t PROGRAMSIZE = 6144;
#elif (!USE_EXTMEM) && (USESD)
const uint16_t PROGRAMSIZE = 5900;
#else
const uint16_t PROGRAMSIZE = 900;
#endif
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega128A__)
const uint16_t PROGRAMSIZE = 3072;
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
#if USESD
const uint16_t PROGRAMSIZE = 256;
#else
const uint16_t PROGRAMSIZE = 1024;
#endif
#elif defined (__AVR_ATmega168__) || defined (__AVR_ATmega168P__)
const uint16_t PROGRAMSIZE = 384;
#elif defined __SAM3X8E__
const uint16_t PROGRAMSIZE = 65535;
#else
const uint16_t PROGRAMSIZE = 1024;
#endif // USE_EXTMEM

#if BASIC_MULTITERMINAL
const uint16_t SINGLE_PROGSIZE = PROGRAMSIZE / (NUM_TERMINALS+1);
#else
const uint16_t SINGLE_PROGSIZE = PROGRAMSIZE;
#endif

// BEGIN PRIVATE

#define USEUTFT            0
#define USETVOUT           0
#define USEPS2USARTKB      0
#define USELIQUIDCRYSTAL   0

// END PRIVATE

} // nameespace BASIC

#endif // CONFIG_ARDUINO_HPP
