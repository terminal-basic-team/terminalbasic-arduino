/*
 * ArduinoExt is a set of utility libraries for Arduino
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
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

#ifndef GFXTERM_HPP
#define GFXTERM_HPP

#include "ascii.hpp"
#include "helper.hpp"

Package(GFXTERM)
{
	EXT_PACKAGE(GFXTERM)
	    
public:
	
	enum class Command : uint8_t
	{
		// Sert terminal video mode
		// param m - uint8_t mode number
		MODE = 0x20,
		// Set text cursor visible
		// param bool (uint8_t) visible
		CURSOR = 0x21
	};
};

#endif /* GFXTERM_HPP */
