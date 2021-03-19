/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2017-2019 Andrey V. Skvortsov <starling13@mail.ru>
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

#include "basic_gfx.hpp"

#if USE_GFX

namespace BASIC
{

static const uint8_t gfxTokens[] PROGMEM = {
	'B','O','X','C', ASCII_NUL,
	'B','O','X', ASCII_NUL,
	'C','I','R','C','L','E','C', ASCII_NUL,
	'C','I','R','C','L','E', ASCII_NUL,
	'C','O','L','O','R', ASCII_NUL,
	'C','U','R','S','O','R', ASCII_NUL,
	'L','I','N','E','T','O', ASCII_NUL,
	'L','I','N','E','C', ASCII_NUL,
	'L','I','N','E', ASCII_NUL,
	'P','O','I','N','T','C', ASCII_NUL,
	'P','O','I','N','T', ASCII_NUL,
	'S','C','R','E','E','N', ASCII_NUL,
	ASCII_ETX
};

const FunctionBlock::function GFXModule::comms[] PROGMEM = {
	GFXModule::command_boxc,
	GFXModule::command_box,
	GFXModule::command_circlec,
	GFXModule::command_circle,
	GFXModule::command_color,
	GFXModule::command_cursor,
	GFXModule::command_lineto,
	GFXModule::command_linec,
	GFXModule::command_line,
	GFXModule::command_pointc,
	GFXModule::command_point,
	GFXModule::command_screen,
};

GFXModule::GFXModule()
{
	commands = comms;
	commandTokens = gfxTokens;
}

} // namespace BASIC

#endif // USE_GFX
