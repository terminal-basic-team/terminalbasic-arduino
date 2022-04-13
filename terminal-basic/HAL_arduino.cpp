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

#ifdef ARDUINO

#include <string.h>

#include "HAL_arduino.h"
#include "Arduino.h"

#if HAL_EXTMEM && (HAL_ARDUINO_EXTMEM == HAL_ARDUINO_EXTMEM_SDFS)
#include "sd.hpp"
#endif

#if (HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341) || \
    (HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_TFTeSPI)
#include "vt100.hpp"
#include "HAL_arduino_gfx.hpp"
#endif

#if HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_PS2KBD
#include "ps2_keyboardstream.hpp"
#endif

#if HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_PS2KBD
static PS2::Keyboard kbd;
static PS2::KeyboardStream kstream(kbd);
#endif

#if HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_TFTeSPI

#include "TFT_eSPI.h"

#define TFT_ROWS 16
#define TFT_COLS 21

class ESPI : public VT100::Print
{
public:

#define CURSOR_BLINK_PERIOD 20

  ESPI()
  {
    m_row = m_column = m_scroll = 0;
    m_rows = TFT_ROWS, m_columns = TFT_COLS;
    m_attr = VT100::TextAttr::NO_ATTR;
    m_lockCursor = false;
    m_cursorCounter = CURSOR_BLINK_PERIOD;
    memset(m_screenBuffer[0], ' ', m_rows*m_columns);
    memset(m_screenBuffer[1], VT100::TextAttr::NO_ATTR, m_rows*m_columns);
  }

  void begin()
  {
    tft.begin();
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setTextSize(1);
  }

  void clear() override
  {
    memset(m_screenBuffer[0], ' ', m_rows*m_columns);
    memset(m_screenBuffer[1], ' ', m_rows*m_columns);
    tft.fillScreen(TFT_BLACK);
    m_scroll = 0;
    //tft.scrollTo(m_scroll);
    setCursor(0, 0);
    drawCursor(true);
  }

  void onTimer()
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

  TFT_eSPI tft;

protected:

  void drawCursor(bool v)
  {
    tft.fillRect(tft.getCursorX(), tft.getCursorY(), 6, 8,
        v ? TFT_WHITE : TFT_BLACK);
  }

  void scrollLine()
  {
    if (m_row >= m_rows-1) {
      //m_scroll += 8;
      //if (m_scroll >= tft.height())
      //  m_scroll -= tft.height();
      //tft.scrollTo(m_scroll);
      memmove(m_screenBuffer[0], m_screenBuffer[0]+m_columns, m_columns*(m_rows-1));
      memmove(m_screenBuffer[1], m_screenBuffer[1]+m_columns, m_columns*(m_rows-1));

      memset(m_screenBuffer[0]+m_columns*(m_rows-1), ' ', m_columns);
      memset(m_screenBuffer[1]+m_columns*(m_rows-1), VT100::TextAttr::NO_ATTR, m_columns);

      tft.setCursor(0, 0);
      tft.fillScreen(TFT_BLACK);

      VT100::TextAttr attr = VT100::TextAttr::NO_ATTR;
      // Save current attributes
      VT100::TextAttr oldAttr = m_attr;
      for (uint8_t row=0; row<m_rows-1; ++row)
        for (uint8_t col=0; col<m_columns; ++col) {
          if (m_screenBuffer[1][row*m_columns + col] != attr) {
            attr = m_attr = VT100::TextAttr(m_screenBuffer[1][row*m_columns + col]);
            setTextAttrs();
          }
          tft.write(m_screenBuffer[0][row*m_columns + col]);
        }
      // Restore current attributes
      m_attr = oldAttr;
      tft.fillRect(0, (tft.height() - 8 + m_scroll) %
          tft.height(), tft.width(), 8, TFT_BLACK);
    } else
      ++m_row;
    setCursor(m_column, m_row);
  }

  void writeChar(uint8_t c) override
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
      m_screenBuffer[0][m_row*m_columns+m_column] = c;
      m_screenBuffer[1][m_row*m_columns+m_column] = m_attr;
      if (m_column >= m_columns-1) {
        m_column = 0;
        scrollLine();
      } else
        ++m_column;
    }
    drawCursor(true);
    m_lockCursor = false;
  }

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
    setTextAttrs();
  }

  void resetAttributes() override
  {
    tft.setTextColor(TFT_LIGHTGREY);
    m_attr = VT100::TextAttr::NO_ATTR;
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
      tft.setTextColor(pgm_read_word(&s_colors[color][1]));
    else
      tft.setTextColor(pgm_read_word(&s_colors[color][0]));
  }

  uint8_t m_row, m_column;

  uint8_t m_rows, m_columns;

  uint16_t m_scroll;

  VT100::TextAttr m_attr;

  static uint16_t s_colors[uint8_t(VT100::Color::NUM_COLORS)][2] PROGMEM;

  bool m_lockCursor, m_cursorState;

  uint8_t m_cursorCounter;

  uint8_t m_screenBuffer[2][TFT_COLS*TFT_ROWS];
};

#define TFT_DARKYELLOW 0x7BE0
#define TFT_DARKMAGENTA 0x780F

uint16_t ESPI::s_colors[uint8_t(VT100::Color::NUM_COLORS)][2] PROGMEM = {
  { TFT_BLACK,       TFT_DARKGREY }, // COLOR_BLACK
  { TFT_RED,         TFT_PINK }, // COLOR_RED
  { TFT_DARKGREEN,   TFT_GREEN }, // COLOR_GREEN
  { TFT_DARKYELLOW,  TFT_YELLOW },  // COLOR_YELLOW
  { TFT_NAVY,        TFT_BLUE },  // COLOR_BLUE
  { TFT_DARKMAGENTA, TFT_MAGENTA },  // COLOR_MAGENTA
  { TFT_DARKCYAN,    TFT_CYAN },  // COLOR_CYAN
  { TFT_LIGHTGREY,   TFT_WHITE } // COLOR_WHITE
};

static ESPI espi;

#endif // HAL_ARDUINO_TERMINAL_OUTPUT

__BEGIN_DECLS
void
HAL_initialize_concrete();
__END_DECLS

void
HAL_initialize()
{
#if (HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_SERIAL) || \
    (HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_SERIAL)
	Serial.begin(HAL_ARDUINO_TERMINAL_SERIAL_0_BR);
#if defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
	Serial1.begin(HAL_ARDUINO_TERMINAL_SERIAL_1_BR);
#endif
#if defined(HAVE_HWSERIAL2) && (HAL_TERMINAL_NUM > 1)
	Serial2.begin(HAL_ARDUINO_TERMINAL_SERIAL_2_BR);
#endif
#if defined(HAVE_HWSERIAL3) && (HAL_TERMINAL_NUM > 2)
	Serial3.begin(HAL_ARDUINO_TERMINAL_SERIAL_3_BR);
#endif
#endif // HAL_ARDUINO_TERMINAL

#if HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341 || \
    HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_TFTeSPI
	espi.begin();
#endif // HAL_ARDUINO_TERMINAL

#if HAL_EXTMEM && (HAL_ARDUINO_EXTMEM == HAL_ARDUINO_EXTMEM_SDFS)
	if (!SDCard::SDFS.begin())
		abort();
#endif

#if HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_PS2KBD
  kbd.begin(HAL_ARDUINO_TERMINAL_INPUT_PS2KBD_DATAPIN, HAL_ARDUINO_TERMINAL_INPUT_PS2KBD_CLKPIN);
#endif
	HAL_initialize_concrete();
}

void
HAL_time_sleep_ms(uint32_t ms)
{
	delay(ms);
}

uint32_t
HAL_time_gettime_ms()
{
	return millis();
}

#if HAL_ARDUINO_TERMINAL_OUTPUT != HAL_ARDUINO_TERMINAL_OUTPUT_NONE

void
HAL_terminal_write(HAL_terminal_t t, uint8_t b)
{
#if HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_SERIAL
	if (t == 0)
		Serial.write(b);
#if defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
	else if (t == 1)
		Serial1.write(b);
#endif
#elif HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341 || \
      HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_TFTeSPI
	if (t == 0)
		espi.write(b);
#endif
}

#endif // #if HAL_ARDUINO_TERMINAL_OUTPUT

#if HAL_ARDUINO_TERMINAL_INPUT != HAL_ARDUINO_TERMINAL_INPUT_NONE

uint8_t
HAL_terminal_read(HAL_terminal_t t)
{
#if HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_SERIAL
	if (t == 0)
		return Serial.read();
#if defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
	else if (t == 1)
		return Serial1.read();
#endif // defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
#elif HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_PS2KBD
  if (t == 0)
    return kstream.read();
#endif // HAL_ARDUINO_TERMINAL_INPUT
	return 0;
}

BOOLEAN
HAL_terminal_isdataready(HAL_terminal_t t)
{
#if HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_SERIAL
	if (t == 0)
		return Serial.available();
#if defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
	else if (t == 1)
		return Serial1.available();
#endif // defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
#elif HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_PS2KBD
  if (t == 0)
    return kstream.available();
#endif // HAL_ARDUINO_TERMINAL_INPUT
	return FALSE;
}

#endif // HAL_ARDUINO_TERMINAL

#if HAL_EXTMEM && (HAL_ARDUINO_EXTMEM == HAL_ARDUINO_EXTMEM_SDFS)

static SDCard::File files[HAL_EXTMEM_NUM_FILES];

static SDCard::File
getRootDir()
{
	SDCard::File root = SDCard::SDFS.open("/",
	    SDCard::Mode::WRITE | SDCard::Mode::READ | SDCard::Mode::CREAT);
	if (!root || !root.isDirectory())
		abort();
	return root;
}

HAL_extmem_file_t
HAL_extmem_openfile(const char path[13])
{
	uint8_t i=0;
	for (; i<HAL_EXTMEM_NUM_FILES; ++i) {
		if (!files[i]) {
			files[i] = SDCard::SDFS.open(path, SDCard::Mode::CREAT|SDCard::Mode::WRITE|
			    SDCard::Mode::READ);
			if (files[i]) {
				files[i].seek(0);
				return i+1;
			}
		}
	}
	return 0;
}

HAL_extmem_fileposition_t
HAL_extmem_getfilesize(HAL_extmem_file_t f)
{
	if ((f > 0) && files[f-1])
		return files[f-1].size();
	return 0;
}

HAL_extmem_fileposition_t
HAL_extmem_getfileposition(HAL_extmem_file_t f)
{
	if ((f > 0) && files[f-1])
		return files[f-1].position();
	return 0;
}

void
HAL_extmem_setfileposition(HAL_extmem_file_t f, HAL_extmem_fileposition_t pos)
{
	if ((f > 0) && files[f-1])
		files[f-1].seek(pos);
}

uint8_t
HAL_extmem_readfromfile(HAL_extmem_file_t f)
{
	if ((f > 0) && files[f-1])
		return files[f-1].read();
	return 0;
}

void
HAL_extmem_writetofile(HAL_extmem_file_t f, uint8_t b)
{
	if ((f > 0) && files[f-1])
		files[f-1].write(b);
}

void
HAL_extmem_closefile(HAL_extmem_file_t f)
{
	if ((f > 0) && files[f-1]) {
		files[f-1].close();
	}
}

uint16_t
HAL_extmem_getnumfiles()
{
	uint16_t result = 0;
	auto root = getRootDir();
	root.rewindDirectory();
	SDCard::File f;
	while (f=root.openNextFile(SDCard::Mode::READ_ONLY)) {
		++result;
		f.close();
	}
	return result;
}

void
HAL_extmem_getfilename(uint16_t num, char path[13])
{
	auto root = getRootDir();
	root.rewindDirectory();
	SDCard::File f;
	uint16_t n = num;
	while (f=root.openNextFile(SDCard::Mode::READ_ONLY)) {
		if (n-- == 0) {
			strncpy(path, f.name(), 13);
			f.close();
			return;
		}
		f.close();
	}
}

void
HAL_extmem_deletefile(const char path[13])
{
	SDCard::SDFS.remove(path);
}

BOOLEAN
HAL_extmem_fileExists(const char path[13])
{
	return SDCard::SDFS.exists(path);
}

#endif // HAL_ARDUINO_EXTMEM

void
HAL_random_seed(uint32_t seed)
{
	randomSeed(seed);
}

uint32_t
HAL_random_generate(uint32_t max)
{
	return random(max);
}

#if HAL_GPIO

#if HAL_GPIO_ARDUINO == HAL_GPIO_ARDUINO_CORE

void
HAL_gpio_writePin(uint8_t pin, BOOLEAN val)
{
	pinMode(pin, OUTPUT);
	digitalWrite(pin, val ? HIGH : LOW);
}

BOOLEAN
HAL_gpio_readPin(uint8_t pin)
{
	pinMode(pin, INPUT);
	return digitalRead(pin) == HIGH;
}

#endif // HAL_GPIO_ARDUINO

#endif // HAL_GPIO

#if HAL_BUZZER

#if HAL_BUZZER_ARDUINO == HAL_BUZZER_ARDUINO_TONE

void
HAL_buzzer_tone(uint8_t channel, uint16_t freq, uint16_t dur)
{
	pinMode(channel, OUTPUT);
	tone(channel, freq, dur);
}

void
HAL_buzzer_notone(uint8_t channel)
{
	noTone(channel);
}

#endif // HAL_BUZZER_ARDUINO

#endif // HAL_BUZZER

__BEGIN_DECLS
void
HAL_update_concrete();
__END_DECLS

void
HAL_update()
{
#if HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341 || \
    HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_TFTeSPI
	espi.onTimer();
#endif

	HAL_update_concrete();
}

#endif // ARDUINO
