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

#include "bytearray.hpp"

#include "Print.h"

ByteArray::ByteArray(const uint8_t *data, size_t size) :
_data(data), _size(size)
{
}

size_t
ByteArray::printTo(Print& p) const
{
	size_t res = 0;
	for (size_t i = 0; i < size();) {
		size_t ii = i;

		// Leading zeros of the absolute address
		size_t addr = i + uintptr_t(data());
		uint8_t digits;
		for (digits = 0; addr > 15; addr >>= 4, ++digits);
		while (++digits < sizeof(intptr_t)*2)
			p.print('0');
		
		p.print(i + intptr_t(data()), HEX), p.print('(');
		
		// Leading zeros of the relative address
		addr = i;
		for (digits = 0; addr > 15; addr >>= 4, ++digits);
		while (++digits < sizeof(intptr_t)*2)
			p.print('0');
		
		p.print(i, HEX), p.print(')'), p.print(":\t");
		size_t j;
		for (j = 0; j < 8; ++j, ++ii) {
			if (ii >= size())
				break;
			uint8_t c = data()[ii];
			if (c > 0x0F) {
				res += p.print(' ');
				res += p.print(c, 16);
			} else {
				res += p.print(' ');
				res += p.print('0');
				res += p.print(c, 16);
			}
		}
		for (; j < 8; ++j)
			res += p.print("   ");
		res += p.print('\t');
		for (size_t j = 0; j < 8; ++j, ++i) {
			if (i >= size())
				break;
			const char c = ((const char*) data())[i];
			if (c < ' ')
				res += p.print('.');
			else
				res += p.print(c);
		}
		res += p.println();
	}
	return (res);
}
