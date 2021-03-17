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

#include <string.h>
#include <ctype.h>

#include "basic.hpp"

#if USEUTFT

#include "utft_stream.hpp"
#include "ascii.hpp"
#include "Arduino.h"
#include "bytearray.hpp"
#include "arduino_logger.hpp"

#include "seriallight.hpp"

extern uint8_t SmallFont[];
extern uint8_t BigFont[];

UTFTTerminal::UTFTTerminal(UTFT &screen) :
_screen(screen)
{
}

void
UTFTTerminal::begin()
{
	_page = 1;
	_screen.InitLCD();
	_screen.setFont(BigFont);
	_screenSizePixels.setX(_screen.getDisplayXSize());
	_screenSizePixels.setY(_screen.getDisplayYSize());
	_screenSizeChars.setX(_screenSizePixels.x() /
		_screen.getFontXsize());
	_screenSizeChars.setY(_screenSizePixels.y() /
		_screen.getFontYsize());
	_cursorPosition.set(0,0);
	memset(_data, 0, sizeof (_data));
	
	_screen.setColor(VGA_SILVER);
	_state = REGULAR;
	_screen.clrScr();
	drawCursor();
}

void
UTFTTerminal::end()
{
}

void
UTFTTerminal::dump()
{	
	ByteArray ba((uint8_t*)_data, sizeof (_data));
	Serial.println(ba);
}

void
UTFTTerminal::setPower(bool val)
{
	if (val)
		_screen.lcdOn();
	else
		_screen.lcdOff();
}

size_t
UTFTTerminal::write(uint8_t c)
{
	switch (_state) {
	case REGULAR:
		LOG("REGULAR");
		switch (ASCII(c)) {
		case ASCII::ESC:
			_attr1 = 0;
			_state = ESCAPE;
			return 1;
		case ASCII::BS:
			drawCursor(false);
			insertChar(c);
			drawCursor();
			return 1;
		case ASCII::CR:
			drawCursor(false);
			insertChar(c);
			return 1;
		case ASCII::LF:
			insertChar(c);
			drawCursor();
			return 1;
		case ASCII::HT:
			for (uint8_t i=0; i<8; ++i)
				this->write(' ');
			return 1;
		default:
			insertChar(c);
			drawCursor();
			return 1;
		};
		break;
	case ESCAPE:
		LOG("ESCAPE");
		switch (c) {
		case '[':
			_state = I_BRAC; return 1;
		case char(ASCII::CAN):
		default:
			_state = REGULAR; return 1;
		}
		break;
	case I_BRAC:
		LOG("I_BRAC");
		switch (c) {
		case char(ASCII::CAN):
			_state = REGULAR; return 1;
		case 'm':
			_state = REGULAR;
			if (_attr1 == 1)
				addAttr(VT100::BRIGHT);
			else if (_attr1 == 31)
				addAttr(VT100::C_RED);
			else if (_attr1 == 32)
				addAttr(VT100::C_GREEN);
			else if (_attr1 == 33)
				addAttr(VT100::C_YELLOW);
			else if (_attr1 == 34)
				addAttr(VT100::C_BLUE);
			else if (_attr1 == 35)
				addAttr(VT100::C_MAGENTA);
			else if (_attr1 == 36)
				addAttr(VT100::C_CYAN);
			else if (_attr1 == 37)
				addAttr(VT100::BRIGHT);
			else
				addAttr(VT100::NO_ATTR);
			return 1;
		case 'J':
			if (_attr1 == 2) {
				_state = REGULAR;
				_screen.clrScr();
				_cursorPosition.set(0,0);
				memset(_data, 0, sizeof (_data));
			}
			return 1;
		default:
			if (isdigit(c)) {
				_attr1*=10, _attr1+=c-'0';
			} else
				_state = REGULAR;
		};
		break;
	default:
		LOG("Unknown state");
	}
	
	return 1;
}

void
UTFTTerminal::redraw()
{
	LOG_TRACE;
	
	_page = 3-_page;
	
	_cursorPosition.set(0,0);
	VT100::TextAttr a = _attributes;
	_attributes = VT100::NO_ATTR;
	_screen.setColor(VGA_SILVER);
	for (uint16_t i=0; i<H; ++i) {
		uint16_t j;
		for (j=0; j<W; ++j) {
			_cursorPosition.set(j,i);
			char c = _data[i][j]._symbol;
			LOG(Logger::format_t::hex, c, Logger::format_t::dec);
			if (c == '\0') {
				break;
			}
			printChar(i, j);
			if (c == '\n') {
				break;
			} else
				_attributes = _data[i][j]._attrs;
		}
		word cl = _screen.getColor();
		_screen.setColor(VGA_BLACK);
		_screen.fillRect(j*_screen.getFontXsize(), i*_screen.getFontYsize(),
		    _screen.getDisplayXSize()-1, (i+1)*_screen.getFontYsize());
		_screen.setColor(cl);
	}
	_screen.setColor(VGA_BLACK);
	_screen.fillRect(0, _screen.getDisplayYSize()-_screen.getFontYsize()-1,
	    _screen.getDisplayXSize()-1, _screen.getDisplayYSize()-1);
	addAttr(a);
}

void
UTFTTerminal::printChar(uint8_t y, uint8_t x)
{
	LOG_TRACE;	
	
	VT100::TextAttr a = _data[y][x]._attrs;
	if (_attributes != a)
		addAttr(VT100::NO_ATTR), addAttr(a);
	_screen.printChar(_data[y][x]._symbol,
	    x*_screen.getFontXsize(), y*_screen.getFontYsize());
}

void
UTFTTerminal::insertChar(uint8_t c)
{
	LOG_TRACE;
	LOG(Logger::format_t::hex, c, Logger::format_t::dec);
	LOG(_insertPosition.x(), _insertPosition.y());

	switch (c) {
	case char(ASCII::BS):
		if (_cursorPosition.x > 0)
			_cursorPosition.x--;
		else if (_cursorPosition.y > 0)
			_cursorPosition.y--, _cursorPosition.x = W-1;
		return;
	case '\r' : _cursorPosition.x = 0; break;
	case '\n' : _cursorPosition.y++; break;
	default:
		_data[_cursorPosition.y][_cursorPosition.x]._attrs = _attributes;
		_data[_cursorPosition.y][_cursorPosition.x]._symbol = c;
		printChar(_cursorPosition.y, _cursorPosition.x);
		_cursorPosition.next();
		break;
	};

	if (_cursorPosition.x >= W) {
		LOG("New line");
		_cursorPosition.x = 0;
		_cursorPosition.y++;
	}
	if (_cursorPosition.y >= H) {
		LOG("Scroll");
		_cursorPosition.y--;
		uint32_t start = millis();
		memmove(_data[0], _data[1], sizeof (Cell)*W*(H-1));
		memset(_data[H-1], 0, sizeof (Cell)*W);
		redraw();
		uint32_t stop = millis();
		SerialL.print("Redraw "); SerialL.println(stop-start, DEC);
	}
}

void
UTFTTerminal::putChar(uint8_t c)
{	
	uint16_t x = _cursorPosition.x*_screen.getFontXsize(),
		y = _cursorPosition.y*_screen.getFontYsize();
	
	LOG("pos:", x, y);
	_screen.printChar(c, x, y);
	_cursorPosition.next();
	if (_cursorPosition.x >= W) {
		_cursorPosition.x = 0;
		_cursorPosition.y++;
	}
	if (_cursorPosition.y >= H) {
		_cursorPosition.y--;
	}
}

void
UTFTTerminal::addAttr(VT100::TextAttr attrs)
{
	if (attrs == VT100::NO_ATTR) {
		_attributes = VT100::NO_ATTR;
		_screen.setColor(VGA_SILVER);
		return;
	}
	
	_attributes |= attrs;
	if ((_attributes & 0xF0) == VT100::C_GREEN)
		_screen.setColor(20, 255, 20);
	else if ((_attributes & 0xF0) == VT100::C_RED)
		_screen.setColor(VGA_RED);
	else if ((_attributes & 0xF0) == VT100::C_BLUE)
		_screen.setColor(VGA_BLUE);
	else if ((_attributes & 0xF0) == VT100::C_YELLOW)
		_screen.setColor(VGA_YELLOW);
	else if ((_attributes & 0xF0) == VT100::C_MAGENTA)
		_screen.setColor(255, 0, 255);
	else if ((_attributes & 0xF0) == VT100::C_CYAN) {
		_screen.setColor(0, 255,255);
	} else if (_attributes & VT100::BRIGHT)
		_screen.setColor(VGA_WHITE);
}

void
UTFTTerminal::drawCursor(bool v)
{
	word c = _screen.getColor(), b = _screen.getBackColor();
	if (!v) 
		_screen.setColor(b);
	_screen.fillRect(_cursorPosition.x*_screen.getFontXsize(),
			 _cursorPosition.y*_screen.getFontYsize(),
			 (_cursorPosition.x+1)*_screen.getFontXsize()-1,
			 (_cursorPosition.y+1)*_screen.getFontYsize()-1);
	if (!v)
		_screen.setColor(c);
}

#endif // USE_UTFT
