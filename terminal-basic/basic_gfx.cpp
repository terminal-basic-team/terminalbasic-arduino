/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2017-2018 Andrey V. Skvortsov <starling13@mail.ru>
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
	'B','O','X'+0x80,
	'C','I','R','C','L','E'+0x80,
	'C','O','L','O','R'+0x80,
	'L','I','N','E','T','O'+0x80,
	'L','I','N','E'+0x80,
	'P','O','I','N','T'+0x80,
	'S','C','R','E','E','N'+0x80,
	0
};

const FunctionBlock::function GFXModule::comms[] PROGMEM = {
	GFXModule::command_box,
	GFXModule::command_circle,
	GFXModule::command_color,
	GFXModule::command_lineto,
	GFXModule::command_line,
	GFXModule::command_point,
	GFXModule::command_screen
};

GFXModule::GFXModule()
{
	commands = comms;
	commandTokens = gfxTokens;
}

} // namespace BASIC

#endif // USE_GFX
