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

/**
 * @brief Configuration for the Arduino core HAL implementation
 */

#ifndef HAL_ARDUINO_H
#define HAL_ARDUINO_H

#include "HAL.h"

#define HAL_ARDUINO_TERMINAL_NONE 0
/* Use Serial[N] objects for terminal */
#define HAL_ARDUINO_TERMINAL_SERIAL 1

#define HAL_ARDUINO_TERMINAL HAL_ARDUINO_TERMINAL_NONE

#if AL_ARDUINO_TERMINAL == HAL_ARDUINO_TERMINAL_SERIAL
#define HAL_ARDUINO_TERMINAL_SERIAL_0_BR 115200
#if HAL_TERMINAL_NUM > 0
#define HAL_ARDUINO_TERMINAL_SERIAL_1_BR 115200
#endif /* HAL_TERMINAL_NUM */
#if HAL_TERMINAL_NUM > 1
#define HAL_ARDUINO_TERMINAL_SERIAL_2_BR 115200
#endif /* HAL_TERMINAL_NUM */
#if HAL_TERMINAL_NUM > 2
#define HAL_ARDUINO_TERMINAL_SERIAL_3_BR 115200
#endif /* HAL_TERMINAL_NUM */
#endif /* HAL_ARDUINO_TERMINAL */

#if HAL_EXTMEM

#define HAL_ARDUINO_EXTMEM_NONE 0
/* Standart SD library implementation */
#define HAL_ARDUINO_EXTMEM_SD 1
/* SDFS HAL extmem implementation */
#define HAL_ARDUINO_EXTMEM_SDFS 2
/* I2C eeprom HAL extmem implementation */
#define HAL_ARDUINO_EXTMEM_I2CEEPROM 3

/*
 * Used HAL extmem omplementation
 */
#define HAL_ARDUINO_EXTMEM HAL_ARDUINO_EXTMEM_NONE

#endif /* HAL_EXTMEM */

#if HAL_GFX

/* Serial 2-mode graphics protocol HAL GFX implementation */
#define HAL_ARDUINO_GFX_SERIAL 1
/* UTFT HAL GFX implementation */
#define HAL_ARDUINO_GFX_UTFT 2

/*
 * SDL2 HAL_GFX implementation
 */
#define HAL_ARDUINO_GFX HAL_ARDUINO_GFX_SERIAL
#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL
#define HAL_ARDUINO_GFX_SERIAL_TERMNO 0
#endif

#endif /* HAL_GFX */

#endif /* HAL_ARDUINO_H */

