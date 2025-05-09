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

// Similar to record_file_set_record but it formats the record within the call and reads the
// id from the resulting string.
int record_file_set_record_f(const char *file, const char *format, ...);

// Get the record with given id and does a sscanf to it (so the format will probably start like "%*s ...").
// Returns the number of variables scanned successfully.
int record_file_scanf(const char *file, const char *id, const char *format, ...);