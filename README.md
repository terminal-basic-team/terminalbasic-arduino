[![License GPLv3](https://img.shields.io/badge/license-GPL_v3-green.svg)](http://www.gnu.org/licenses/gpl-3.0.html)
[![Download Terminal-BASIC](https://img.shields.io/sourceforge/dt/terminal-basic.svg)](https://sourceforge.net/projects/terminal-basic/files/latest/download)

# Terminal-BASIC ![TB](https://a.fsdn.com/allura/p/terminal-basic/icon?1550272878)

Terminal-BASIC (**TB**) is a lightweight BASIC-like language interpreter.  Written in C++11 it is cross-platform, although
it was designed for small uC-based systems like Arduino with at least 32kb of program memory.
TB supports classic BASIC dialect (non-structured) with line numbering and interactive line editor.
It was inspired by the [TinyBASIC port](https://github.com/BleuLlama/TinyBasicPlus) for Arduino-compatible uc systems and
[Darthmouth BASIC](https://en.wikipedia.org/wiki/Dartmouth_BASIC) - the first one.

Resulting interpreter is partially compliant to the standards ISO/IEC 6373:1984 and USSR/Russian ГОСТ 27787-88 and
supports some non-standard features, allowing to use many type-in utilities and games from old BASIC books and magazines.

Some features of the TB interpreter:

- supports number of datatypes (integer (2 bytes signed), long integer (4 bytes signed), real (4 bytes binary floating point),
long real (8 bytes binary floating point), boolean and string) using variables and function suffixes;
- multidimensional arrays of arbitrary size and dimensions;
- Darthmouth-BASIC-like matrix operations;
- optional time-sharing system mode with round-robin scheduling using multiple I/O devices for each user (i.e. USART);
- configuration headers provide the number of options, which enable inclusion of the language parts and features thus allowing
to adjust the code size.

Sources of the TB and supporting libraries are hosted at [bitbucket.org](https://bitbucket.org/terminalbasicteam/)
with  [Github mirrors](https://github.com/users/starling13/projects/1). Main project page for downloads and support placed
at [SourceForge](https://sourceforge.net/projects/terminal-basic/). Project blog with news and discussion is hosted at
[hackaday.io](https://hackaday.io/project/22036-terminal-basic).

There are manual documents, describing the process of ackuiring different versions of TB, configuration and setup questions, language syntax
and developing extensions and TB itself.
