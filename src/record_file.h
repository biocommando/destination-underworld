#pragma once

#include <stdio.h>

// Writes any pending changes to the file and frees all associated memory.
void record_file_flush();

// Reads a "record" from a file which means
// it reads a line from the file that starts with the given id.
// Reads the whole file on first access to the file and does not
// do any disk operations before calling one of the get/set functions with
// a different filename.
int record_file_get_record(const char *file, const char *id, char *record, size_t sz);

// Sets a "record" in a file which means
// it writes a line to the file that starts with the given id.
// Writes the new data to the file only when one of the get/set functions is called with
// a different filename or the file is flushed.
int record_file_set_record(const char *file, const char *id, const char *record);
