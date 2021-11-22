/*
 * This file is part of Terminal-BASIC: a lightweight BASIC-like language
 * interpreter.
 * 
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
 * Copyright (C) 2019-2021 Terminal-BASIC team
 *     <https://github.com/terminal-basic-team>
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

#ifndef TOOLS_H
#define TOOLS_H

#include "sys/cdefs.h"
#include <stdint.h>

__BEGIN_DECLS

typedef uint8_t BOOLEAN;

#ifndef TRUE
#define TRUE ((BOOLEAN)1)
#endif

#ifndef FALSE
#define FALSE ((BOOLEAN)0)
#endif

/**
 * @brief Test if character is alphabetic
 * @param 
 */
BOOLEAN tools_isAlpha(uint8_t);

BOOLEAN tools_isAlphaNum(uint8_t);

void _ftoa(float, char*);

void _dtoa(double, char*);

void readU16(uint16_t*, const uint8_t*);

void writeU16(uint16_t, uint8_t*);

void readU32(uint32_t*, const uint8_t*);

void writeU32(uint32_t, uint8_t*);

void readU64(uint64_t*, const uint8_t*);

void writeU64(uint64_t, uint8_t*);

void readR32(float*, const uint8_t*);

void writeR32(float, uint8_t*);

void writeR64(double, uint8_t*);

void readR64(double*, const uint8_t*);

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define LOW_BYTE_U16(w) ((uint8_t)w)

#define HIGH_BYTE_U16(w) ((uint8_t)((w) >> 8))

__END_DECLS

#endif /* TOOLS_H */

