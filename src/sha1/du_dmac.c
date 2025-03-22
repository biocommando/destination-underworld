#include "du_dmac.h"
#include <stdio.h>
#include <string.h>
#include "sha1.h"
#include "../logging.h"

static int key_ctx = 0;

void convert_sha1_hash_to_hex(char *hex, const char *hash)
{
    for (int i = 0; i < DMAC_SHA1_HASH_SIZE; i++)
    {
        sprintf(hex + i * 2, "%02x", (unsigned char)hash[i]);
    }
    hex[DMAC_SHA1_HASH_SIZE * 2] = 0;
}

void convert_sha1_hex_to_hash(char *hash, const char *hex)
{
    for (int i = 0; i < DMAC_SHA1_HASH_SIZE; i++)
    {
        char b[3];
        b[0] = hex[i * 2];
        b[1] = hex[i * 2 + 1];
        b[2] = 0;
        unsigned u;
        sscanf(b, "%02x", &u);
        hash[i] = u;
    }
}

void dmac_sha1_default_key_fn(char *key, int ctx)
{
    memset(key, 0, 64);
    sprintf(key, "SecretKe%c", ctx);
}

void dmac_sha1_set_ctx(int ctx)
{
    key_ctx = ctx;
}

void dmac_sha1_calculate_hash(char *hash, const char *str, size_t size)
{
    char key[64];
    DMAC_SHA1_KEY_FN(key, key_ctx);

    char inner_key[64], outer_key[64];
    for (int i = 0; i < 64; i++)
    {
        inner_key[i] = key[i] ^ 0x36;
        outer_key[i] = key[i] ^ 0x5c;
    }
    char *buf = (char *)malloc(size + 64);
    memcpy(buf + 64, str, size);
    memcpy(buf, inner_key, 64);

    SHA1(hash, buf, size + 64);
    free(buf);
    buf = NULL;

    char buf2[64 + DMAC_SHA1_HASH_SIZE];
    memcpy(buf2 + 64, hash, DMAC_SHA1_HASH_SIZE);
    memcpy(buf2, outer_key, 64);
    SHA1(hash, buf2, 64 + DMAC_SHA1_HASH_SIZE);

}

void dmac_sha1_calculate_hash_f(char *hash, const char *fname)
{
	FILE *f = fopen(fname, "rb");
	fseek(f, 0, SEEK_END);
	size_t sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *contents = (char*)malloc(sz);
	fread(contents, 1, sz, f);
	fclose(f);
    dmac_sha1_calculate_hash(hash, contents, sz);
    free(contents);
}

int dmac_sha1_verify_string(const char *str, size_t size, const char *hash)
{
    char comp_hash[DMAC_SHA1_HASH_SIZE];

    dmac_sha1_calculate_hash(comp_hash, str, size);

    int err = strncmp(hash, comp_hash, DMAC_SHA1_HASH_SIZE);
    if (err)
    {
        char hex[DMAC_SHA1_HASH_SIZE * 2 + 1];
        convert_sha1_hash_to_hex(hex, comp_hash);
        LOG_TRACE("dmac_sha1_verify_string failed: err = %d\n", err);
        LOG_TRACE("dmac_sha1_verify_string failed: expected hash:    '%s'\n", hex);
        convert_sha1_hash_to_hex(hex, hash);
        LOG_TRACE("dmac_sha1_verify_string failed: verified against: '%s'\n", hex);
    }

    return err;
}

int dmac_sha1_verify_file(const char *fname, const char *hash)
{
    char comp_hash[DMAC_SHA1_HASH_SIZE];
    dmac_sha1_calculate_hash_f(comp_hash, fname);

    int err = strncmp(hash, comp_hash, DMAC_SHA1_HASH_SIZE);

    if (err)
    {
        char hex[DMAC_SHA1_HASH_SIZE * 2 + 1];
        convert_sha1_hash_to_hex(hex, comp_hash);
        LOG_TRACE("dmac_sha1_verify_file failed: err = %d\n", err);
        LOG_TRACE("dmac_sha1_verify_file failed: expected hash:    '%s'\n", hex);
        convert_sha1_hash_to_hex(hex, hash);
        LOG_TRACE("dmac_sha1_verify_file failed: verified against: '%s'\n", hex);
    }

    return err;
}