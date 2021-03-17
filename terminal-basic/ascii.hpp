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

/**
 * @brief ASCII control characters
 */
enum class ASCII : uint8_t
{
	NUL = 0x00,
	SOH = 0x01,
        EOT = 0x03,
	ENQ = 0x05,
	BEL = 0x07,
        BS  = 0x08,
	HT  = 0x09,
	LF  = 0x0A,
	CR  = 0x0D,
	CAN = 0x18,
	ESC = 0x1B
};

#endif
