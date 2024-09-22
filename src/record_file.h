#pragma once

#include <stdio.h>

void record_file_flush();

// Reads a "record" from a file which means
// it reads a line from the file that starts with the given id
int record_file_get_record(const char *file, const char *id, char *record, size_t sz);

// Sets a "record" in a file which means
// it writes a line to the file that starts with the given id
int record_file_set_record(const char *file, const char *id, const char *record);
