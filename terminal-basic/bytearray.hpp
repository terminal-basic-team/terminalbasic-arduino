/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016  Andrey V. Skvortsov <starling13@mail.ru>
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

#ifndef BYTEARRAY_HPP
#define BYTEARRAY_HPP

#include <inttypes.h>

#include "Printable.h"

class ByteArray : public Printable
{
public:
	ByteArray(const uint8_t*, size_t);
	ByteArray(const char *c, size_t s) :
	ByteArray(reinterpret_cast<const uint8_t*>(c), s) {}

	size_t printTo(Print& p) const override;

	const uint8_t *data() const
	{
		return (_data);
	}

	size_t size() const
	{
		return (_size);
	}
private:
	const uint8_t *_data;
	size_t _size;
};

#endif
