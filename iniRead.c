#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iniRead.h"

void iniReadCharValue(FILE *file, char *segment, char *key, char *value)
{
	fseek(file, SEEK_SET, 0);
	char s[256] = "";
	int segmentFound = 0;
	while (!feof(file))
	{
		int i;
		if (s[0] == '#')
		{
            s[0] = 0;
            fgets(s, 256, file);
			continue;
        }
		char *xSegment = NULL, *xKey = s, *xValue = NULL;
		for (i = 0; i < 256; i++)
		{
			char c = s[i];
			if (c == 0)
				break;
			if (c == '[')
			{
				xSegment = &s[i + 1];
				segmentFound = 0;
			}
			else if (c == ']' && xSegment != NULL)
			{
				s[i] = 0;
				if (!strcmp(segment, xSegment))
				{
					segmentFound = 1;
				}
				break;
			}
			else if (xSegment != NULL || segmentFound)
			{
				if (c == '=')
				{
					s[i] = 0;
					if (!strcmp(key, xKey))
					{
						xValue = &s[i + 1];
					}
				}
				else if (c == '\n')
				{
					s[i] = 0;
				}
			}
		}
		if (xValue != NULL)
		{
			strcpy(value, xValue);
			break;
		}
		fgets(s, 256, file);
	}
}

int iniReadIntValue(FILE *file, char *segment, char *key)
{
	char value[256] = "";
	iniReadCharValue(file, segment, key, value);
	int iValue = 0;
	sscanf(value, "%d", &iValue);
	return iValue;
}

double iniReadDoubleValue(FILE *file, char *segment, char *key)
{
	char value[256] = "";
	iniReadCharValue(file, segment, key, value);
	int dValue = 0;
	sscanf(value, "%lf", &dValue);
	return dValue;
}
