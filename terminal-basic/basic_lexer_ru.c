#include "avr/pgmspace.h"
#include "config.h"
#include "basic.h"

#if CONF_LEXER_LANG == LANG_RU

const uint8_t _basic_lexer_symbolsShift PROGMEM = (uint8_t)('Ä')-'A';

const uint8_t _basic_lexer_tokenTable[] PROGMEM = {
	ASCII_NUL,
	'A', 'N', 'D',ASCII_NUL,                // 1
#if USE_DUMP
	'A', 'R', 'R', 'A', 'Y', 'S',ASCII_NUL, // 2
#endif
#if USE_SAVE_LOAD
	'C', 'H', 'A', 'I', 'N', ASCII_NUL,      // 4
#endif
#if USE_TEXTATTRIBUTES
	'C', 'L', 'S', ASCII_NUL,                // 5
#endif
#if USESTOPCONT
	'C', 'O', 'N', 'T', ASCII_NUL,           // 6
#endif
#if USE_MATRIX
	'C', 'O', 'N', ASCII_NUL,                // 7
#endif
#if USE_DATA
	'D', 'A', 'T', 'A', ASCII_NUL,           // 8
#endif
#if USE_DEFFN
	'D', 'E', 'F', ASCII_NUL,                // 9
#endif
#if USE_DELAY
	'D', 'E', 'L', 'A', 'Y', ASCII_NUL,      // 10
#endif
#if USE_MATRIX
	'D', 'E', 'T', ASCII_NUL,                // 11
#endif
	'D', 'I', 'M', ASCII_NUL,                // 12
#if USE_DIV_KW
	'D', 'I', 'V', ASCII_NUL,                // 13
#endif
#if USE_DOLOOP
	'D', 'O', ASCII_NUL,                     // 14
#endif
#if USE_DUMP
	'D', 'U', 'M', 'P', ASCII_NUL,           // 15
#endif
	'F', 'A', 'L', 'S', 'E', ASCII_NUL,
#if USE_DEFFN
	'F', 'N', ASCII_NUL,                     // 18
#endif
	'G', 'O', 'S', 'U', 'B', ASCII_NUL,      // 20
	'G', 'O', 'T', 'O', ASCII_NUL,           // 11
#if CONF_SEPARATE_GO_TO
	'G', 'O', ASCII_NUL,                     // 12
#endif
#if USE_MATRIX
	'I', 'D', 'N', ASCII_NUL,
	'I', 'N', 'V', ASCII_NUL,
#endif
	'L', 'I', 'S', 'T', ASCII_NUL,           // 19
#if USE_SAVE_LOAD
	'L', 'O', 'A', 'D', ASCII_NUL,           // 20
#endif
#if USE_TEXTATTRIBUTES
	'L', 'O', 'C', 'A', 'T', 'E', ASCII_NUL, // 21
#endif
#if USE_DOLOOP
	'L', 'O', 'O', 'P', ASCII_NUL,           // 22
#endif
#if USE_MATRIX
	'M', 'A', 'T', ASCII_NUL,                // 23
#endif
#if USE_INTEGER_DIV
	'M', 'O', 'D', ASCII_NUL,                // 24
#endif
	'N', 'E', 'W', ASCII_NUL,                // 21
	'N', 'O', 'T', ASCII_NUL,
	'O', 'N', ASCII_NUL,                     // 23
#if USE_RANDOM
	'R', 'A', 'N', 'D', 'O', 'M', 'I', 'Z', 'E', ASCII_NUL, //26
#endif
#if USE_DATA
	'R', 'E', 'A', 'D', ASCII_NUL,           // 27
#endif
	'R', 'E', 'M', ASCII_NUL,
#if USE_DATA
	'R', 'E', 'S', 'T', 'O', 'R', 'E', ASCII_NUL,
#endif
#if USE_SAVE_LOAD
	'S', 'A', 'V', 'E', ASCII_NUL,
#endif
#if CONF_USE_SPC_PRINT_COM
	'S', 'P', 'C', ASCII_NUL,
#endif
#if USE_TEXTATTRIBUTES
	'T', 'A', 'B', ASCII_NUL,
#endif
#if USE_MATRIX
	'T', 'R', 'N', ASCII_NUL,
#endif
	'T', 'R', 'U', 'E', ASCII_NUL,
#if USE_DUMP
	'V', 'A', 'R', 'S', ASCII_NUL,
#endif
	'X', 'O', 'R', ASCII_NUL,
#if USE_MATRIX
	'Z', 'E', 'R',ASCII_NUL,
#endif
	'Ç', 'Ç', 'é', 'Ñ', ASCII_NUL,
	'Ç', 'é', 'á', 'Ç', 'ê', 'Ä', 'í', ASCII_NUL,
	'Ç', 'ë', '', ASCII_NUL,
	'Ç', 'õ', 'Ç', 'é', 'Ñ', ASCII_NUL,
	'Ñ', 'ã', 'ü', ASCII_NUL,
	'Ñ', 'é', ASCII_NUL,
	'Ö', 'ë', 'ã', 'à', ASCII_NUL,
	'à', 'ã', 'à', ASCII_NUL,
	'ä', 'é', 'ç', 'Ö', 'ñ', ASCII_NUL,
	'è', 'ì', 'ë', 'ä', ASCII_NUL,
	'è', 'ì', 'ë', 'í', 'ú', ASCII_NUL,
#if USESTOPCONT
	'ë', 'í', 'é', 'è', ASCII_NUL,
#endif
	'í', 'é', ASCII_NUL,
	'ò', 'Ä', 'É', ASCII_NUL,
	ASCII_ETX
};

#endif  // CONF_LEXER_LANG
