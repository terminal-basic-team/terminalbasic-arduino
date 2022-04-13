/*
 * ArduinoExt is a set of utility libraries for Arduino
 * 
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
 * Copyright (C) 2019,2021 Terminal-BASIC team
 *     <https://github.com/terminal-basic-team>
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

#ifndef BUFFERED_TERMINAL_HPP
#define BUFFERED_TERMINAL_HPP

#include "vt100.hpp"

class BufferedTerminal : public VT100::Print
{
	EXT_NOTCOPYABLE(BufferedTerminal)

public:
	/**
	 * @param rows
	 * @param columns
	 */
	void begin(
	    uint16_t,
	    uint16_t);
	
	void update();

protected:

  BufferedTerminal() = default;
	
	virtual void drawCursor(bool) = 0;
	
	// Current row and column
	uint16_t m_row, m_column;
	// Number of rows and columns
	uint16_t m_rows, m_columns;
	// Cursor lock object and blinking cursor state
	bool m_lockCursor, m_cursorState;
	// Cursor counter and period
	uint8_t m_cursorCounter, m_cursorBlinkPeriod = 20;
	// Current attribute
	VT100::TextAttr m_attr;
	
// VT100::Print interface
protected:
	uint8_t getCursorX() override;

	void setCursorX(uint8_t) override;
};

#endif // BUFFERED_TERMINAL_HPP
