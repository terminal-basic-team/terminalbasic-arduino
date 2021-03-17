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

#include "basic.hpp"
#include "arduino_logger.hpp"
#include "basic_program.hpp"
#ifdef ARDUINO
#include "seriallight.hpp"
#endif

#if USEARDUINOIO
#include "basic_arduinoio.hpp"
#endif

#if USESD
#include "basic_sdfs.hpp"
#endif

#if USEUTFT
#include "utft_stream.hpp"
#endif

#if USEMATH
#include "basic_math.hpp"
#endif

#if USETVOUT
#include "TVoutPrint.hpp"
#include "fonts/Font6x8.h"
#endif

#if USE_EXTEEPROM
#include "basic_exteeprom.hpp"
#include <Wire.h>
#endif

/**
 * Instantiating modules
 */

#if USEUTFT
static UTFT	utft(CTE32HR, 38, 39, 40, 41);
static UTFTTerminal utftPrint(utft);
#elif USETVOUT
static uint8_t		tvOutBuf[TVoutEx::bufferSize(TVOUT_HORIZ, TVOUT_VERT)];
static TVoutEx		tvOut;
static TVoutPrint	tvoutPrint;
#endif

#if USESD
static BASIC::SDFSModule sdfs;
#endif

#if USEMATH
static BASIC::Math mathBlock;
#endif

#if USEARDUINOIO
static BASIC::ArduinoIO arduinoIo;
#endif

#if USE_EXTEEPROM
static BASIC::ExtEEPROM extEeprom;
#endif

#if BASIC_MULTITERMINAL
static BASIC::Interpreter::Program program(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic(SERIAL_PORT, SERIAL_PORT, program);
#ifdef HAVE_HWSERIAL1
static BASIC::Interpreter::Program program1(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic1(SERIAL_PORT1, SERIAL_PORT1, program1);
#endif
#ifdef HAVE_HWSERIAL2
static BASIC::Interpreter::Program program2(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic2(SERIAL_PORT2, SERIAL_PORT2, program2);
#endif
#ifdef HAVE_HWSERIAL3
static BASIC::Interpreter::Program program3(BASIC::PROGRAMSIZE / 5);
static BASIC::Interpreter basic3(SERIAL_PORT3, SERIAL_PORT3, program3);
#endif
#else
static BASIC::Interpreter::Program program(BASIC::PROGRAMSIZE);
#if USEUTFT
static BASIC::Interpreter basic(SERIAL_PORT, utftPrint, program);
#elif USETVOUT
static BASIC::Interpreter basic(SERIAL_PORT, tvoutPrint, program);
#else
static BASIC::Interpreter basic(SERIAL_PORT, SERIAL_PORT, program);
#endif // USEUTFT
#endif // BASIC_MULTITERMINAL

void
setup()
{
#if USE_EXTEEPROM
	Wire.begin();
	Wire.setClock(400000);
#endif
#if USE_EXTMEM
	XMCRA |= 1ul<<7; // Switch ext mem iface on
	XMCRB = 0;
#endif
	SERIAL_PORT.begin(115200);
#if USETVOUT
	tvOut.begin(PAL, TVOUT_HORIZ, TVOUT_VERT, tvOutBuf);
        tvOut.selectFont(Font6x8);
#endif
#if USEUTFT
	utftPrint.begin();
#endif
	
#if BASIC_MULTITERMINAL
#ifdef HAVE_HWSERIAL1
	SERIAL_PORT1.begin(115200);
#endif
#ifdef HAVE_HWSERIAL2
	SERIAL_PORT2.begin(115200);
#endif
#ifdef HAVE_HWSERIAL3
	SERIAL_PORT3.begin(115200);
#endif
#endif

	LOG_INIT(SERIAL_PORT);

	LOG_TRACE;

#if USEARDUINOIO
	basic.addModule(&arduinoIo);
#endif
	
#if USEMATH
	basic.addModule(&mathBlock);
#if BASIC_MULTITERMINAL
#ifdef HAVE_HWSERIAL1
	basic1.addModule(&mathBlock);
#endif
#endif // BASIC_MULTITERMINAL
#endif // USEMATH

#if USE_EXTEEPROM
	basic.addModule(&extEeprom);
#endif
	
#if USESD
	basic.addModule(&sdfs);
#endif
	
	basic.init();
#if BASIC_MULTITERMINAL
#ifdef HAVE_HWSERIAL1
	basic1.init();
#endif
#ifdef HAVE_HWSERIAL2
	basic2.init();
#endif
#ifdef HAVE_HWSERIAL3
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
#endif // BASIC_MULTITERMINAL
}
