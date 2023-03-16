#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iniRead.h"

void ini_read_string_value(FILE *file, const char *segment, const char *key, char *value)
{
	fseek(file, SEEK_SET, 0);
	char s[256] = "";
	int segment_found = 0;
	while (!feof(file))
	{
		int i;
		if (s[0] == '#')
		{
			s[0] = 0;
			fgets(s, 256, file);
			continue;
		}
		char *x_segment = NULL, *x_key = s, *x_value = NULL;
		for (i = 0; i < 256; i++)
		{
			char c = s[i];
			if (c == 0)
				break;
			if (c == '[')
			{
				x_segment = &s[i + 1];
				segment_found = 0;
			}
			else if (c == ']' && x_segment != NULL)
			{
				s[i] = 0;
				if (!strcmp(segment, x_segment))
				{
					segment_found = 1;
				}
				break;
			}
			else if (x_segment != NULL || segment_found)
			{
				if (c == '=')
				{
					s[i] = 0;
					if (!strcmp(key, x_key))
					{
						x_value = &s[i + 1];
					}
				}
				else if (c == '\n')
				{
					s[i] = 0;
				}
			}
		}
		if (x_value != NULL)
		{
			strcpy(value, x_value);
			break;
		}
		fgets(s, 256, file);
	}
}

int ini_read_int_value(FILE *file, const char *segment, const char *key)
{
	char value[256] = "";
	ini_read_string_value(file, segment, key, value);
	int i_value = 0;
	sscanf(value, "%d", &i_value);
	return i_value;
}

double ini_read_double_value(FILE *file, const char *segment, const char *key)
{
	char value[256] = "";
	ini_read_string_value(file, segment, key, value);
	double d_value = 0;
	sscanf(value, "%lf", &d_value);
	return d_value;
}

// ++
// function for reading an array of integers from ini file where every element of array is suffixed with the index starting from 0
// example: key0=1, key1=2, key2=3, key3=4, key4=5
// will be read as: {1, 2, 3, 4, 5}
// Returns FixedSizeArray struct containing the size of the array and the array itself
FixedSizeArray ini_read_int_array_value(FILE *file, const char *segment, const char *key)
{
	FixedSizeArray array = {0, {0}};
	int i = 0;
	while (i < 256)
	{
		char key_index[256] = "";
		sprintf(key_index, "%s%d", key, i);
		char value[256] = "";
		ini_read_string_value(file, segment, key_index, value);
		if (strlen(value) == 0)
			break;
		sscanf(value, "%d", &array.array[i]);
		i++;
	}
	array.size = i;
	return array;
}
