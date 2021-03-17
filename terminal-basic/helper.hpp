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

#ifndef HELPER_HPP
#define HELPER_HPP

#include "cps.hpp"
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>

inline void
positiveLedBlink(size_t num = 1)
{
	for (size_t i = 0; i<num; ++i) {
		digitalWrite(13, HIGH);
		delay(100);
		digitalWrite(13, LOW);
		delay(100);
	}
}

inline void
negativeLedBlink(size_t num = 1)
{
	for (size_t i = 0; i < num; ++i) {
		digitalWrite(13, HIGH);
		delay(500);
		digitalWrite(13, LOW);
		delay(300);
	}
}

#else

#ifndef ARDUINO
#define positiveLedBlink(a...)
#define negativeLedBlink(a...)
#define PGM_P const char*
#define PROGMEM
#define pgm_read_byte(a) (*a)
#define pgm_read_word(a) (*a)
#define pgm_read_ptr(a) (*a)
#define strcpy_P(a,b) strcpy(a,b)
#define strcmp_P(a,b) strcmp(a,b)
#endif

#endif

inline bool
endsWith(const char *str, const char end)
{
	if (str == NULL || strlen(str)==0)
		return false;
	if (end == 0)
		return true;
	if (str[strlen(str)-1] == end)
		return true;
	else
		return false;
}

inline bool
endsWith(const char *str, const char *end)
{
	size_t l1, l2;
	
	l1 = strlen(str); l2 = strlen(end);
	
	if (str == NULL || l1 == 0 || l2 == 0 || l1 < l2)
		return false;
	else if (strcmp(str+l1-l2, end) == 0)
		return true;
	else
		return false;
}

#endif /* HELPER_HPP */
