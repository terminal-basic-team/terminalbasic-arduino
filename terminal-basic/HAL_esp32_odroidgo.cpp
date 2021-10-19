/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * 
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
 * Copyright (C) 2019,2021 Terminal-BASIC team
 *     <https://bitbucket.org/%7Bf50d6fee-8627-4ce4-848d-829168eedae5%7D/>
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

#include "HAL_esp32.h"

#if HAL_ESP32_ODROIDGO

#include "odroid_go.h"
#include "buffered_terminal.hpp"

#if HAL_ESP32_ODROIDGO_QWERTY
#include "utility/KeyboardStream.h"

static ODROIDGO::OKeyboard* kbd;
static ODROIDGO::OKeyboardStream* kbdStream;
#endif // HAL_ESP32_ODROIDGO_QWERTY

class OdroidGoScreen : public BufferedTerminal
{
public:

  OdroidGoScreen()
  {
  }

  void
  clear() override
  {
    memset(m_screenBuffer[0], ' ', m_rows * m_columns);
    memset(m_screenBuffer[1], char(VT100::TextAttr::NO_ATTR), m_rows * m_columns);
    GO.lcd.fillScreen(TFT_BLACK);
    setCursor(0, 0);
    //drawCursor(true);
  }

  void
  begin(uint8_t mode)
  {
    GO.lcd.setTextColor(TFT_LIGHTGREY);
    GO.lcd.setTextSize(1);

    if (mode == 0) {
      GO.lcd.setTextFont(3);
      m_charWidth = 6, m_charHeight = 8;
    } else if (mode == 1) {
      GO.lcd.setTextFont(4);
      m_charWidth = 8, m_charHeight = 10;
    } else {
      GO.lcd.setTextFont(5);
      m_charWidth = 8, m_charHeight = 16;
    }
      
    GO.lcd.clearDisplay();

    if (mode == 0)
      BufferedTerminal::begin(30, 53);
    else if (mode == 1)
      BufferedTerminal::begin(24, 40);
    else
      BufferedTerminal::begin(15, 40);
    clear();
  }

protected:

  void drawCursor(bool v) override
  {
    GO.lcd.fillRect(GO.lcd.getCursorX(), GO.lcd.getCursorY(), m_charWidth, m_charHeight,
        v ? TFT_WHITE : TFT_BLACK);
  }

  void scrollLine()
  {
    m_lockCursor = true;
    if (m_row >= m_rows - 1) {

      memmove(m_screenBuffer[0], m_screenBuffer[0] + m_columns, uint16_t(m_columns)*(m_rows - 1));
      memmove(m_screenBuffer[1], m_screenBuffer[1] + m_columns, uint16_t(m_columns)*(m_rows - 1));

      memset(m_screenBuffer[0] + uint16_t(m_columns)*(m_rows - 1), ' ', m_columns);
      memset(m_screenBuffer[1] + uint16_t(m_columns)*(m_rows - 1), VT100::TextAttr::NO_ATTR, m_columns);

      GO.lcd.setCursor(0, 0);
      //GO.lcd.fillScreen(TFT_BLACK);

      VT100::TextAttr attr = VT100::TextAttr(-1);
      // Save current attributes
      VT100::TextAttr oldAttr = m_attr;
      for (uint16_t row = 0; row < m_rows - 1; ++row) {
        GO.lcd.fillRect(0, row*m_charHeight, m_columns*m_charWidth, m_charHeight, TFT_BLACK);
        for (uint16_t col = 0; col < m_columns; ++col) {
          if (m_screenBuffer[1][row * m_columns + col] != attr) {
            attr = m_attr = VT100::TextAttr(m_screenBuffer[1][row * m_columns + col]);
            setTextAttrs();
          }
          GO.lcd.write(m_screenBuffer[0][row * m_columns + col]);
        }
      }
      // Restore current attributes
      m_attr = oldAttr;
      GO.lcd.fillRect(0, (GO.lcd.height() - m_charHeight) %
          GO.lcd.height(), GO.lcd.width(), m_charHeight, TFT_BLACK);
    } else
      ++m_row;
    setCursor(m_column, m_row);
    m_lockCursor = false;
  }

private:

  void setTextAttrs()
  {
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
    case VT100::TextAttr::C_BLACK:
      color = VT100::Color::COLOR_BLACK;
      break;
    }

    if (m_attr & VT100::TextAttr::BRIGHT)
      GO.lcd.setTextColor(s_colors[color][1]);
    else
      GO.lcd.setTextColor(s_colors[color][0]);
  }

  static uint16_t s_colors[uint8_t(VT100::Color::NUM_COLORS)][2];

  uint8_t m_screenBuffer[2][53*30];

  uint8_t m_charWidth, m_charHeight;

  // VT100::Print interface
protected:

  void writeChar(uint8_t c) override
  {
    if (m_cursorEnable) {
      m_lockCursor = true;
      drawCursor(false);
    }
    switch (c) {
    case '\n':
      if (m_row < m_rows - 1)
        GO.lcd.write('\n');
      scrollLine();
      break;
    case '\r':
      m_column = 0;
      break;
    case 8: //backspace
      if (m_column > 0)
        setCursor(--m_column, m_row);
      else if (m_row > 0) {
        m_column = m_columns - 1;
        setCursor(m_column, --m_row);
      }
      break;
    default:
      GO.lcd.write(c);
      m_screenBuffer[0][m_row * m_columns + m_column] = c;
      m_screenBuffer[1][m_row * m_columns + m_column] = m_attr;
      if (m_column >= m_columns - 1) {
        m_column = 0;
        scrollLine();
      } else
        ++m_column;
    }
    if (m_cursorEnable) {
      drawCursor(true);
      m_lockCursor = false;
    }
  }

  void setCursor(uint8_t x, uint8_t y) override
  {
    if (m_cursorEnable)
      drawCursor(false);
    m_row = y % m_rows;
    m_column = x % m_columns;
    GO.lcd.setCursor(m_column * m_charWidth, m_row * m_charHeight);
    if (m_cursorEnable)
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
    setTextAttrs();
  }

  void resetAttributes() override
  {
    GO.lcd.setTextColor(TFT_LIGHTGREY);
    m_attr = VT100::TextAttr::NO_ATTR;
  }
};

#define TFT_DARKYELLOW 0x7BE0
#define TFT_DARKMAGENTA 0x780F

uint16_t OdroidGoScreen::s_colors[
uint8_t(VT100::Color::NUM_COLORS)][2] = {
  { TFT_BLACK, TFT_DARKGREY}, // COLOR_BLACK
  { TFT_RED, TFT_PINK}, // COLOR_RED
  { TFT_DARKGREEN, TFT_GREEN}, // COLOR_GREEN
  { TFT_DARKYELLOW, TFT_YELLOW}, // COLOR_YELLOW
  { TFT_NAVY, TFT_BLUE}, // COLOR_BLUE
  { TFT_DARKMAGENTA, TFT_MAGENTA}, // COLOR_MAGENTA
  { TFT_DARKCYAN, TFT_CYAN}, // COLOR_CYAN
  { TFT_LIGHTGREY, TFT_WHITE} // COLOR_WHITE
};

static OdroidGoScreen* GOScreen;

void
HAL_terminal_write(HAL_terminal_t t, uint8_t b)
{
  if (t == 0)
    GOScreen->write(b);
}

void
HAL_esp32_odroidgo_init()
{
  GO.begin();
  GOScreen = new OdroidGoScreen();
  GOScreen->begin(1);
  //GO.Speaker.beep();

#if HAL_ESP32_ODROIDGO_QWERTY
  kbd = new ODROIDGO::OKeyboard;
  kbd->begin();
  kbdStream = new ODROIDGO::OKeyboardStream(*kbd);
#endif // HAL_ESP32_ODROIDGO_QWERTY

#if HAL_ESP32_ODROIDGO_GFX

#endif // HAL_ESP32_ODROIDGO_GFX
}

#if HAL_ESP32_ODROIDGO_QWERTY
uint8_t
HAL_terminal_read(HAL_terminal_t t)
{
  if (t == 0) {
    int c = kbdStream->read();
    if (c > 0)
      return c;
    return Serial.read();
  }
  
  return 0;
}

BOOLEAN
HAL_terminal_isdataready(HAL_terminal_t t)
{
  if (t == 0) {
    if (kbdStream->available() > 0 ||
        Serial.available() > 0)
       return TRUE;
  }
  return FALSE;
}
#endif // HAL_ESP32_ODROIDGO_QWERTY

void
HAL_esp32_odroidgo_update()
{
  GO.update();
  GOScreen->update();
}

#if HAL_ESP32_ODROIDGO_GFX

void HAL_gfx_setmode(uint8_t mode)
{
  GOScreen->begin(mode);
}

void
HAL_gfx_setCursor(BOOLEAN state)
{
  GOScreen->enableCursor(state);
}

static const uint16_t gfx_colors[] = {
  0,
  TFT_WHITE,   // HAL_GFX_COLOR_WHITE
  TFT_BLACK,   // HAL_GFX_COLOR_BLACK
  TFT_RED,     // 
  TFT_GREEN,   // 
  TFT_BLUE,    // 
  TFT_CYAN,    // 
  TFT_MAGENTA, //
  TFT_YELLOW
};

static uint16_t gfx_active_colors[2] = {HAL_GFX_COLOR_WHITE, HAL_GFX_COLOR_BLACK};

void
HAL_gfx_setColor(HAL_gfx_color_t color)
{
  gfx_active_colors[0] = color;
}

void
HAL_gfx_setBgColor(HAL_gfx_color_t color)
{
  gfx_active_colors[1] = color;
}

void
HAL_gfx_point(uint16_t x, uint16_t y)
{
  if (gfx_active_colors[0] != HAL_GFX_NOTACOLOR)
    GO.lcd.drawPixel(x, y, gfx_colors[gfx_active_colors[0]]);
}

static uint16_t linex = 0, liney = 0;

void
HAL_gfx_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  if (gfx_active_colors[0] != HAL_GFX_NOTACOLOR)
    GO.lcd.drawLine(x1, y1, x2, y2, gfx_colors[gfx_active_colors[0]]);
  
  linex = x2;
  liney = y2;
}

void
HAL_gfx_lineto(uint16_t x, uint16_t y)
{
  if (gfx_active_colors[0] != HAL_GFX_NOTACOLOR)
    GO.lcd.drawLine(linex, liney, x, y, gfx_colors[gfx_active_colors[0]]);
  
  linex = x;
  liney = y;
}

void
HAL_gfx_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (gfx_active_colors[1] != HAL_GFX_NOTACOLOR)
    GO.lcd.fillRect(x, y, w, h, gfx_colors[gfx_active_colors[1]]);
  if (gfx_active_colors[0] != HAL_GFX_NOTACOLOR)
    GO.lcd.drawRect(x, y, w, h, gfx_colors[gfx_active_colors[0]]);
}

void
HAL_gfx_circle(uint16_t x, uint16_t y, uint16_t r)
{
  if (gfx_active_colors[1] != HAL_GFX_NOTACOLOR)
    GO.lcd.fillCircle(x, y, r, gfx_colors[gfx_active_colors[1]]);
  if (gfx_active_colors[0] != HAL_GFX_NOTACOLOR)
    GO.lcd.drawCircle(x, y, r, gfx_colors[gfx_active_colors[0]]);
}

#if !HAL_GFX_EXPCOLOR_SIMPL
void
HAL_gfx_pointc(uint16_t x, uint16_t y, HAL_gfx_color_t color)
{

}

void
HAL_gfx_linec(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, HAL_gfx_color_t color)
{

}

void
HAL_gfx_rectc(uint16_t x, uint16_t y, uint16_t w, uint16_t h, HAL_gfx_color_t color)
{

}

void
HAL_gfx_circlec(uint16_t x, uint16_t y, uint16_t r, HAL_gfx_color_t color)
{

}
#endif // HAL_GFX_EXPCOLOR_SIMPL
#endif // HAL_ESP32_ODROIDGO_GFX

#endif // HAL_ESP32_ODROIDGO
 
