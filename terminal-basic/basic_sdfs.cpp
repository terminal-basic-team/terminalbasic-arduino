/*
 * Terminal-BASIC is a lightweight BASIC-like language interpreter
 * 
 * Copyright (C) 2016-2018 Andrey V. Skvortsov <starling13@mail.ru>
 * Copyright (C) 2019,2020 Terminal-BASIC team
 *     <https://bitbucket.org/%7Bf50d6fee-8627-4ce4-848d-829168eedae5%7D/>
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

#if USE_FILEOP
static SDCard::File userFiles[FILE_NUMBER];
#endif // USE_FILEOP

SDCard::File SDFSModule::_root;

static const uint8_t sdfsCommands[] PROGMEM = {
	'D', 'C', 'H', 'A', 'I', 'N', ASCII_NUL,
	'D', 'I', 'R', 'E', 'C', 'T', 'O', 'R', 'Y', ASCII_NUL,
	'D', 'L', 'O', 'A', 'D', ASCII_NUL,
	'D', 'S', 'A', 'V', 'E', ASCII_NUL,
#if USE_FILEOP
	'F', 'C', 'L', 'O', 'S', 'E', ASCII_NUL,
	'F', 'S', 'E', 'E', 'K', ASCII_NUL,
	'F', 'W', 'R', 'I', 'T', 'E', ASCII_NUL,
#endif
	'H', 'E', 'A', 'D', 'E', 'R', ASCII_NUL,
	'S', 'C', 'R', 'A', 'T', 'C', 'H', ASCII_NUL,
	ASCII_ETX
};

#if USE_FILEOP
static const uint8_t sdfsFunctions[] PROGMEM = {
	'F', 'O', 'P', 'E', 'N', ASCII_NUL,
	'F', 'R', 'E', 'A', 'D', ASCII_NUL,
	'F', 'S', 'I', 'Z', 'E', ASCII_NUL,
	ASCII_ETX
};

#endif // USE_FILEOP

const FunctionBlock::function  SDFSModule::_commands[] PROGMEM = {
	SDFSModule::dchain,
	SDFSModule::directory,
	SDFSModule::dload,
	SDFSModule::dsave,
#if USE_FILEOP
	SDFSModule::com_fclose,
	SDFSModule::com_fseek,
	SDFSModule::com_fwrite,
#endif
	SDFSModule::header,
	SDFSModule::scratch
};

#if USE_FILEOP
const FunctionBlock::function SDFSModule::_functions[] PROGMEM = {
	SDFSModule::func_fopen,
	SDFSModule::func_fread,
	SDFSModule::func_fsize
};
#endif // USE_FILEOP

SDFSModule::SDFSModule()
{
	commands = _commands;
	commandTokens = sdfsCommands;
#if USE_FILEOP
	functions = _functions;
	functionTokens = sdfsFunctions;
#endif
}

void
SDFSModule::loadAutorun(Interpreter& i)
{
	static const char ar[] PROGMEM = "/AUTORUN.BAS";
	char ss[13];
	strcpy_P(ss, ar);
	SDCard::File f = SDCard::SDFS.open(ss);
	if (!f)
		return;
	
	if (!_loadText(f, i))
		return;
	
	i.run();
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

#if USE_FILEOP

bool
SDFSModule::com_fclose(Interpreter& i)
{
	INT iv;
	if (getIntegerFromStack(i, iv)) {
		if (iv >= 0 && iv < FILE_NUMBER) {
			if (userFiles[iv]) {
				userFiles[iv].close();
				return true;
			}
		}
	}
	return false;
}

bool
SDFSModule::com_fseek(Interpreter& i)
{
	INT iv;
	if (getIntegerFromStack(i, iv)) {
		if (iv >= 0 && iv < FILE_NUMBER) {
			if (userFiles[iv]) {
				INT bv;
				if (getIntegerFromStack(i, bv)) {
					return userFiles[iv].seek(bv);
				}
			}
		}
	}
	return false;
}

bool
SDFSModule::com_fwrite(Interpreter& i)
{
	INT iv;
	if (getIntegerFromStack(i, iv)) {
		if (iv >= 0 && iv < FILE_NUMBER) {
			if (userFiles[iv]) {
				INT bv;
				if (getIntegerFromStack(i, bv))
					return (userFiles[iv].write(bv) == 1);
			}
		}
	}
	return false;
}

bool
SDFSModule::func_fopen(Interpreter& i)
{
	uint8_t currentFile;
	for (currentFile = 0; currentFile<FILE_NUMBER; ++currentFile)
		if (!userFiles[currentFile])
			break;
	if (currentFile < FILE_NUMBER) {
		const char* s;
		if (i.popString(s)) {
			userFiles[currentFile] = SDCard::SDFS.open(s,
			    SDCard::Mode::WRITE |
			    SDCard::Mode::READ |
			    SDCard::Mode::CREAT);
			if (userFiles[currentFile]) {
				userFiles[currentFile].seek(0);
				if (i.pushValue(Integer(currentFile))) {
					return true;
				}
				// Can't push fileno, close file
				userFiles[currentFile].close();
			}
		}
	}
	return false;
}

bool
SDFSModule::func_fread(Interpreter& i)
{
	INT iv;
	if (getIntegerFromStack(i, iv)) {
		if (iv >= 0 && iv < FILE_NUMBER) {
			if (userFiles[iv]) {
				if (i.pushValue(INT(userFiles[iv].read())))
					return true;
			}
		}
	}
	return false;
}

bool
SDFSModule::func_fsize(Interpreter& i)
{
	INT iv;
	if (getIntegerFromStack(i, iv)) {
		if (iv >= 0 && iv < FILE_NUMBER) {
			if (userFiles[iv]) {
				if (i.pushValue(INT(userFiles[iv].size())))
					return true;
			}
		}
	}
	return false;
}

#endif // USE_FILEOP

bool
SDFSModule::directory(Interpreter &i)
{
	static const char str[] PROGMEM = "SD CARD CONTENTS";
	
	uint16_t startFile = 0;
	uint16_t endFile = 65535;
	INT iv;
	if (getIntegerFromStack(i, iv)) {
		startFile = iv;
		if (getIntegerFromStack(i, iv)) {
			endFile = startFile;
			startFile = iv;
		}
	}

	_root.rewindDirectory();

	char buf[17];
	strcpy_P(buf, (PGM_P)str);
	i.print(buf);
	i.newline();
	Integer index = 0;
	for (SDCard::File ff = _root.openNextFile(); ff; ff = _root.openNextFile()) {
		++index;
		if (index < startFile) {
			ff.close();
			continue;
		}
		if (index > endFile) {
			ff.close();
			break;
		}
		i.print(index);
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
		f.print(READ_VALUE(s->number));
		lex.init(s->text, true);
		Token tPrev = Token::NOTOKENS;
		while (lex.getNext()) {
			Token t = lex.getToken();
			if (t != Token::COMMA &&
			    t != Token::RPAREN &&
			    tPrev != Token::LPAREN)
			    f.write(' ');
			tPrev = t;
			if (t < Token::INTEGER_IDENT) {
				uint8_t buf[16];
				const bool res = Lexer::getTokenString(t,
				    reinterpret_cast<uint8_t*>(buf));
				if (res)
					f.print((const char*)buf);
				else {
					f.close();
					return false;
				}
				if (t == Token::KW_REM) {
					f.write(' ');
					f.print((const char*)s->text +
					    lex.getPointer());
					break;
				}
			} else if (t < Token::C_INTEGER) {
				f.print(lex.id());
			} else if (t < Token::C_STRING) {
				lex.getValue().printTo(f);
			} else if (t == Token::C_STRING) {
				f.write('"');
				f.print(lex.id());
				f.write('"');
			}
#if FAST_MODULE_CALL
			else if (t == Token::COMMAND) {
				uint8_t buf[16];
				FunctionBlock::command com =
				    reinterpret_cast<FunctionBlock::command>(
				    readValue<uintptr_t>((const uint8_t*)lex.id()));
				i.parser().getCommandName(com, buf);
				f.print((const char*)buf);
			}
#endif
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
		// f.setTimeout(10);
		size_t res = 0;
		int c;
		while (f.available() > 0) {
			c = f.read();
			if (c == '\r') {
			    continue;
			} else if (c == '\n') {
			    buf[res] = '\0';
			    break;
			} else if (res < (PROGSTRINGSIZE-1))
				buf[res++] = c;
		}
		if (res > 0) {
                	if (buf[res-1] == '\r')
                        	buf[res-1] = 0;
			Lexer lex;
			lex.init((uint8_t*)buf, false);
			if (!lex.getNext() || lex.getToken() !=
			    Token::C_INTEGER)
				return false;
			if (!i._program.addLine(i.parser(), Integer(lex.getValue()),
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
