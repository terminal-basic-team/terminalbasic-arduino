/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
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
	'D', 'C', 'H', 'A', 'I', 'N'+0x80,
	'D', 'I', 'R', 'E', 'C', 'T', 'O', 'R', 'Y'+0x80,
	'D', 'L', 'O', 'A', 'D'+0x80,
	'D', 'S', 'A', 'V', 'E'+0x80,
	'H', 'E', 'A', 'D', 'E', 'R'+0x80,
	'S', 'C', 'R', 'A', 'T', 'C', 'H'+0x80,
	0
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
		lex.init(s->text);
		while (lex.getNext()) {
			f.write(' ');
			Token t = lex.getToken();
			if (t < Token::STAR) {
				char buf[16];
				const uint8_t *res = Lexer::getTokenString(t,
				    reinterpret_cast<uint8_t*>(buf));
				if (res != nullptr)
					f.print(buf);
				else {
					f.close();
					return false;
				}
			} else if (t <= Token::RPAREN) {
				char buf[16];
				strcpy_P(buf, (PGM_P)pgm_read_ptr(
				    &(Lexer::tokenStrings[uint8_t(t)-
				    uint8_t(Token::STAR)])));
				f.print(buf);
				if (t == Token::KW_REM) {
					f.write(' ');
					f.print(s->text+lex.getPointer());
					break;
				}
			} else if (t <= Token::BOOL_IDENT) {
				f.print(lex.id());
			} else if (t <= Token::C_BOOLEAN) {
				f.print(lex.getValue());
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
			lex.init(buf);
			if (!lex.getNext() || lex.getToken() !=
			    Token::C_INTEGER)
				return false;
			if (!i._program.addLine(Integer(lex.getValue()),
			    buf+lex.getPointer()))
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
