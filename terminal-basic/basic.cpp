/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
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

#if (LANG == LANG_RU)
#include "strings_ru_koi8r.hpp"
#elif (LANG == LANG_EN)
#include "strings_en.hpp"
#endif

static const char strStatic[] PROGMEM = "STATIC";
static const char strDynamic[] PROGMEM = "DYNAMIC";
static const char strError[] PROGMEM = STR_ERROR;
static const char strSemantic[] PROGMEM = STR_SEMANTIC;
static const char strSyntax[] PROGMEM = STR_SYNTAX;
static const char strAt[] PROGMEM = "AT";
static const char strReady[] PROGMEM = STR_CLI_PROMPT;
static const char strBytes[] PROGMEM = STR_BYTES;
static const char strAvailable[] PROGMEM = STR_AVAILABLE;
static const char strucTERMINAL[] PROGMEM = "TERMINAL";
static const char strucBASIC[] PROGMEM = "BASIC";
static const char strVERSION[] PROGMEM = STR_VERSION;
static const char strTEXT[] PROGMEM = "TEXT";
static const char strOF[] PROGMEM = "OF";
#if USE_DUMP
static const char strVARS[] PROGMEM = "VARS";
static const char strARRAYS[] PROGMEM = "ARRAYS";
static const char strSTACK[] PROGMEM = "STACK";
#endif
#if USESD
static const char strDIR[] PROGMEM = "DIR";
#endif
static const char strREALLY[] PROGMEM = "REALLY";
static const char strEND[] PROGMEM = "END";
#if USE_TEXTATTRIBUTES
static const char strVT100_PROLOGUESEQ[] PROGMEM = "\x1B[";
static const char strVT100_CLS[] PROGMEM = "2J";
static const char strVT100_NOATTR[] PROGMEM = "0m";
static const char strVT100_BRIGHT[] PROGMEM = "1m";
static const char strVT100_UNDERSCORE[] PROGMEM = "4m";
static const char strVT100_REVERSE[] PROGMEM = "7m";
static const char strVT100_LINEHOME[] PROGMEM = "80D";
#if SET_PRINTZNES
static const char strVT100_CLEARZONES[] PROGMEM = "3g";
static const char strVT100_SETZONE[] PROGMEM = "\x1BH";
#endif
#if USE_COLORATTRIBUTES
static const char strVT100_RED[] PROGMEM = "31m";
static const char strVT100_GREEN[] PROGMEM = "32m";
static const char strVT100_YELLOW[] PROGMEM = "33m";
static const char strVT100_BLUE[] PROGMEM = "34m";
static const char strVT100_MAGENTA[] PROGMEM = "35m";
static const char strVT100_CYAN[] PROGMEM = "36m";
static const char strVT100_WHITE[] PROGMEM = "37m";
#endif
#endif // USE_TEXTATTRIBUTES

static PGM_P const progmemStrings[uint8_t(ProgMemStrings::NUM_STRINGS)] PROGMEM = {
	strAt,     // AT
	strBytes, // BYTES
	strStatic, // STATIC
	strDynamic, // DYNAMAIC
	strError, // ERROR
	strSemantic, // SEMANTIC
	strSyntax, // SYNTAX
	strReady, // READY
	strAvailable, // AVAILABLE
	strucTERMINAL, // TERMINAL
	strucBASIC, // BASIC
	strVERSION, // VERSION
	strTEXT, // TEXT
	strOF, // OF
#if USE_DUMP
	strVARS, // VARS
	strARRAYS, // ARRAYS
	strSTACK, // STACK
#endif
#if USESD
	strDIR, // DIR
#endif
	strREALLY, // REALLY
	strEND, // END
#if USE_TEXTATTRIBUTES
	strVT100_PROLOGUESEQ, // x1B[
	strVT100_CLS,
	strVT100_NOATTR,
	strVT100_BRIGHT,
	strVT100_UNDERSCORE,
	strVT100_REVERSE,
	strVT100_LINEHOME,
#if SET_PRINTZNES
	strVT100_CLEARZONES,
	strVT100_SETZONE,
#endif
#if USE_COLORATTRIBUTES
	strVT100_RED,
	strVT100_GREEN,
	strVT100_YELLOW,
	strVT100_BLUE,
	strVT100_MAGENTA,
	strVT100_CYAN,
	strVT100_WHITE
#endif // USE_COLORATTRIBUTES
#endif // USE_TEXTATTRIBUTES
};

uint8_t*
scanTable(const uint8_t *token, const uint8_t table[], uint8_t &index)
{
	uint8_t tokPos = 0, tabPos = 0;
	while (true) {
		uint8_t c = pgm_read_byte(table);
		uint8_t ct = token[tokPos];
		if (c == 0)
			return nullptr;
		
		if (ct == c) {
			++tokPos, ++table;
			continue;
		} else if (ct+uint8_t(0x80) == c) {
			index = tabPos;
			++tokPos;
			return (uint8_t*)token+tokPos;
		} else {
			if (c & uint8_t(0x80))
				c &= ~uint8_t(0x80);
			if (c > ct && ct != 0)
				return nullptr;
			else {
				while ((pgm_read_byte(table++) & uint8_t(0x80)) ==
				    0);
				++tabPos, tokPos=0;
			}
			continue;
		}
		
		if (ct == 0) {
			while ((pgm_read_byte(table++) & uint8_t(0x80)) ==
				    0);
			++tabPos, tokPos=0;
		}
	}
	return nullptr;
}

PGM_P
progmemString(ProgMemStrings index)
{
	return (PGM_P)pgm_read_ptr(&progmemStrings[uint8_t(index)]);
}

} // namespace BASIC
