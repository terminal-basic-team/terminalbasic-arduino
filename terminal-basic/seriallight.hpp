/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016, 2017 Andrey V. Skvortsov <starling13@mail.ru>
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

#ifndef SERIALLIGHT_HPP
#define SERIALLIGHT_HPP

#include <Arduino.h>
#include <Stream.h>

#define SERIAL_8N1 0x06

class SerialLight : public Stream
{
public:
	SerialLight(
		volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
		volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
		volatile uint8_t *ucsrc, volatile uint8_t *udr);

	void
	begin(unsigned long baud)
	{
		begin(baud, SERIAL_8N1);
	}
	void begin(unsigned long, uint8_t);

	int available() override;
	void flush() override;
	int peek() override;
	int read() override;
	size_t write(uint8_t) override;
private:
	volatile uint8_t * const _ubrrh;
	volatile uint8_t * const _ubrrl;
	volatile uint8_t * const _ucsra;
	volatile uint8_t * const _ucsrb;
	volatile uint8_t * const _ucsrc;
	volatile uint8_t * const _udr;
	bool _hasByte;
	uint8_t _byte;
};

extern SerialLight SerialL;
#ifdef HAVE_HWSERIAL1
extern SerialLight SerialL1;
#endif
#ifdef HAVE_HWSERIAL2
extern SerialLight SerialL2;
#endif

#endif
