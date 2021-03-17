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

#include "basic_gfx.hpp"

#if USE_GFX && SERIAL_GFX

#include "gfxterm.hpp"

namespace BASIC
{

void GFXModule::_init() {}

bool
GFXModule::command_box(Interpreter &i)
{
	INT x,y,w,h;
	
	if (getIntegerFromStack(i, h)) {
		if (getIntegerFromStack(i, w)) {
			if (getIntegerFromStack(i, y)) {
				if (getIntegerFromStack(i, x)) {
					
					return true;
				}
			}
		}
	}
	return false;
}

bool
GFXModule::command_circle(Interpreter &i)
{
	INT x,y,r;
	
	if (getIntegerFromStack(i, r)) {
		if (getIntegerFromStack(i, y)) {
			if (getIntegerFromStack(i, x)) {
				
				return true;
			}
		}
	}
	return false;
}

bool
GFXModule::command_color(Interpreter &i)
{
	INT c, b;
	
	if (getIntegerFromStack(i, b)) {
		if (getIntegerFromStack(i, c)) {
			i.print(char(ASCII::DLE));
			i.print(char(GFXTERM::Command::COLOR));
			i.print(char(c));
			i.print(char(b));
			
			return true;
		}
	}
	return false;
}

bool
GFXModule::command_line(Interpreter &i)
{
	INT x1,y1,x2,y2;
	
	if (getIntegerFromStack(i, y2)) {
		if (getIntegerFromStack(i, x2)) {
			if (getIntegerFromStack(i, y1)) {
				if (getIntegerFromStack(i, x1)) {
					
					return true;
				}
			}
		}
	}
	return false;
}

bool
GFXModule::command_lineto(Interpreter &i)
{
	INT x1,y1;
	
	if (getIntegerFromStack(i, y1)) {
		if (getIntegerFromStack(i, x1)) {
			
			return true;
		}
	}
	
	return false;
}

bool
GFXModule::command_point(Interpreter &i)
{
	INT x,y;
	
	if (getIntegerFromStack(i, y)) {
		if (getIntegerFromStack(i, x)) {
			
			return true;
		}
	}
	return false;
}

bool
GFXModule::command_screen(Interpreter &i)
{
	INT x;
	
	if (getIntegerFromStack(i, x)) {
		i.print(char(ASCII::DLE));
		i.print(char(GFXTERM::Command::MODE));
		i.print(char(x));
		return true;
	}
	return false;
}

} // namespace BASIC

#endif // USE_GFX && SERIAL_GFX
