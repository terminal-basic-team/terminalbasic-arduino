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

#ifndef LEXER_HPP
#define LEXER_HPP

#include <inttypes.h>
#include <stdlib.h>

#include "arduino_logger.hpp"
#include "basic_parser.hpp"
#include "basic_parser_value.hpp"
#include "helper.hpp"

namespace BASIC
{

#if ARDUINO_LOG
Logger&
operator<<(Logger &logger, Token tok);
#endif

/**
 * @brief Lexical analyzer class
 * @param 
 */
class Lexer
{
public:
	/**
	 * @brief initialize lexer session
	 * @param str string to extract tokens from
	 */
	void init(const char*);
	/**
	 * @brief continue lexical analyze for next token
	 * @return string end flag
	 */
	bool getNext();
	/**
	 * @brief get last extracted token
	 * @return last token
	 */
	Token getToken() const
	{
		return _token;
	}
	/**
	 * @brief get current value (numberm boolean...)
	 * @return value, extracted from string
	 */
	const Parser::Value &getValue() const { return _value; }
	/**
	 * @brief get current string (identifier)
	 * @return identifier string
	 */
	const char *id() const { return _id; }
	/**
	 * @brief get analised string position
	 * @return string position index
	 */
	size_t getPointer() const { return _pointer; }
	/**
	 * @brief token strings array
	 */
	static PGM_P const tokenStrings[uint8_t(Token::NUM_TOKENS)];
private:

	void pushSYM();
	void next();

	void first_A();
	void first_C();
	void first_D();
	void first_E();
	void first_F();
	void first_G();
	void first_I();
	void first_L();
	void first_N();
	void first_O();
	void first_P();
	void first_R();
	void first_S();
	void first_T();
	void first_V();

	void fitst_LT();
	void fitst_GT();
	
	// Parse decimal number
	void decimalNumber();
	// Parse Binary number
	void binaryInteger();
#if USE_REALS
	bool numberScale();
#endif
	void ident();
	void stringConst();
	
	// analyzed string
	const char *_string;
	// analysed string position
	uint8_t _pointer;
	// current value
	Parser::Value _value;
	// current identifier string
	char _id[STRINGSIZE];
	// identifier string pointer
	uint8_t _valuePointer;
	Token _token;
};

}

#endif
