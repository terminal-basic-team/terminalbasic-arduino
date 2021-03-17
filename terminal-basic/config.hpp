/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016, 2017 Andrey V. Skvortsov <starling13@mail.ru>
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
 * @brief Configuration parameters, common among versions
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdint.h>

namespace BASIC
{

/*
 * Allow '_' symbol in identifiers
 */
#define ALLOW_UNDERSCORE_ID 1
	
/*
 * DELAY command, suspends execution for N ms
 */
#define USE_DELAY    1

/*
 * Real arithmetics
 * 
 * Support of operations with real numbers.
 * When enabled, all variables and arrays, which names are not ending with "$ ! %"
 * are treated as reals. Mathematical functions support depend on this option
 */
#define USE_REALS           1

#if USE_REALS
/*
 * Mathematical functions support
 */
#define USEMATH 	        1

#if USEMATH
/*
 * SIN COS TAN COT
 */
#define M_TRIGONOMETRIC         1
/*
 * ACS ASN ATN
 */
#define M_REVERSE_TRIGONOMETRIC	0
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
#define USE_GET            1
// CHR$ function, returns string from the parameter ASCII code
#define USE_CHR            1
// ASC function, returns code of the first symbol in a string
#define USE_ASC            1
// LEN function, returns length of the string
#define USE_LEN            1
#endif

/**
 * Allow GO TO OPERATOR in addition to GOTO
 */
#define CONF_SEPARATE_GO_TO     1
/*
 * Use >< as not-equals operator (with default <>)
 */
#define CONF_USE_ALTERNATIVE_NE 0
/*
 * Support of 4-byte integer datatype
 * Functions, variables and arrays of long integer type ends with double % mark
 */
#define USE_LONGINT          0
/**
 * DUMP command support
 * This command can be used to see BASIC memory image, variables and arrays list
 */
#define USE_DUMP             0
/*
 * Clear program memory on NEW command
 */
#define CLEAR_PROGRAM_MEMORY 1
/*
 * RANDOMIZE command and RND() function support
 */
#define USE_RANDOM           1
/*
 * Support of Darthmouth BASIX-style matrix operations
 */
#define USE_MATRIX           1
/**
 * Support of DATA/READ statements
 */
#define USE_DATA             0
/*
 * Use vt100 text attributes
 */
#define USE_TEXTATTRIBUTES   1
#if USE_TEXTATTRIBUTES
/*
 * Use vt100 color attributes
 */
#define USE_COLORATTRIBUTES  1
/*
 * Support of SPC(N) print command
 */
#define CONF_USE_SPC_PRINT_COM  1
#endif // USE_TEXTATTRIBUTES

#define USE_SAVE_LOAD        1 // SAVE, LOAD and CHAIN commands support
#if USE_SAVE_LOAD
#define SAVE_LOAD_CHECKSUM   1 // Compute checksums while SAVE, LOAD and CHAIN
#endif // USE_SAVE_LOAD
/*
 * STOP and CONTINUE commands support
 */
#define USESTOPCONT       1
#define AUTOCAPITALIZE    0 // Convert all input to upper register

#define OPT_SPEED     1
#define OPT_SIZE      2
#define OPT           OPT_SIZE

#define USESD         0 // SDcard module

#define CONF_MODULE_ARDUINOIO      1 // Arduino IO module
#if CONF_MODULE_ARDUINOIO
#define CONF_MODULE_ARDUINOIO_TONE 1
#endif // CONF_MODULE_ARDUINOIO

#define USE_EXTEEPROM    0 // External EEPROM functions module
#if USE_EXTEEPROM
/*
 * Size in bytes
 */
#define EXTEEPROM_SIZE   32768
#endif // USE_EXTEEPROM

#define USE_DOLOOP       0
/*
 * Indention of the loop bodies
 */
#define LOOP_INDENT      1

#define LINE_NUM_INDENT  1

/*
 * GFX module
 */
#define USE_GFX          0
    
    
/*
 * Input and output for single terminal mode
 */

// Input variants
#define SERIAL_I    1  // Serial input
#define SERIAL1_I   2  // Serial1 input
#define SERIALL_I   3  // SerialL input
#define SERIALL1_I  4  // SerialL1 input
#define SERIALL2_I  5  // SerialL2 input
#define SERIALL3_I  6  // SerialL3 input
#define PS2UARTKB_I 7  // PS/2 keyboard through USART

// Output variants
#define SERIAL_O   1 // Serial output
#define SERIAL1_O  2 // Serial1 output
#define SERIALL_O  3 // SerialL output
#define SERIALL1_O 4 // SerialL1 output
#define SERIALL2_O 5 // SerialL2 output
#define SERIALL3_O 6 // SerialL3 output
#define UTFT_O     7 // UTFT output
#define TVOUT_O    8 // TVout output
	#define TVOUT_HORIZ 240
	#define TVOUT_VERT 192

/*
 * Max size of the program line
 */
const uint8_t PROGSTRINGSIZE = 73;

// Max size of the string constants/variables
const uint8_t STRINGSIZE = 65;

// Number of characters in variable name
const uint8_t VARSIZE = 5;

} // namespace BASIC

#endif // CONFIG_HPP
