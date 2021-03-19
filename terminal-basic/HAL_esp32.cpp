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

#ifdef ARDUINO_ARCH_ESP32

#include "HAL.h"
#include "FS.h"
#include "SPIFFS.h"

#define NVRAMSIZE 65536

static File f;

void
HAL_initialize()
{
	if (!SPIFFS.begin(false))
		if (!SPIFFS.begin(true))
			exit(1);
	
	f = SPIFFS.open("/nvram.bin", "r+");
	if (!f) {
		f = SPIFFS.open("/nvram.bin", "w");
		if (!f)
			exit(4);
		f.close();
	}
}

void
HAL_finalize()
{
}

HAL_nvram_address_t
HAL_nvram_getsize()
{
	return NVRAMSIZE;
}

uint8_t
HAL_nvram_read(HAL_nvram_address_t addr)
{
	f = SPIFFS.open("/nvram.bin", "r");
	if (!f)
		exit(2);
	if (!f.seek(uint32_t(addr)))
		exit(3);
	uint8_t r = f.read();
	f.close();
	return r;
}

void
HAL_nvram_write(HAL_nvram_address_t addr, uint8_t b)
{
	Serial.println((uint32_t) addr, HEX);
	f = SPIFFS.open("/nvram.bin", "r+");
	if (!f) {
		exit(5);
	}

	if (f.size() > uint32_t(addr)) {
		if (!f.seek(uint32_t(addr)))
			exit(6);
	} else {
		if (!f.seek(f.size()))
			exit(7);
		while (f.size() < uint32_t(addr)) {
			f.write(0xFF);
		}
	}

	f.write(b);
	f.close();
}

#endif // ARDUINO_ARCH_ESP32
