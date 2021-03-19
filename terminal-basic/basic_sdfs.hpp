/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016-2019 Andrey V. Skvortsov <starling13@mail.ru>
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

#ifndef BASIC_SDFS_HPP
#define BASIC_SDFS_HPP

#include "basic_functionblock.hpp"
#include "basic_interpreter.hpp"

#if USESD

#include <SD.h>
#include <sd.hpp>

namespace BASIC
{
/**
 * @brief Module with commands to store the programs on SD card
 */
class SDFSModule : public FunctionBlock
{
	// Function block interface
public:
	SDFSModule();
	
	void loadAutorun(Interpreter&);
	
protected:
	void _init() override;
private:
	static bool dchain(Interpreter&);
	static bool dsave(Interpreter&);
	static bool directory(Interpreter&);
	static bool scratch(Interpreter&);
	static bool dload(Interpreter&);
	static bool header(Interpreter&);
	
	static bool getFileName(Interpreter&, char[]);
	static bool _loadText(SDCard::File&, Interpreter&);
	
	static SDCard::File	_root;
	static const FunctionBlock::function _commands[] PROGMEM;
};

} // namespace BASIC

#endif // USESD

#endif // BASIC_SDFS_HPP
