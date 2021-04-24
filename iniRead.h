#ifndef iniRead_h
#define iniRead_h
void iniReadCharValue(FILE *file, char *segment, char *key, char *value);
int iniReadIntValue(FILE *file, char *segment, char *key);
double iniReadDoubleValue(FILE *file, char *segment, char *key);

#endif