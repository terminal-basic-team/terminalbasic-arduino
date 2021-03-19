/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016-2019 Andrey V. Skvortsov <starling13@mail.ru>
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

static void
write16(Interpreter &i, int16_t v)
{
    i.print(char(v>>8)); i.print(char(v&0xFF));
}

bool
GFXModule::command_box(Interpreter &i)
{
	INT x,y,w,h;
	
	if (getIntegerFromStack(i, h)) {
		if (getIntegerFromStack(i, w)) {
			if (getIntegerFromStack(i, y)) {
				if (getIntegerFromStack(i, x)) {
					i.print(char(ASCII::DLE));
					i.print(char(GFXTERM::Command::BOX));
					write16(i, x);
					write16(i, y);
					write16(i, w);
					write16(i, h);
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
				i.print(char(ASCII::DLE));
				i.print(char(GFXTERM::Command::CIRCLE));
				write16(i, x);
				write16(i, y);
				write16(i, r);
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
GFXModule::command_cursor(Interpreter &i)
{
	Parser::Value v(false);
	if (i.popValue(v)) {
		if (v.type() == Parser::Value::LOGICAL) {
			i.print(char(ASCII::DLE));
			i.print(char(GFXTERM::Command::CURSOR));
			i.print(bool(v) ? char(1) : char(0));
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
					i.print(char(ASCII::DLE));
					i.print(char(GFXTERM::Command::LINE));
					write16(i, x1);
					write16(i, y1);
					write16(i, x2);
					write16(i, y2);
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
	INT x,y;
	
	if (getIntegerFromStack(i, y)) {
		if (getIntegerFromStack(i, x)) {
			i.print(char(ASCII::DLE));
			i.print(char(GFXTERM::Command::LINETO));
			write16(i, x);
			write16(i, y);
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
			i.print(char(ASCII::DLE));
			i.print(char(GFXTERM::Command::POINT));
			write16(i, x);
			write16(i, y);
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

bool
GFXModule::command_pointc(Interpreter &i)
{
	INT x,y,z;
	if (getIntegerFromStack(i, z)) {
		if (getIntegerFromStack(i, y)) {
			if (getIntegerFromStack(i, x)) {
				i.print(char(ASCII::DLE));
				i.print(char(GFXTERM::Command::POINTC));
				write16(i, x);
				write16(i, y);
				i.print(char(z));
				return true;
			}
		}
	}
	return false;
}

bool
GFXModule::command_circlec(Interpreter &i)
{
	INT x,y,r,z;
	
	if (getIntegerFromStack(i, z)) {
		if (getIntegerFromStack(i, r)) {
			if (getIntegerFromStack(i, y)) {
				if (getIntegerFromStack(i, x)) {
					i.print(char(ASCII::DLE));
					i.print(char(GFXTERM::Command::CIRCLEC));
					write16(i, x);
					write16(i, y);
					write16(i, r);
					i.print(char(z));
					return true;
				}
			}
		}
	}
	return false;
}

bool
GFXModule::command_boxc(Interpreter &i)
{
	INT x,y,w,h,z;
	if (getIntegerFromStack(i, z)) {
		if (getIntegerFromStack(i, h)) {
			if (getIntegerFromStack(i, w)) {
				if (getIntegerFromStack(i, y)) {
					if (getIntegerFromStack(i, x)) {
						i.print(char(ASCII::DLE));
						i.print(char(GFXTERM::Command::BOXC));
						write16(i, x);
						write16(i, y);
						write16(i, w);
						write16(i, h);
						i.print(char(z));
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool
GFXModule::command_linec(Interpreter &i)
{
	INT x1,y1,x2,y2,z;
	
	if (getIntegerFromStack(i, z)) {
		if (getIntegerFromStack(i, y2)) {
			if (getIntegerFromStack(i, x2)) {
				if (getIntegerFromStack(i, y1)) {
					if (getIntegerFromStack(i, x1)) {
						i.print(char(ASCII::DLE));
						i.print(char(GFXTERM::Command::LINEC));
						write16(i, x1);
						write16(i, y1);
						write16(i, x2);
						write16(i, y2);
						i.print(char(z));
						return true;
					}
				}
			}
		}
	}
	return false;
}
} // namespace BASIC

#endif // USE_GFX && SERIAL_GFX
