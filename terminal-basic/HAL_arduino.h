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

/**
 * @brief Configuration for the Arduino core HAL implementation
 */

#ifndef HAL_ARDUINO_H
#define HAL_ARDUINO_H

#include "HAL.h"
#include "_HAL_arduino.h"

/**
 * Output devices
 *
 * HAL_ARDUINO_TERMINAL_OUTPUT_NONE - output not used (implemented outside aeduino module)
 * HAL_ARDUINO_TERMINAL_OUTPUT_SERIAL - Serial[N] objects from arduino library
 * HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341 - Use Adafruit GFX library for 240x320 ILI9341 tft displays
 * HAL_ARDUINO_TERMINAL_OUTPUT_TFTeSPI - Use Use TFT_eSPI library
 */
#define HAL_ARDUINO_TERMINAL_OUTPUT HAL_ARDUINO_TERMINAL_OUTPUT_NONE

/**
 * Input devices
 *
 * HAL_ARDUINO_TERMINAL_INPUT_NONE
 * HAL_ARDUINO_TERMINAL_INPUT_SERIAL
 * HAL_ARDUINO_TERMINAL_INPUT_PS2KBD
 */
#define HAL_ARDUINO_TERMINAL_INPUT HAL_ARDUINO_TERMINAL_INPUT_NONE

#if (HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_SERIAL) || \
    (HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_SERIAL)
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
#endif
#if HAL_ARDUINO_TERMINAL_OUTPUT == HAL_ARDUINO_TERMINAL_OUTPUT_ILI9341
	#define TFT_DC 27
	#define TFT_CS 2
	#define TFT_RS 4
#endif /* HAL_ARDUINO_TERMINAL_OUTPUT */

#if HAL_ARDUINO_TERMINAL_INPUT == HAL_ARDUINO_TERMINAL_INPUT_PS2KBD
#define HAL_ARDUINO_TERMINAL_INPUT_PS2KBD_DATAPIN 32
#define HAL_ARDUINO_TERMINAL_INPUT_PS2KBD_CLKPIN 33
#endif

#if HAL_EXTMEM

/*
 * External memory (storage) implementation
 *
 * HAL_ARDUINO_EXTMEM_NONE - No extmem implementation
 * HAL_ARDUINO_EXTMEM_SD - Standard SD library implementation
 * HAL_ARDUINO_EXTMEM_SDFS - SDFS library implememntation
 * HAL_ARDUINO_EXTMEM_I2CEEPROM - I2C eeprom HAL extmem implementation
 */
#define HAL_ARDUINO_EXTMEM HAL_ARDUINO_EXTMEM_SDFS

#endif /* HAL_EXTMEM */

#if HAL_GFX

/*
 * Used GFX implementation
 *
 * HAL_ARDUINO_GFX_NONE - No GFX implementation (implemented outside)
 * HAL_ARDUINO_GFX_SERIAL - Serial binary protocol implementation
 * HAL_ARDUINO_GFX_UTFT - UTFT HAL GFX implementation
 * HAL_ARDUINO_GFX_ILI9341 - Implementation based on Adafruit lib for ILI9341 displays
 * HAL_ARDUINO_GFX_TFTeSPI - TFTeSPI library implementation
 */
#define HAL_ARDUINO_GFX HAL_ARDUINO_GFX_SERIAL

#if HAL_ARDUINO_GFX == HAL_ARDUINO_GFX_SERIAL
#define HAL_ARDUINO_GFX_SERIAL_TERMNO 0
#endif

#endif /* HAL_GFX */

#if HAL_GPIO
/**
 * HAL_GPIO_ARDUINO_NONE - No GPIO implementation
 * HAL_GPIO_ARDUINO_CORE - Implementation based on Arduino-core functions (digitalRead, digitalWrite...)
 */
#define HAL_GPIO_ARDUINO HAL_GPIO_ARDUINO_CORE
#endif /* HAL_GPIO */

#if HAL_BUZZER
/**
 * HAL_BUZZER_ARDUINO_NONE - No buzzer implementation
 * HAL_BUZZER_ARDUINO_TONE - Implementation, base4d on tone function
 */
#define HAL_BUZZER_ARDUINO HAL_BUZZER_ARDUINO_TONE
#endif /* HAL_BUZZER */

#endif /* HAL_ARDUINO_H */
