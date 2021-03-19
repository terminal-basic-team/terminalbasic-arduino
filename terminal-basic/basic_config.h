/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2017-2018 Andrey V. Skvortsov <starling13@mail.ru>
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

#ifndef BASIC_CONFIG_H
#define BASIC_CONFIG_H

#define REAL_NONE     0
#define REAL_SINGLE   1
#define REAL_DOUBLE   2
#define REAL_EXTENDED 3
#define REAL_QUAD     4

/*
 * Real arithmetics
 * 
 * Support of operations with real numbers.
 * When enabled, all variables and arrays, which names are not ending with "$ ! %"
 * are treated as reals. Mathematical functions support depend on this option
 */
#define USE_REALS               REAL_SINGLE

/*
 * Support of 4-byte integer datatype
 * Functions, variables and arrays of long integer type ends with double % mark
 */
#define USE_LONGINT          0

/**
 * DUMP command support
 * This command can be used to check BASIC memory image, variables and arrays list
 */
#define USE_DUMP             0

/*
 * RANDOMIZE command and RND() function support
 */
#define USE_RANDOM           1

/*
 * Support of Darthmouth BASIX-style matrix operations
 */
#define USE_MATRIX           0

/**
 * Support of DATA/READ statements
 */
#define USE_DATA             0

/*
 * Support of DEF FN construct
 */
#define USE_DEFFN            0

/*
 * DELAY command, suspends execution for N ms
 */
#define USE_DELAY           1

/*
 * Allow GO TO OPERATOR in addition to GOTO
 */
#define CONF_SEPARATE_GO_TO     0

/*
 * Support of integer division and modulo operation
 */
#define USE_INTEGER_DIV      1
#if USE_INTEGER_DIV
	/*
	 * Use DIV keyword for integer division in addition to \ operation
	 */
	#define USE_DIV_KW   0
#endif // USE_INTEGER_DIV

/*
 * Use >< as not-equals operator (with default <>)
 */
#define CONF_USE_ALTERNATIVE_NE 1

/*
 * Structured loop support
 */
#define USE_DOLOOP           0

/*
 * SAVE, LOAD and CHAIN commands support
 */
#define USE_SAVE_LOAD        1

/*
 * Use ANSI text attributes
 */
#define USE_TEXTATTRIBUTES   1
#if USE_TEXTATTRIBUTES
	/*
	 * Support of SPC(N) print command
	 */
	#define CONF_USE_SPC_PRINT_COM  1
#endif // USE_TEXTATTRIBUTES

/*
 * STOP and CONTINUE commands support
 */
#define USESTOPCONT       1

#define USE_PEEK_POKE 1

#endif /* BASIC_CONFIG_H */
