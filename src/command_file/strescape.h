#pragma once

void strescape_inplace(char *data, char escape_char, const char *escape_char_map, unsigned sz);

char *strescape(const char *data, char escape_char, const char *escape_char_map, unsigned sz);