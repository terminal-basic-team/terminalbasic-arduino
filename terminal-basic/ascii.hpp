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

#ifndef ASCII_HPP
#define ASCII_HPP

#include <inttypes.h>

/**
 * @brief ASCII characters
 */
enum class ASCII : uint8_t
{
	NUL = 0x00,
	SOH = 0x01,
	STX = 0x02,
	ETX = 0x03,
	EOT = 0x04,
	ENQ = 0x05,
	ACK = 0x06,
	BEL = 0x07,
	BS = 0x08,
	HT = 0x09,
	LF = 0x0A,
	VT = 0x0B,
	FF = 0x0C,
	CR = 0x0D,
	SO = 0x0E,
	SI = 0x0F,
	NAK = 0x15,
	SYN = 0x16,
	CAN = 0x18,
	ESC = 0x1B,
	SPACE = ' ',
	QMARK = '?',
	DOT = '.',
	COLON = ':',
	QUMARK = '\"',
	LPAREN = '(',
	RPAREN = ')',
	COMMA = ',',
	TILDE = 0x7E,
	DEL = 0x7F
};

#endif
