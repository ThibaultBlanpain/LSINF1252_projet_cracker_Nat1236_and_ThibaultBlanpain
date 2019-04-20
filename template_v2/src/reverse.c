#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "sha256.h"
#include "reverse.h"

static bool __reversehash(const uint8_t *hash, char *tmp, unsigned int depth,
			  unsigned int idx, uint8_t *hashres)
{
	unsigned char c;

	for (c = 'a'; c <= 'z'; c++) {
		tmp[idx] = c;
		tmp[idx+1] = 0;

		if (idx + 1 == depth) {
			sha256_buffer(tmp, idx + 1, hashres);
			if (memcmp(hash, hashres, SHA256_DIGEST_SIZE) == 0)
				return true;
		} else {
			if (__reversehash(hash, tmp, depth, idx + 1, hashres))
				return true;
		}
	}

	return false;
}

bool reversehash(const uint8_t *hash, char *res, size_t len)
{
	uint8_t hashres[SHA256_DIGEST_SIZE];
	unsigned int depth;
	bool found;
	char tmp[len+1];

	for (depth = 1; depth <= len; depth++) {
		found = __reversehash(hash, tmp, depth, 0, hashres);
		if (found)
			break;
	}

	if (found)
		strncpy(res, tmp, len);

	return found;
}
