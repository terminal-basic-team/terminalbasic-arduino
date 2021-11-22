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
 * @file config.hpp
 * @brief Configuration parameters, specific to Terminal-BASIC
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "basic_config.h"

/*
 * Allow '_' symbol in identifiers
 */
#define ALLOW_UNDERSCORE_ID 0

#if USE_REALS
	/*
	 * Mathematical functions support
	 */
	#define USEMATH  1
	#if USEMATH
		/*
		 * SIN COS TAN COT
		 */
		#define M_TRIGONOMETRIC         1
		#define M_HYPERBOLIC            0
		/*
		 * ACS ASN ATN
		 */
		#define M_REVERSE_TRIGONOMETRIC 1
		/*
		 * CBR (cubic root) ...
		 */
		#define M_ADDITIONAL            0
	#endif // USEMATH
#endif // USE_REALS

// Use string functions and operations
#define USE_STRINGOPS      1
#if USE_STRINGOPS
	// GET$ function, returns string from last pressed key symbol
	#define USE_INKEY   1
	// CHR$ function, returns string from the parameter ASCII code
	#define USE_CHR     1
	// ASC function, returns code of the first symbol in a string
	#define USE_ASC     1
	// LEN function, returns length of the string
	#define USE_LEN     1
	// LEFT$ function, returns leftmost part of the string
	#define USE_LEFT    1
	// RIGHT$ function, returns rightmost part of the string
	#define USE_RIGHT   1
	// MID$ function, returns middle part of the string
	#define USE_MID     1
	// An alias for MID$
	#define USE_SEG     1
	// HEX$ function, returns string with hexsadecimal representation of the expression value
	#define USE_HEX     1
	// VAL function, returns numeric value from string
	#define USE_VAL     1
#endif // USE_STRINGOPS
/*
 * Clear program memory on NEW command
 */
#define CLEAR_PROGRAM_MEMORY 1
/**
 * Allow INPUT command with text message e.g. INPUT "A:";A
 */
#define OPT_INPUT_WITH_TEXT  1
/**
 * Implicit arrays without DIM statement
 */
#define OPT_IMPLICIT_ARRAYS  0

#if USE_TEXTATTRIBUTES
	/*
	 * Use ANSI color attributes
	 */
	#define USE_COLORATTRIBUTES       1
	/*
	 * Set print zones width (tab spacing)
	 */
	#define SET_PRINTZNES             1
	#if SET_PRINTZNES
		#define PRINT_ZONE_WIDTH 16
		#define PRINT_ZONES_NUMBER 5
	#endif // SET_PRINTZNES
#endif // USE_TEXTATTRIBUTES

#if USE_SAVE_LOAD
	// Compute checksums while SAVE, LOAD and CHAIN
	#define SAVE_LOAD_CHECKSUM   1
#endif // USE_SAVE_LOAD

// Convert all input to upper register
#define AUTOCAPITALIZE    0

/*
 * External memory filesystem module
 */
#define CONF_USE_EXTMEMFS         0
#if CONF_USE_EXTMEMFS
	// Unix-like file operations
	#define USE_FILEOP 1
#endif

// Use text error strings
#define CONF_ERROR_STRINGS 0

// Arduino IO module
#define CONF_MODULE_ARDUINOIO      0
#if CONF_MODULE_ARDUINOIO
	// TONE command support
	#define CONF_MODULE_ARDUINOIO_TONE 0
	// BEEP command
	#if CONF_MODULE_ARDUINOIO_TONE
		#define CONF_BEEP     1
		#if CONF_BEEP
			#define BEEP_PIN 32
			#define BEEP_FREQ 440
			#define BEEP_DURATION 300
		#endif // CONF_BEEP
	#endif // CONF_MODULE_ARDUINOIO_TONE
#endif // CONF_MODULE_ARDUINOIO

// External EEPROM functions module
#define USE_EXTEEPROM    0
#if USE_EXTEEPROM
	/*
	 * Size in bytes
	 */
	#define EXTEEPROM_SIZE   32768
#endif // USE_EXTEEPROM

/*
 * Indention of the loop bodies
 */
#define LOOP_INDENT      1

/*
 * Indention of the line numbers in LIST output
 */
#define LINE_NUM_INDENT  1

/**
 * Aligned memory access. A bit slower, but necessary on ESP8266, MC68k, etc.
 */
#define CONF_USE_ALIGN 0

/*
 * GFX module
 */
#define USE_GFX          0
#if USE_GFX
// Use drawing functions with explisit color setting
#define GFX_EXP_COLOR 1
// Serial binary graphics protocol
#define SERIAL_GFX    0
#endif

/*
 * Prompt on new line
 */
#define CLI_PROMPT_NEWLINE 1

/*
 * LF character processing
 */
#define LF_NONE    0 // Not handled
#define LF_EAT     1 // Silently eat it
#define LF_NEWLINE 2 // Use as new line
#define PROCESS_LF LF_NEWLINE

/*
 * Input and output for single terminal mode
 */

// Input variants
#define HAL_I       11

// Output variants
#define HAL_O       13

// Input select
#define S_INPUT HAL_I

// Output select
#define S_OUTPUT HAL_O

#if USE_EXTEEPROM
	#define USE_WIRE 1
#else
	#define USE_WIRE 0
#endif

// Use multiterminal mode
#define BASIC_MULTITERMINAL       0

/*
 * Max size of the program line
 */
#define PROGSTRINGSIZE 80

// Number of characters in variable name
#define VARSIZE 5

// Show license message on start
#define LICENSE_TEXT_ON_START 0

#endif // CONFIG_HPP
