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

#ifndef UTFT_STREAM_HPP
#define UTFT_STREAM_HPP

#include <memorysaver.h>
#include <UTFT.h>
#include "math.hpp"
#include "arduinoext.hpp"
#include "vt100.hpp"

#define W 30
#define H 20

extern uint8_t SmallFont[];
extern uint8_t BigFont[];

class UTFTTerminal : public Print
{
public:
	UTFTTerminal(UTFT&);
	void begin();
	void end();
	void dump();
	void setPower(bool);
private:
	enum State_t : uint8_t
	{
		REGULAR = 0, ESCAPE, I_BRAC, F_ATTR1
	};
	
	struct EXT_PACKED ScreenCoordinates
	{
		void next()
		{
			++x;
		}
		void set(uint8_t xx, uint8_t yy)
		{
			x=xx, y=yy;
		}
		uint8_t x,y;
	};
	
	void redraw();
	void insertChar(uint8_t);
	void printChar(uint8_t, uint8_t);
	void putChar(uint8_t);
	void addAttr(VT100::TextAttr);
	void drawCursor(bool=true);
	void drawCurrent();
	
	UTFT &_screen;
	State_t	_state;
	CartesianCoordinates2D<uint16_t> _screenSizePixels;
	CartesianCoordinates2D<uint8_t> _screenSizeChars;
	ScreenCoordinates _cursorPosition;
	struct EXT_PACKED Cell
	{
		VT100::TextAttr _attrs;
		char	     _symbol;
	};
	Cell		_data[H][W];
	char		_attr1;
	VT100::TextAttr	_attributes;
	uint8_t		_page;
// Print interface
public:
	size_t write(uint8_t) override;
};

#endif
