#ifndef BASIC_LEXER_H
#define BASIC_LEXER_H

#include "basic.h"
#include "basic_value.h"

__BEGIN_DECLS

/**
 * @brief lexical analyses stage errors
 */
typedef enum basic_lexer_error
{
	BASIC_LEXER_ERROR_NOERROR = 0,
	BASIC_LEXER_ERROR_STRING_OVERFLOW = 1
} basic_lexer_error_t;

typedef struct _basic_lexer_context_t
{
	const uint8_t *string_to_parse;
	/* position in the parsing string */
	uint8_t string_pointer;
	basic_token_t token;
	
	/* current identifier string */
	char _id[STRING_SIZE];
	/* Identifier string pointer */
	uint8_t _value_pointer;
	/* scanned numeric/logical value */
	basic_value_t value;
	/* scanner error */
	basic_lexer_error_t _error;
	
	BOOLEAN tokenized;
} basic_lexer_context_t;

/**
 * @brief Initialize lexer by the string
 * @param self lexer context
 * @param str ASCII or tokenized basic string
 * @param tokenized
 */
void basic_lexer_init(basic_lexer_context_t*, const uint8_t*, BOOLEAN);

/**
 * @brief Get next token from ASCII string
 * @param self lexer context
 * @return true if there are more symbols to scan
 */
BOOLEAN basic_lexer_getnextPlain(basic_lexer_context_t*);

/**
 * @brief Get next token from tokenized string
 * @param self lexer context
 * @return true if there are more symbols to scan
 */
BOOLEAN basic_lexer_getnextTokenized(basic_lexer_context_t*);

BOOLEAN basic_lexer_getNext(basic_lexer_context_t*);

/**
 * @brief Get token text representation to buffer
 * @param t token
 * @param buf char buffer of appropriate length 
 */
BOOLEAN basic_lexer_tokenString(basic_token_t, uint8_t*);

/**
 * @brief tokenize a string
 * @param self Lexer context
 * @param dst destination buffer
 * @param dstsize length of the dst buffer
 * @param src source buffer
 * @return size of the actually tokenized buffer
 */
uint8_t basic_lexer_tokenize(basic_lexer_context_t*, uint8_t*, uint8_t,
    const uint8_t*);

__END_DECLS

#endif /* BASIC_LEXER_H */

