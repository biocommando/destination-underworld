#pragma once
#include <stdlib.h>

#ifndef DMAC_SHA1_KEY_FN
#define DMAC_SHA1_KEY_FN dmac_sha1_default_key_fn
#else
extern void DMAC_SHA1_KEY_FN(char *key);
#endif

#define DMAC_SHA1_HASH_SIZE 20

/*
 * Sets a default 64 byte DMAC-SHA1 key to the key pointer.
 */
void dmac_sha1_default_key_fn(char *key);

/*
 * Produces the 20 byte hash for the string.
 */
void dmac_sha1_calculate_hash(char *hash, const char *str, size_t size);

/*
 * Produces the 20 byte hash for the file.
 */
void dmac_sha1_calculate_hash_f(char *hash, const char *fname);

/*
 * Returns 0 if string produces the expected hash.
 */
int dmac_sha1_verify_string(const char *str, size_t size, const char *hash);

/*
 * Returns 0 if file produces the expected hash.
 */
int dmac_sha1_verify_file(const char *fname, const char *hash);

/*
 * Converts the binary hash to 0-terminated hex string.
 * Parameter hex needs to be at least DMAC_SHA1_HASH_SIZE * 2 + 1 bytes.
 */
void convert_sha1_hash_to_hex(char *hex, const char *hash);

/*
 * Similar to convert_sha1_hash_to_hex but to the other direction.
 */
void convert_sha1_hex_to_hash(char *hash, const char *hex);