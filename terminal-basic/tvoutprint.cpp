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

#include "tvoutprint.hpp"

#if USETVOUT

#include "ascii.hpp"

#if TVOUT_DEBUG
#include "seriallight.hpp"
#endif

TVoutPrint::TVoutPrint()
{
}

void
TVoutPrint::writeChar(uint8_t c)
{
	TVoutEx::instance()->write(c);
}

uint8_t
TVoutPrint::getCursorX()
{
	return TVoutEx::instance()->getCursorX();
}

void
TVoutPrint::setCursorX(uint8_t x)
{
	TVoutEx::instance()->setCursorX(x);
}

void
TVoutPrint::setCursor(uint8_t x, uint8_t y)
{
	TVoutEx::instance()->setCursorCharPosition(x, y);
}

void
TVoutPrint::clear()
{
	TVoutEx::instance()->clearScreen();
}

#endif // USETVOUT
