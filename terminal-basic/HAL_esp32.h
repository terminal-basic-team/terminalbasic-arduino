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
 * @brief Configuration for the Arduino core ESP32 HAL implementation
 */

#ifndef HAL_ESP32_H
#define HAL_ESP32_H

#include "HAL_arduino.h"

#if HAL_NVRAM

// Size of the SPIFFS file, used as NVRAM storage
#define NVRAMSIZE 65536

#endif /* HAL_NVRAM */

#if HAL_EXTMEM

/** External memory implementations **/
/* SPIFFS external memory implementation */
#define HAL_ESP32_EXTEM_SPIFFS 1
/* SD card external memory implementation */
#define HAL_ESP32_EXTEM_SD 2

/* Used external memory implementation */
#define HAL_ESP32_EXTMEM HAL_ESP32_EXTEM_SD

#endif /* HAL_EXTMEM */

void analogWrite(uint8_t, uint8_t);

#if HAL_BUZZER

#define HAL_BUZZER_ESP32_NONE 0
#define HAL_BUZZER_ESP32_PWM 1

#define HAL_BUZZER_ESP32 HAL_BUZZER_ESP32_PWM

#endif /* HAL_BUZZER */

/* ODROID-GO Hardware support */
#define HAL_ESP32_ODROIDGO 0
#if HAL_ESP32_ODROIDGO
/* Use ODROID-GO qwerty keyboard for input */
#define HAL_ESP32_ODROIDGO_QWERTY 1
#if HAL_GFX
#define HAL_ESP32_ODROIDGO_GFX 1
#endif
#endif /* HAL_ESP32_ODROIDGO */

#endif /* HAL_ESP32_H */
