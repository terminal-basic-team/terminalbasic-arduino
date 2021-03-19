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

#ifdef ARDUINO_ARCH_ESP8266

#include "HAL.h"
#include "FS.h"

#if HAL_NVRAM
#define NVRAMSIZE 32768
static File f;
#endif

#if HAL_EXTMEM
static File extmem_files[EXTMEM_NUM_FILES];
#endif

__BEGIN_DECLS

void
HAL_initialize_concrete()
{
	if (!SPIFFS.begin())
		exit(1);
#if HAL_NVRAM
	f = SPIFFS.open("/nvram.bin", "r+");
	if (!f) {
		f = SPIFFS.open("/nvram.bin", "w");
		if (!f)
			exit(4);
		f.close();
	}
#endif // HAL_NVRAM
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
	f = SPIFFS.open("/nvram.bin", "r+");
	if (!f)
		exit(5);

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

	if (i == EXTMEM_NUM_FILES)
		return 0;

	char fname_[14];
	fname_[0] = '/';
	strncpy(fname_ + 1, fname, 12);
	extmem_files[i] = SPIFFS.open(fname_, "r+");
	if (!extmem_files[i]) {
		extmem_files[i] = SPIFFS.open(fname_, "w");
		if (!extmem_files[i])
			return 0;
	}

	return i + 1;
}

void
HAL_extmem_closefile(HAL_extmem_file_t file)
{
	if ((file == 0)
	|| (file > EXTMEM_NUM_FILES)
	|| (!extmem_files[file - 1]))
		return;

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
	Dir d = SPIFFS.openDir("");
	if (!d.rewind())
		return 0;

	while (d.next())
		++result;

	return result;
}

void
HAL_extmem_getfilename(uint16_t num, char fname[13])
{
	Dir d = SPIFFS.openDir("");
	if (!d.rewind())
		return;

	fname[0] = '\0';
	while (d.next()) {
		if (num == 0) {
			strncpy(fname, d.fileName().c_str() + 1, 12);
			fname[12] = '\0';
			break;
		}
		num--;
	}
}

void
HAL_extmem_deletefile(const char fname[13])
{
	char fname_[14];
	fname_[0] = '/';
	strncpy(fname_ + 1, fname, 12);
	if (!SPIFFS.remove(fname_))
		Serial.println("ERROR: SPIFFS.remove");
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

#endif // ARDUINO_ARCH_ESP8266
