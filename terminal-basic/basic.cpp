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

namespace BASIC
{

static const char strStatic[] PROGMEM = "STATIC";
static const char strDynamic[] PROGMEM = "DYNAMIC";
static const char strError[] PROGMEM = "ERROR";
static const char strSemantic[] PROGMEM = "SEMANTIC";
static const char strReady[] PROGMEM = "READY";
static const char strBytes[] PROGMEM = "BYTES";
static const char strAvailable[] PROGMEM = "AVAILABLE";
static const char strucTERMINAL[] PROGMEM = "TERMINAL";
static const char strucBASIC[] PROGMEM = "BASIC";
static const char strVERSION[] PROGMEM = "VERSION";
static const char strTEXT[] PROGMEM = "TEXT";
static const char strOF[] PROGMEM = "OF";
static const char strVARS[] PROGMEM = "VARS";
static const char strARRAYS[] PROGMEM = "ARRAYS";
static const char strSTACK[] PROGMEM = "STACK";
static const char strDIR[] PROGMEM = "DIR";
static const char strREALLY[] PROGMEM = "REALLY";
static const char strEND[] PROGMEM = "END";

static PGM_P const progmemStrings[uint8_t(ProgMemStrings::NUM_STRINGS)] PROGMEM = {
	strStatic, // STATIC
	strDynamic, // DYNAMAIC
	strError, // ERROR
	strSemantic, // SEMANTIC
	strReady, // READY
	strBytes, // BYTES
	strAvailable, // AVAILABLE
	strucTERMINAL, // TERMINAL
	strucBASIC, // BASIC
	strVERSION, // VERSION
	strTEXT, // TEXT
	strOF, // OF
	strVARS, // VARS
	strARRAYS, // ARRAYS
	strSTACK, // STACK
	strDIR, // DIR
	strREALLY, // REALLY
	strEND
};

bool
scanTable(const uint8_t *token, const uint8_t table[], uint8_t &index)
{
	uint8_t tokPos = 0, tabPos = 0;
	while (token[tokPos] != 0) {
		uint8_t c = pgm_read_byte(table);
		uint8_t ct = token[tokPos];
		if (c == 0)
			return (false);
		
		if (ct == c)
			++tokPos, ++table;
		else if (ct+uint8_t(0x80) == c) {
			index = tabPos;
			if (token[++tokPos] != 0) {
				while ((pgm_read_byte(table++) & uint8_t(0x80)) ==
				    0);
				++tabPos, tokPos=0;
			} else
				return (true);
		} else {
			if (c & uint8_t(0x80))
				c &= ~uint8_t(0x80);
			if (c > ct)
				return (false);
			else {
				while ((pgm_read_byte(table++) & uint8_t(0x80)) ==
				    0);
				++tabPos, tokPos=0;
			}
		}
	}
	return (false);
}

PGM_P progmemString(ProgMemStrings index)
{
	return ((PGM_P)pgm_read_word(&progmemStrings[uint8_t(index)]));
}

}
