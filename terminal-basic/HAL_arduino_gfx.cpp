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

#include "HAL_arduino_gfx.hpp"

#if HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341

ESPI::ESPI(uint8_t cs, uint8_t dc, uint8_t rs) :
tft(cs, dc, rs)
{
		m_row = m_column = m_scroll = 0;
		m_rows = 40, m_columns = 40;
		m_attr = VT100::TextAttr::NO_ATTR;
		m_lockCursor = false;
		m_cursorCounter = CURSOR_BLINK_PERIOD;
}

void
ESPI::begin()
{
    tft.begin(40000000ul);
    tft.setTextColor(ILI9341_LIGHTGREY);
    tft.setTextSize(1);
}

void
ESPI::clear()
{
    tft.fillScreen(ILI9341_BLACK);
    m_scroll = 0;
    tft.scrollTo(m_scroll);
    setCursor(0, 0);
    drawCursor(true);
}

void
ESPI::onTimer()
{
    // If cursor is locked by the non-interrupt code - return
    if (m_lockCursor)
      return;

    // Count down cursor blank interrupt counter
    if (--m_cursorCounter == 0)
      m_cursorCounter = CURSOR_BLINK_PERIOD;
    else
      return;

    m_cursorState = !m_cursorState;
    drawCursor(m_cursorState);
}

void
ESPI::drawCursor(bool v)
{
    tft.fillRect(tft.getCursorX(), tft.getCursorY(), 6, 8,
        v ? ILI9341_WHITE : ILI9341_BLACK);
}

void
ESPI::scrollLine()
  {
    if (m_row >= m_rows-1) {
      m_scroll += 8;
      if (m_scroll >= tft.height())
        m_scroll -= tft.height();
      tft.scrollTo(m_scroll);
      tft.fillRect(0, (tft.height() - 8 + m_scroll) %
          tft.height(), tft.width(), 8, ILI9341_BLACK);
    } else
      ++m_row;
    setCursor(m_column, m_row);
}

void
ESPI::writeChar(uint8_t c)
{
    m_lockCursor = true;
    drawCursor(false);
    switch (c) {
    case '\n':
      if (m_row < m_rows-1)
        tft.write('\n');
      scrollLine();
      break;
    case '\r':
      m_column = 0;
      break;
    case 8: //backspace
      if (m_column > 0)
        setCursor(--m_column, m_row);
      else if (m_row > 0) {
        m_column = m_columns-1;
        setCursor(m_column, --m_row);
      }
      break;
    default:
      tft.write(c);
      if (m_column >= m_columns-1) {
        m_column = 0;
        scrollLine();
      } else
        ++m_column;
    }
    drawCursor(true);
    m_lockCursor = false;
}

#define ILI9341_DARKYELLOW 0x7BE0
#define ILI9341_DARKMAGENTA 0x780F

const uint16_t ESPI::s_colors[uint8_t(VT100::Color::NUM_COLORS)][2] PROGMEM = {
    { ILI9341_BLACK, ILI9341_DARKGREY }, // COLOR_BLACK
  { ILI9341_RED, ILI9341_PINK }, // COLOR_RED
  { ILI9341_DARKGREEN, ILI9341_GREEN }, // COLOR_GREEN
  { ILI9341_DARKYELLOW, ILI9341_YELLOW },  // COLOR_YELLOW
  { ILI9341_NAVY, ILI9341_BLUE },  // COLOR_BLUE
  { ILI9341_DARKMAGENTA, ILI9341_MAGENTA },  // COLOR_MAGENTA
  { ILI9341_DARKCYAN, ILI9341_CYAN },  // COLOR_CYAN
  { ILI9341_LIGHTGREY, ILI9341_WHITE } // COLOR_WHITE
};

ESPI espi(TFT_CS, TFT_DC, TFT_RS);

#endif // HAL_ARDUINO_TERMINAL_OUTPUT

#if HAL_ARDUINO_GFX != HAL_ARDUINO_GFX_NONE

static HAL_gfx_color_t _colors[2] = {HAL_GFX_NOTACOLOR, HAL_GFX_NOTACOLOR};

#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL
#include "gfxterm.hpp"

static void
_write16(int16_t v)
{
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(v >> 8));
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(v & 0xFFu));
}

void
_writeCommand(GFXTERM::Command c)
{
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(ASCII::DLE));
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(c));
}

void
_setColors()
{
	_writeCommand(GFXTERM::Command::COLOR);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(_colors[0]));
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(_colors[1]));
}

#endif // HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341
static const uint16_t ilicolors[] PROGMEM = {
	ILI9341_BLACK, // 0
	ILI9341_WHITE, // 1
	ILI9341_RED, // 2
	ILI9341_GREEN, // 3
	ILI9341_BLUE, // 4
	ILI9341_CYAN, // 5
	ILI9341_MAGENTA, // 6
	ILI9341_YELLOW
};

static uint16_t ilicolor[2] = {ILI9341_WHITE, ILI9341_BLACK};

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI
static const uint16_t ilicolors[] PROGMEM = {
	TFT_BLACK, // 0
	TFT_WHITE, // 1
	TFT_RED, // 2
	TFT_GREEN, // 3
	TFT_BLUE, // 4
	TFT_CYAN, // 5
	TFT_MAGENTA, // 6
	TFT_YELLOW // 7
};

static uint16_t ilicolor[2] = {TFT_WHITE, TFT_BLACK};

#endif // HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341

void
HAL_gfx_setColor(HAL_gfx_color_t color)
{
	_colors[0] = color;

#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

	_setColors();

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341 || \
      HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI

	if (color != HAL_GFX_NOTACOLOR)
		ilicolor[0] = pgm_read_word(&ilicolors[uint8_t(color) - 1]);

#endif // HAL_ARDUINO_GFX
}

void
HAL_gfx_setBgColor(HAL_gfx_color_t color)
{
	_colors[1] = color;

#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

	_setColors();

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341 || \
      HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI

	if (color != HAL_GFX_NOTACOLOR)
		ilicolor[1] = pgm_read_word(&ilicolors[uint8_t(color) - 1]);

#endif // HAL_ARDUINO_GFX
}

static uint16_t linex = 0, liney = 0;

void
HAL_gfx_point(uint16_t x, uint16_t y)
{
#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

	_writeCommand(GFXTERM::Command::POINT);
	_write16(x); _write16(y);

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341 || \
      HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI

	if (_colors[0] != HAL_GFX_NOTACOLOR)
		espi.tft.drawPixel(x, y, ilicolor[0]);

	linex = x, liney = y;
#endif // HAL_ARDUINO_GFX
}

void
HAL_gfx_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

	_writeCommand(GFXTERM::Command::LINE);
	_write16(x1); _write16(y1);
	_write16(x2); _write16(y2);

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341 || \
      HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI

	if (_colors[0] != HAL_GFX_NOTACOLOR)
		espi.tft.drawLine(x1, y1, x2, y2, ilicolor[0]);

	linex = x2, liney = y2;

#endif // HAL_ARDUINO_GFX
}

void
HAL_gfx_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

	_writeCommand(GFXTERM::Command::BOX);
	_write16(x); _write16(y);
	_write16(w); _write16(h);

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341 || \
      HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI

	if (_colors[1] != HAL_GFX_NOTACOLOR)
		espi.tft.fillRect(x, y, w, h, ilicolor[1]);
	if (_colors[0] != HAL_GFX_NOTACOLOR)
		espi.tft.drawRect(x, y, w, h, ilicolor[0]);

#endif // HAL_ARDUINO_GFX
}

void
HAL_gfx_lineto(uint16_t x, uint16_t y)
{
#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

	_writeCommand(GFXTERM::Command::LINE);
	_write16(x); _write16(y);

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341 || \
    HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI

	if (_colors[0] != HAL_GFX_NOTACOLOR)
		espi.tft.drawLine(linex, liney, x, y, ilicolor[0]);
	linex = x, liney = y;

#endif // HAL_ARDUINO_GFX
}

void
HAL_gfx_circle(uint16_t x, uint16_t y, uint16_t r)
{
#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL

	_writeCommand(GFXTERM::Command::CIRCLEC);
	_write16(x); _write16(y);
	_write16(r);

#elif HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_ILI9341 || \
      HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_TFTeSPI

	if (_colors[1] != HAL_GFX_NOTACOLOR)
		espi.tft.fillCircle(x, y, r, ilicolor[1]);
	if (_colors[0] != HAL_GFX_NOTACOLOR)
		espi.tft.drawCircle(x, y, r, ilicolor[0]);

#endif // HAL_ARDUINO_GFX
}

#if !HAL_GFX_EXPCOLOR_SIMPL

void
HAL_gfx_pointc(uint16_t x, uint16_t y, HAL_gfx_color_t color)
{
	_writeCommand(GFXTERM::Command::POINTC);
	_write16(x); _write16(y);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(color));
}

void
HAL_gfx_linec(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, HAL_gfx_color_t color)
{
	_writeCommand(GFXTERM::Command::LINEC);
	_write16(x1); _write16(y1);
	_write16(x2); _write16(y2);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(color));
}

void
HAL_gfx_rectc(uint16_t x, uint16_t y, uint16_t w, uint16_t h, HAL_gfx_color_t color)
{
	_writeCommand(GFXTERM::Command::BOXC);
	_write16(x); _write16(y);
	_write16(w); _write16(h);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(color));
}

void
HAL_gfx_circlec(uint16_t x, uint16_t y, uint16_t r, HAL_gfx_color_t color)
{
	_writeCommand(GFXTERM::Command::CIRCLEC);
	_write16(x); _write16(y);
	_write16(r);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(color));
}
#endif // HAL_GFX_EXPCOLOR_SIMPL

#endif // HAL_GFX
