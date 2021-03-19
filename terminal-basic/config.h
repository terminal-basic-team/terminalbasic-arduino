/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2017-2020 Andrey V. Skvortsov <starling13@mail.ru>
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
 * @file basic_config.h
 * @brief Configuration parameters, common among versions
 */

#ifndef BASIC_CONFIG_H
#define BASIC_CONFIG_H

#include <stdint.h>

/*
 * Real arithmetics
 * 
 * Support of operations with real numbers.
 * When enabled, all variables and arrays, which names are not ending with "$ ! %"
 * are treated as reals. Mathematical functions support depend on this option
 */
#define USE_REALS            1
#if USE_REALS
#define USE_LONG_REALS       0
#endif

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
 * Allow ON ... GOTO ... statements
 */
#define CONF_USE_ON_GOTO    0

/*
 * Allow GO TO OPERATOR in addition to GOTO
 */
#define CONF_SEPARATE_GO_TO     0

/*
 * Fast command call using function address
 */
#define FAST_MODULE_CALL 1

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
#define CONF_USE_ALTERNATIVE_NE 0

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

/**
 * PEEK FUNCTION, POKE command support
 */
#define USE_PEEK_POKE 1

#define LANG_EN 0
#define LANG_RU 1
#define LANG_FR 3

/*
 * Messages localization
 */
#define CONF_LANG LANG_EN

/*
 * Lexer localization
 */
#define CONF_LEXER_LANG LANG_EN

/* Size of the string identifiers */
#define STRING_SIZE 72

#define USE_PACKED_STRUCT 1

/*
 * High-level code optimisation mode
 */
#define OPT_SPEED     1 // Extensive use of switch/case constructs
#define OPT_SIZE      2 // Use cascade of if/else if instead of switch/case
#define OPT           OPT_SIZE // Selected mode

typedef uintptr_t pointer_t;

#endif /* BASIC_CONFIG_H */
