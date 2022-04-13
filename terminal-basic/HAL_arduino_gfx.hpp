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

#ifndef HAL_ARDUINO_GFX_HPP
#define HAL_ARDUINO_GFX_HPP

#include "HAL_arduino.h"
#include "vt100.hpp"

#if HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341
#include "Adafruit_ILI9341.h"

class ESPI : public VT100::Print
{
public:

#define CURSOR_BLINK_PERIOD 20

	ESPI(uint8_t cs, uint8_t dc, uint8_t rs);

	void begin();

	void clear() override;

	void onTimer();

	Adafruit_ILI9341 tft;

protected:

	void drawCursor(bool v);

	void scrollLine();

	void writeChar(uint8_t c) override;

	uint8_t getCursorX() override
	{
		return m_column;
	}

	void setCursorX(uint8_t x) override
	{
		setCursor(x, m_row);
	}

	void setCursor(uint8_t x, uint8_t y) override
	{
		drawCursor(false);
		m_row = y % m_rows;
		m_column = x % m_columns;
		tft.setCursor(m_column * 6, (y * 8 + m_scroll) % tft.height());
		drawCursor(true);
	}

	void addAttribute(VT100::TextAttr ta) override
	{
		switch (ta) {
		case VT100::TextAttr::BRIGHT:
			m_attr |= VT100::TextAttr::BRIGHT;
			break;
		case VT100::TextAttr::C_GREEN:
		case VT100::TextAttr::C_YELLOW:
		case VT100::TextAttr::C_BLUE:
		case VT100::TextAttr::C_CYAN:
		case VT100::TextAttr::C_MAGENTA:
		case VT100::TextAttr::C_RED:
		case VT100::TextAttr::C_WHITE:
		case VT100::TextAttr::C_BLACK:
			m_attr &= VT100::TextAttr(0x0F);
			m_attr |= ta;
			break;
		default:
			break;
		}

		VT100::Color color = VT100::Color::COLOR_BLACK;
		switch (m_attr & 0xF0) {
		case VT100::TextAttr::C_GREEN:
			color = VT100::Color::COLOR_GREEN;
			break;
		case VT100::TextAttr::C_YELLOW:
			color = VT100::Color::COLOR_YELLOW;
			break;
		case VT100::TextAttr::C_BLUE:
			color = VT100::Color::COLOR_BLUE;
			break;
		case VT100::TextAttr::C_CYAN:
			color = VT100::Color::COLOR_CYAN;
			break;
		case VT100::TextAttr::C_MAGENTA:
			color = VT100::Color::COLOR_MAGENTA;
			break;
		case VT100::TextAttr::C_RED:
			color = VT100::Color::COLOR_RED;
			break;
		case VT100::TextAttr::C_WHITE:
			color = VT100::Color::COLOR_WHITE;
			break;
		}

		if (m_attr & VT100::TextAttr::BRIGHT)
			tft.setTextColor(pgm_read_word(&s_colors[color][1]));
		else
			tft.setTextColor(pgm_read_word(&s_colors[color][0]));
	}

	void resetAttributes() override
	{
		tft.setTextColor(ILI9341_LIGHTGREY);
		m_attr = VT100::TextAttr::NO_ATTR;
	}

private:

	uint8_t m_row, m_column;

	uint8_t m_rows, m_columns;

	uint16_t m_scroll;

	VT100::TextAttr m_attr;

	static const uint16_t s_colors[uint8_t(VT100::Color::NUM_COLORS)][2] PROGMEM;

	bool m_lockCursor, m_cursorState;

	uint8_t m_cursorCounter;
};

extern ESPI espi;

#endif // HAL_ARDUINO_TERMINAL_OUTPUT

#endif // HAL_ARDUINO_GFX_HPP
