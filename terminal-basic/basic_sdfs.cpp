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

#include "basic_sdfs.hpp"

#if USESD

#include "basic_program.hpp"
#include <assert.h>
#include <avr/pgmspace.h>

namespace BASIC
{

SDCard::File SDFSModule::_root;

static const uint8_t sdfsCommands[] PROGMEM = {
	'D', 'C', 'H', 'A', 'I', 'N', ASCII_NUL,
	'D', 'I', 'R', 'E', 'C', 'T', 'O', 'R', 'Y', ASCII_NUL,
	'D', 'L', 'O', 'A', 'D', ASCII_NUL,
	'D', 'S', 'A', 'V', 'E', ASCII_NUL,
	'H', 'E', 'A', 'D', 'E', 'R', ASCII_NUL,
	'S', 'C', 'R', 'A', 'T', 'C', 'H', ASCII_NUL,
	ASCII_ETX
};

const FunctionBlock::function  SDFSModule::_commands[] PROGMEM = {
	SDFSModule::dchain,
	SDFSModule::directory,
	SDFSModule::dload,
	SDFSModule::dsave,
	SDFSModule::header,
	SDFSModule::scratch
};

SDFSModule::SDFSModule()
{
	commands = _commands;
	commandTokens = sdfsCommands;
}

void
SDFSModule::_init()
{
	if (!SDCard::SDFS.begin())
		abort();
	
	_root = SDCard::SDFS.open("/", SDCard::Mode::WRITE | SDCard::Mode::READ |
	    SDCard::Mode::CREAT);
	if (!_root || !_root.isDirectory())
		abort();
}

bool
SDFSModule::directory(Interpreter &i)
{
	static const char str[] PROGMEM = "SD CARD CONTENTS";

	_root.rewindDirectory();

	char buf[17];
	strcpy_P(buf, (PGM_P)str);
	i.print(buf);
	i.newline();
	Integer index = 0;
	for (SDCard::File ff = _root.openNextFile(); ff; ff = _root.openNextFile()) {
		i.print(++index);
		i.print('\t');
		i.print(ff.name());
		uint8_t len = min((uint8_t(13)-strlen(ff.name())),
		    uint8_t(13));
		while (len-- > 0)
			i.print(' ');
		if (ff.isDirectory())
			i.print(ProgMemStrings::S_DIR);
		else
			i.print(Integer(ff.size()));
		i.newline();
		ff.close();
	}
	return true;
}

bool
SDFSModule::scratch(Interpreter &i)
{
	if (!i.confirm())
		return true;

	char ss[16];
	if (getFileName(i, ss)) {
		SDCard::SDFS.remove(ss);
		return true;
	} else
		return false;
}

bool
SDFSModule::dchain(Interpreter &i)
{
	char ss[16];
	if (!getFileName(i, ss))
		return false;
	
	SDCard::File f = SDCard::SDFS.open(ss);
	if (!f)
		return false;
	
	i._program.clearProg();
	i._program.moveData(0);
	i._program.jump(0);
	i.stop();
	return _loadText(f, i);
}

bool
SDFSModule::dsave(Interpreter &i)
{
	SDCard::File f;
	
	{ // Stack section 1
	char ss[16];
	if (getFileName(i, ss))
		SDCard::SDFS.remove(ss);
	f = SDCard::SDFS.open(ss, SDCard::Mode::WRITE |
	    SDCard::Mode::READ | SDCard::Mode::CREAT);
	} // Stack section 1
	if (!f)
		return false;
	
	i._program.reset();
	{ // Stack section 2
	Lexer lex;
	for (Program::Line *s = i._program.getNextLine(); s != nullptr;
	    s = i._program.getNextLine()) {
		f.print(s->number);
		lex.init(s->text, false);
		while (lex.getNext()) {
			f.write(' ');
			Token t = lex.getToken();
			if (t < Token::STAR) {
				uint8_t buf[16];
				const bool res = Lexer::getTokenString(t,
				    reinterpret_cast<uint8_t*>(buf));
				if (res)
					f.print((char*)buf);
				else {
					f.close();
					return false;
				}
			} else if (t <= Token::BOOL_IDENT) {
				f.print(lex.id());
			} else if (t <= Token::C_BOOLEAN) {
				lex.getValue().printTo(f);
			} else if (t == Token::C_STRING) {
				f.write('"');
				f.print(lex.id());
				f.write('"');
			}
		}
		f.print('\n');
	}
	} // Stack section 2
	f.close();
	return true;
}

bool
SDFSModule::_loadText(SDCard::File &f, Interpreter &i)
{
	while (true) {
		char buf[PROGSTRINGSIZE] = {0, };
		f.setTimeout(10);
		size_t res = f.readBytesUntil('\n', buf, PROGSTRINGSIZE-1);
		if (res > 0) {
                	if (buf[res-1] == '\r')
                        	buf[res-1] = 0;
			Lexer lex;
			lex.init((uint8_t*)buf, false);
			if (!lex.getNext() || lex.getToken() !=
			    Token::C_INTEGER)
				return false;
			if (!i._program.addLine(Integer(lex.getValue()),
			    (uint8_t*)buf+lex.getPointer()))
				return false;
		} else
			break;
	}
	
	f.close();
	i._program.reset();
	
	return true;
}

bool
SDFSModule::dload(Interpreter &i)
{
	char ss[16];
	if (!getFileName(i, ss))
		return false;
	
	SDCard::File f = SDCard::SDFS.open(ss);
	if (!f)
		return false;
	
	i._program.newProg();
	
	return _loadText(f, i);
}

bool
SDFSModule::header(Interpreter &i)
{
	if (!i.confirm())
		return true;

	char ss[16];
	_root.rewindDirectory();
	for (SDCard::File ff = _root.openNextFile(SDCard::Mode::WRITE |
	    SDCard::Mode::READ | SDCard::Mode::CREAT); ff;
	    ff = _root.openNextFile(SDCard::Mode::WRITE |
	    SDCard::Mode::READ | SDCard::Mode::CREAT)) {
		ss[0] = '/'; strcpy(ss+1, ff.name());
		ff.close();
		if (!SDCard::SDFS.remove(ss))
			return false;
	}
	return true;
}

bool
SDFSModule::getFileName(Interpreter &i, char ss[])
{
	static const char strBAS[] PROGMEM = ".BAS";
	
	const char *s;
	if (!i.popString(s))
		return false;
	ss[0] = '/';
	const uint8_t len = strlen(s);
	strcpy(ss + 1, s);
	strcpy_P(ss + len + 1, (PGM_P)strBAS);

	return SDCard::SDFS.exists(ss);
}

} // namespace BASIC

#endif // USESD
