#pragma once

// Writes any pending changes to the file and frees all associated memory.
void record_file_flush();

// Finds a record in the file. The file is in command file format where
// the record id is the command and it can have any number of parameters.
// This function setups the internal state to modify or create the record.
// The record parameters are cleared if found.
int record_file_find_and_modify(const char *file, const char *id);

// Finds a record in the file. The file is in command file format where
// the record id is the command and it can have any number of parameters.
// This function setups the internal state to read the record.
int record_file_find_and_read(const char *file, const char *id);

// Adds a parameter to the record. Must be in "modify" mode.
int record_file_add_param(const char *param);

// Adds a parameter to the record that is converted from int to string.
// Must be in "modify" mode.
int record_file_add_int_param(int param);

// Adds a parameter to the record that is converted from float to string.
// Must be in "modify" mode.
int record_file_add_float_param(float param);

// Gets the next parameter or NULL if no more parameters.
// Must be in "read" mode.
const char *record_file_next_param();

// Gets the next parameter and tries to read int from it, returns err on failure.
// Must be in "read" mode.
int record_file_next_param_as_int(int err);

// Gets the next parameter and tries to read float from it, returns err on failure.
// Must be in "read" mode.
float record_file_next_param_as_float(float err);