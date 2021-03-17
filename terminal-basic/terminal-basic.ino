/*
 * ucBASIC is a lightweight BASIC-like language interpreter
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

#include "basic.hpp"
#include "arduino_logger.hpp"
#include "basic_interpreter_program.hpp"
#include "basic_math.hpp"
#include "basic_arduinoio.hpp"

#if USESD
#include "basic_sdfs.hpp"
#endif

#if USEUTFT
#include "utft_stream.hpp"
static UTFT	utft(CTE32HR, 38, 39, 40, 41);
static UTFTTerminal utftPrint(utft);
#endif

#if USESD
static BASIC::SDFSModule sdfs;
static BASIC::Math mathBlock(&sdfs);
#else
static BASIC::Math mathBlock;
#endif
static BASIC::ArduinoIO arduinoIo(&mathBlock);

#if BASIC_MULTITERMINAL
static BASIC::Interpreter::Program program(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic(Serial, Serial, program, &arduinoIo);
#if HAVE_HWSERIAL1
static BASIC::Interpreter::Program program1(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic1(Serial1, Serial1, program1, &arduinoIo);
#endif
#if HAVE_HWSERIAL2
static BASIC::Interpreter::Program program2(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic2(Serial2, Serial2, program2, &arduinoIo);
#endif
#if HAVE_HWSERIAL3
static BASIC::Interpreter::Program program3(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic3(Serial3, Serial3, program3, &arduinoIo);
#endif
#else
static BASIC::Interpreter::Program program(BASIC::PROGRAMSIZE);
#if USEUTFT
static BASIC::Interpreter basic(Serial, utftPrint, program, &arduinoIo);
#else
static BASIC::Interpreter basic(Serial, Serial, program, &arduinoIo);
#endif
#endif

void
setup()
{
#if USE_EXTMEM
	XMCRA |= 1ul<<7; // Switch ext mem iface on
	XMCRB = 0;
#endif
	Serial.begin(57600);
#if USEUTFT
	utftPrint.begin();
#endif
	
#if BASIC_MULTITERMINAL
#if HAVE_HWSERIAL1
	Serial1.begin(57600);
#endif
#if HAVE_HWSERIAL2
	Serial2.begin(57600);
#endif
#if HAVE_HWSERIAL3
	Serial3.begin(57600);
#endif
#endif

	LOG_INIT(Serial);

	LOG_TRACE;
	
	basic.init();
#if BASIC_MULTITERMINAL
#if HAVE_HWSERIAL1
	basic1.init();
#endif
#if HAVE_HWSERIAL2
	basic2.init();
#endif
#if HAVE_HWSERIAL3
	basic3.init();
#endif
#endif
}

void
loop()
{
	LOG_TRACE;
	
	basic.step();
#if BASIC_MULTITERMINAL
#ifdef HAVE_HWSERIAL1
	basic1.step();
#endif
#ifdef HAVE_HWSERIAL2
	basic2.step();
#endif
#ifdef HAVE_HWSERIAL3
	basic3.step();
#endif
#endif
}
