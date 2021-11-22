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

#ifndef STRINGS_EN_HPP
#define STRINGS_EN_HPP

#define STR_NO_ERROR                  "NO ERROR"
#define STR_OPERATOR_EXPECTED         "OPERATOR EXPECTED"
#define STR_IDENTIFYER_EXPECTED       "IDENTIFIER EXPECTED"
#define STR_EXPRESSION_EXPECTED       "EXPRESSION EXPECTED"
#define STR_INTEGER_CONSTANT_EXPECTED "INTEGER CONSTANT EXPECTED"
#define STR_THEN_OR_GOTO_EXPECTED     "THEN OR GOTO EXPECTED"
#define STR_INVALID_DATA_EXPRESSION   "INVALID DATA EXPRESSION"
#define STR_INVALID_READ_EXPRESSION   "INVALID READ EXPRESSION"
#define STR_VARIABLES_LIST_EXPECTED   "VARIABLES LIST EXPECTED"
#define STR_STRING_OVERFLOW           "STRING OVERFLOW"
#define STR_MISSING_RPAREN            "MISSING RIGHT PARENTHESIS"
#define STR_INVALID_ONGOTO_INDEX      "INVALID ON ... GOTO INDEX"

#define STR_OUTTA_MEMORY              "OUT OF MEMORY"
#define STR_REDIMED_ARRAY             "ARRAY ALREADY ALLOCATED"
#define STR_STACK_FRAME_ALLOC         "UNABLE TO ALLOCATE STACK FRAME"
#define STR_MISING_STRING_FRAME       "MISSING STRING FRAME"
#define STR_INVALID_NEXT              "MISPLACED FOR ... NEXT"
#define STR_RETURN_WO_GOSUB           "RETURN WITHOUT GOSUB"
#define STR_NO_SUCH_LINE              "NO SUCH PROGRAM LINE"
#define STR_INVALID_VALUE_TYPE        "INVALID TYPE"
#define STR_NO_SUCH_ARRAY             "NO SUCH ARRAY"
#define STR_INTEGER_EXP_EXPECTED      "INTEGER EXPECTED"
#define STR_BAD_CHECKSUM              "BAD CHECKSUM"
#define STR_INVALID_TAB               "INVALID TAB VALUE"
#define STR_INVALID_ELEMENT_INDEX     "INVALID ELEMENT INDEX"
#define STR_SQUARE_MATRIX_EXPECTED    "SQUARE_MATRIX_EXPECTED"
#define STR_DIMENSIONS_MISMATCH       "DIMENSIONS MISMATCH"
#define STR_COMMAND_FAILED            "COMMAND EXECUTION FAILED"
#define STR_VAR_DUPLICATE             "DUPLICATE VARIABLE"
#define STR_FUNCTION_DUPLICATE        "DUPLICATE FUNCTION"
#define STR_NO_SUCH_FUNCION           "NO SUCH FUNCTION"
#define STR_INSUFFICIENT_DATA         "INSUFFICIENT DATA FOR READ"

#define STR_LICENSE_MESSAGE \
"Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>\n" \
"Copyright (C) 2019-2021 Terminal-BASIC team\n" \
"    <https://github.com/terminal-basic-team>\n" \
"This program comes with ABSOLUTELY NO WARRANTY.\n" \
"This is free software, and you are welcome to redistribute it\n" \
"under the terms of the GNU General Public License as published by\n" \
"the Free Software Foundation, either version 3 of the License, or\n" \
"(at your option) any later version."

#define STR_ERROR      "ERROR"
#define STR_BYTES      "BYTES"
#define STR_VERSION    "VERSION"
#define STR_AVAILABLE  "AVAILABLE"
#define STR_CLI_PROMPT "READY"
#define STR_SYNTAX     "SYNTAX"
#define STR_SEMANTIC   "SEMANTIC"

#endif // STRINGS_EN_HPP
