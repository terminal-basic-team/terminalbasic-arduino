/*
 * ArduinoExt is a set of utility libraries for Arduino
 * Copyright (C) 2016, 2017 Andrey V. Skvortsov <starling13@mail.ru>
 *
 * This program is free software: is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.

 * ArduinoExt library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with Posixcpp library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef VT100_HPP
#define VT100_HPP

#include "cps.hpp"

class VT100
{
	CPS_PACKAGE(VT100);
public:
	// Terminal text attributes to use when printing
	enum TextAttr : uint8_t
	{
		NO_ATTR = 0x0,
		BRIGHT = 0x1,
		UNDERSCORE = 0x2,
		BLINK = 0x4,
		REVERSE = 0x8,
		C_WHITE = 0x00,
		C_BLACK = 0x10,
		C_RED = 0x20,
		C_GREEN = 0x30,
		C_YELLOW = 0x40,
		C_BLUE = 0x50,
		C_MAGENTA = 0x60,
		C_CYAN = 0x70,
		CB_BLACK = 0x80,
		CB_RED = 0x90,
		CB_GREEN = 0xA0,
		CB_YELLOW = 0xB0,
		CB_BLUE = 0xC0,
		CB_MAGENTA = 0xD0,
		CB_CYAN = 0xE0,
		CB_WHITE = 0xF0,
	};
};

#endif /* VT100_HPP */

