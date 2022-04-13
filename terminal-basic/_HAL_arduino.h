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

#ifndef _HAL_ARDUINO_H
#define _HAL_ARDUINO_H

/** Output devices **/

/* Output not used */
#define HAL_ARDUINO_TERMINAL_OUTPUT_NONE 0
/* Use Serial[N] objects for output */
#define HAL_ARDUINO_TERMINAL_OUTPUT_SERIAL 1
/* Use Adafruit GFX library for 240x320 ILI9341 tft displays */
#define HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341 2
/* Use Use TFT_eSPI library */
#define HAL_ARDUINO_TERMINAL_OUTPUT_TFTeSPI 3

/** Input devices */

/* Do not use input device (implemented outside) */
#define HAL_ARDUINO_TERMINAL_INPUT_NONE 0
/* Serial[N] objects from arduino library */
#define HAL_ARDUINO_TERMINAL_INPUT_SERIAL 1
/* PS/2 keyboard using libkbddup */
#define HAL_ARDUINO_TERMINAL_INPUT_PS2KBD 2

/** GFX-implementation **/

/* No GFX implementation (implemented outside) */
#define HAL_ARDUINO_GFX_NONE 0
/* Serial 2-mode graphics protocol HAL GFX implementation */
#define HAL_ARDUINO_GFX_SERIAL 1
/* UTFT HAL GFX implementation */
#define HAL_ARDUINO_GFX_UTFT 2
/* Implementation based on Adafruit lib for ILI9341 displays */
#define HAL_ARDUINO_GFX_ILI9341 3
/* TFTeSPI library implementation */
#define HAL_ARDUINO_GFX_TFTeSPI 4

/** External memory (storage) implementation **/
#define HAL_ARDUINO_EXTMEM_NONE 0
/* Standart SD library implementation */
#define HAL_ARDUINO_EXTMEM_SD 1
/* SDFS HAL extmem implementation */
#define HAL_ARDUINO_EXTMEM_SDFS 2
/* I2C eeprom HAL extmem implementation */
#define HAL_ARDUINO_EXTMEM_I2CEEPROM 3

/** GPIO implementations **/

/* No GPIO implementation */
#define HAL_GPIO_ARDUINO_NONE 0
/* Implementation based on Arduino-core functions (digitalRead, digitalWrite...) */
#define HAL_GPIO_ARDUINO_CORE 1

/** Buzzer implementation **/

/* No buzzer implementation */
#define HAL_BUZZER_ARDUINO_NONE 0
/* Implementation, base4d on tone function */
#define HAL_BUZZER_ARDUINO_TONE 1

#endif /* _HAL_ARDUINO_H */
