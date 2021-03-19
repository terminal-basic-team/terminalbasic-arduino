/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * 
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
 * Copyright (C) 2019,2020 Terminal-BASIC team
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

#ifdef ARDUINO

#include <string.h>

#include "HAL_arduino.h"
#include "Arduino.h"

#if HAL_EXTMEM && (HAL_ARDUINO_EXTMEM == HAL_ARDUINO_EXTMEM_SDFS)
#include "sd.hpp"
#endif

#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL
#include "gfxterm.hpp"
#endif

__BEGIN_DECLS
void
HAL_initialize_concrete();
__END_DECLS

void
HAL_initialize()
{
#if HAL_ARDUINO_TERMINAL == HAL_ARDUINO_TERMINAL_SERIAL
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


#if HAL_EXTMEM && (HAL_ARDUINO_EXTMEM == HAL_ARDUINO_EXTMEM_SDFS)
	if (!SDCard::SDFS.begin())
		abort();
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

#if HAL_ARDUINO_TERMINAL == HAL_ARDUINO_TERMINAL_SERIAL

void
HAL_terminal_write(HAL_terminal_t t, uint8_t b)
{
	if (t == 0)
		Serial.write(b);
#if defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
	else if (t == 1)
		Serial1.write(b);
#endif
}

uint8_t
HAL_terminal_read(HAL_terminal_t t)
{
	if (t == 0)
		return Serial.read();
#if defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
	else if (t == 1)
		return Serial1.read();
#endif
	return 0;
}

BOOLEAN
HAL_terminal_isdataready(HAL_terminal_t t)
{
	if (t == 0)
		return Serial.available();
#if defined(HAVE_HWSERIAL1) && (HAL_TERMINAL_NUM > 0)
	else if (t == 1)
		return Serial1.available();
#endif
	return FALSE;
}

#endif // HAL_ARDUINO_TERMINAL

#if HAL_EXTMEM && (HAL_ARDUINO_EXTMEM == HAL_ARDUINO_EXTMEM_SDFS)

static SDCard::File files[EXTMEM_NUM_FILES];

static SDCard::File
getRootDir()
{
	SDCard::File root = SDCard::SDFS.open("/",
	    SDCard::Mode::WRITE | SDCard::Mode::READ | SDCard::Mode::CREAT);
	if (!root || !root.isDirectory())
		abort();
}

HAL_extmem_file_t
HAL_extmem_openfile(const char path[13])
{
	uint8_t i=0;
	for (; i<EXTMEM_NUM_FILES; ++i) {
		if (!files[i]) {
			files[i] = SDCard::SDFS.open(path, SDCard::Mode::CREAT|SDCard::Mode::WRITE|
			    SDCard::Mode::READ);
			if (files[i])
				return i+1;
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
	SDCard::SDFS.exists(path);
}

#endif // HAL_ARDUINO_EXTMEM

#if HAL_GFX && (HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL)

static void
_write16(int16_t v)
{
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(v>>8));
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(v&0xFFu));
}

static HAL_gfx_color_t _colors[2] = {HAL_GFX_NOTACOLOR, HAL_GFX_NOTACOLOR};

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

void
HAL_gfx_setColor(HAL_gfx_color_t color)
{
	_colors[0] = color;
	
	_setColors();
}

void
HAL_gfx_setBgColor(HAL_gfx_color_t color)
{
	_colors[1] = color;
	
	_setColors();
}

void
HAL_gfx_point(uint16_t x, uint16_t y)
{
	_writeCommand(GFXTERM::Command::POINT);
	_write16(x); _write16(y);
}

void
HAL_gfx_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	_writeCommand(GFXTERM::Command::LINE);
	_write16(x1); _write16(y1); _write16(x2); _write16(y2);
}

void
HAL_gfx_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	_writeCommand(GFXTERM::Command::BOX);
	_write16(x); _write16(y); _write16(w); _write16(h);
}

void
HAL_gfx_lineto(uint16_t x, uint16_t y)
{
	
}

void
HAL_gfx_circle(uint16_t x, uint16_t y, uint16_t r)
{
	_writeCommand(GFXTERM::Command::CIRCLEC);
	_write16(x); _write16(y); _write16(r);
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
	_write16(x1); _write16(y1); _write16(x2); _write16(y2);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(color));
}

void
HAL_gfx_rectc(uint16_t x, uint16_t y, uint16_t w, uint16_t h, HAL_gfx_color_t color)
{
	_writeCommand(GFXTERM::Command::BOXC);
	_write16(x); _write16(y); _write16(w); _write16(h);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(color));
}

void
HAL_gfx_circlec(uint16_t x, uint16_t y, uint16_t r, HAL_gfx_color_t color)
{
	_writeCommand(GFXTERM::Command::CIRCLEC);
	_write16(x); _write16(y); _write16(r);
	HAL_terminal_write(HAL_ARDUINO_GFX_SERIAL_TERMNO, uint8_t(color));
}
#endif /* HAL_GFX_EXPCOLOR_SIMPL */

#endif // HAL_ARDUINO_GFX

#endif // ARDUINO
