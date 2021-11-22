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

#include "buffered_terminal.hpp"

void
BufferedTerminal::begin(uint16_t rows, uint16_t columns)
{
	m_row = m_column = 0;
	m_rows = rows, m_columns = columns;
	m_attr = VT100::TextAttr::NO_ATTR;
	m_lockCursor = false;
	m_cursorState = true;
}

void
BufferedTerminal::update()
{
	// If cursor is locked by the non-interrupt code - return
	if (m_lockCursor)
		return;

	// Count down cursor blank interrupt counter
	if (--m_cursorCounter == 0) {
		m_cursorCounter = m_cursorBlinkPeriod;
	} else
		return;

	m_cursorState = !m_cursorState;
	drawCursor(m_cursorState);
}

uint8_t
BufferedTerminal::getCursorX()
{
	return m_column;
}

void
BufferedTerminal::setCursorX(uint8_t x)
{
	setCursor(x, m_row);
}
