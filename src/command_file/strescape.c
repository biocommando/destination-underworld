#include "strescape.h"
#include <stdlib.h>
#include <string.h>

static char escaped(const char *escape_char_map, unsigned sz, char data)
{
    for (unsigned i = 0; i < sz; i += 2)
    {
        if (data == escape_char_map[i])
            return escape_char_map[i + 1];
    }
    return data;
}

void strescape_inplace(char *data, char escape_char, const char *escape_char_map, unsigned sz)
{
    for (size_t i = 0; data[i]; i++)
    {
        if (data[i] == escape_char)
        {
            data[i] = escaped(escape_char_map, sz, data[i + 1]);
            for (size_t j = i + 1; data[j]; j++)
            {
                data[j] = data[j + 1];
            }
        } 
    }
}

char *strescape(const char *data, char escape_char, const char *escape_char_map, unsigned sz)
{
    size_t res_sz = strlen(data) + 1;
    char *result = malloc(res_sz);
    memcpy(result, data, res_sz);
    strescape_inplace(result, escape_char, escape_char_map, sz);
    return result;
}
