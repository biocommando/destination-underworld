#ifndef iniRead_h
#define iniRead_h
void ini_read_string_value(FILE *file, const char *segment, const char *key, char *value);
int ini_read_int_value(FILE *file, const char *segment, const char *key);
double ini_read_double_value(FILE *file, const char *segment, const char *key);

typedef struct {
	int size;
	int array[256];
} FixedSizeArray;
FixedSizeArray ini_read_int_array_value(FILE *file, const char *segment, const char *key);
#endif