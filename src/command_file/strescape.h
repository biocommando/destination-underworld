#pragma once

/**
 * Escape given escape sequences in the string in-place.
 * The escape_char defines what character starts the escape sequence.
 * The escape_char_map is as map where even indices contain the original character
 * and odd indices the replacing character. sz is the byte size of the escape_char_map array.
 *
 * Example invocation:
 *
 *      char data[] = "hello _vorld";
 *      strescape_inplace(data, '_', "vw", 2);
 *      // data contains "hello world"
 */
void strescape_inplace(char *data, char escape_char, const char *escape_char_map, unsigned sz);

/**
 * Same as strescape_inplace but allocates a new buffer that is returned instead of modifying
 * the string in-place. Uses strescape_inplace internally.
 */
char *strescape(const char *data, char escape_char, const char *escape_char_map, unsigned sz);
