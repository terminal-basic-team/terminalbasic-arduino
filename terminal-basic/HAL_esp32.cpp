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

#if HAL_EXTMEM
static File extmem_files[EXTMEM_NUM_FILES];
#endif

__BEGIN_DECLS

void
HAL_initialize_concrete()
{
	if (!SPIFFS.begin(true)) {
		Serial.println("ERROR: SPIFFS.begin");
		exit(1);
	}

	/*Serial.println("Format? [y/n]");
	while (true) {
		while (Serial.available() < 1) delay(100);
		auto r = Serial.read();
		if (r == 'y') {
			if (!SPIFFS.format()) {
				Serial.println("ERROR: SPIFFS.format");
				exit(2);
			}
			break;
		} else if (r == 'n')
			break;
	}*/

	if (SPIFFS.exists("/nvram.bin"))
		f = SPIFFS.open("/nvram.bin", "r+");
	else
		f = SPIFFS.open("/nvram.bin", "w");
	if (!f) {
		Serial.println("ERROR: SPIFFS.open");
		exit(4);
	}
	f.close();
}

__END_DECLS

void
HAL_finalize()
{
}

#if HAL_NVRAM

HAL_nvram_address_t
HAL_nvram_getsize()
{
	return NVRAMSIZE;
}

uint8_t
HAL_nvram_read(HAL_nvram_address_t addr)
{
	f = SPIFFS.open("/nvram.bin", "r");
	if (!f) {
		Serial.println("ERROR: SPIFFS.open");
		exit(2);
	}
	if (!f.seek(uint32_t(addr))) {
		Serial.println("ERROR: SPIFFS.open");
		exit(3);
	}
	uint8_t r = f.read();
	f.close();
	return r;
}

void
HAL_nvram_write(HAL_nvram_address_t addr, uint8_t b)
{
	f = SPIFFS.open("/nvram.bin", "r+");
	if (!f) {
		Serial.println("ERROR: SPIFFS.open");
		exit(5);
	}

	if (f.size() > uint32_t(addr)) {
		if (!f.seek(uint32_t(addr))) {
			Serial.println("ERROR: f.seek(addr)");
			exit(6);
		}
	} else {
		if (!f.seek(f.size())) {
			Serial.println("ERROR: f.seek(f.size())");
		}
		while (f.size() < uint32_t(addr)) {
			f.write(0xFF);
			f.flush();
		}
	}

	auto s = f.write(b);
	Serial.println(s, DEC);
	f.close();
}

#endif // HAL_NVRAM

#if HAL_EXTMEM

HAL_extmem_file_t
HAL_extmem_openfile(const char fname[13])
{
	size_t i = 0;
	for (; i < EXTMEM_NUM_FILES; ++i) {
		if (!extmem_files[i])
			break;
	}

	if (i == EXTMEM_NUM_FILES) {
		Serial.println("ERROR: Maximum opened files reached");
		return 0;
	}

	char fname_[14];
	fname_[0] = '/';
	strncpy(fname_ + 1, fname, 12);
	if (SPIFFS.exists(fname_))
		extmem_files[i] = SPIFFS.open(fname_, "r+");
	else
		extmem_files[i] = SPIFFS.open(fname_, "w");
	if (!extmem_files[i]) {
		Serial.println("ERROR: SPIFFS.open");
		return 0;
	}

	return i + 1;
}

void
HAL_extmem_closefile(HAL_extmem_file_t file)
{
	if ((file == 0)
	|| (file > EXTMEM_NUM_FILES)
	|| (!extmem_files[file - 1])) {
		Serial.println("ERROR: HAL_extmem_closefile");
		return;
	}

	extmem_files[file - 1].close();
}

uint32_t
_seek(HAL_extmem_file_t file, uint32_t pos, SeekMode whence)
{
	if ((file == 0)
	|| (file > EXTMEM_NUM_FILES)
	|| (!extmem_files[file - 1]))
		return 0;

	extmem_files[file - 1].seek(pos, whence);
	return pos;
}

void
HAL_extmem_setfileposition(
			   HAL_extmem_file_t file,
			   HAL_extmem_fileposition_t pos)
{
	_seek(file, pos, SeekSet);
}

HAL_extmem_fileposition_t
HAL_extmem_getfilesize(HAL_extmem_file_t file)
{
	if ((file == 0)
	|| (file > EXTMEM_NUM_FILES)
	|| (!extmem_files[file - 1]))
		return 0;

	return extmem_files[file - 1].size();
}

uint16_t
HAL_extmem_getnumfiles()
{
	uint16_t result = 0;
	File d = SPIFFS.open("/");
	if (!d.isDirectory())
		return 0;

	d.rewindDirectory();
	File f = d.openNextFile();
	while (f) {
		++result;
		f = d.openNextFile();
	}

	Serial.print("Numfiles ");
	Serial.println(result);
	return result;
}

void
HAL_extmem_getfilename(uint16_t num, char fname[13])
{
	fname[0] = '\0';

	File d = SPIFFS.open("/");
	if (!d || !d.isDirectory())
		return;

	d.rewindDirectory();
	File f = d.openNextFile();
	while (f) {
		if (num == 0) {
			strncpy(fname, f.name() + 1, 12);
			break;
		}
		f = d.openNextFile();
		num--;
	}
}

void
HAL_extmem_deletefile(const char fname[13])
{
	char fname_[14];
	fname_[0] = '/';
	strncpy(fname_ + 1, fname, 12);
	if (!SPIFFS.remove(fname_)) {
		Serial.println("ERROR: SPIFFS.remove");
		return;
	}
}

uint8_t
HAL_extmem_readfromfile(HAL_extmem_file_t file)
{
	if ((file == 0)
	|| (file > EXTMEM_NUM_FILES)
	|| !extmem_files[file - 1])
		return 0;

	return extmem_files[file - 1].read();
}

void
HAL_extmem_writetofile(HAL_extmem_file_t file, uint8_t b)
{
	if ((file == 0)
	|| (file > EXTMEM_NUM_FILES)
	|| !extmem_files[file - 1])
		return;

	extmem_files[file - 1].write(b);
}

HAL_extmem_fileposition_t
HAL_extmem_getfileposition(HAL_extmem_file_t file)
{
	if ((file == 0)
	|| (file > EXTMEM_NUM_FILES)
	|| !extmem_files[file - 1])
		return 0;

	return extmem_files[file - 1].position();
}

BOOLEAN
HAL_extmem_fileExists(const char fname[13])
{
	char fname_[14];
	fname_[0] = '/';
	strncpy(fname_ + 1, fname, 12);
	return SPIFFS.exists(fname_);
}

#endif // HAL_EXTMEM

#endif // ARDUINO_ARCH_ESP32
