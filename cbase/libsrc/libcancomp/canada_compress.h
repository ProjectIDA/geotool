#ifndef _CANADA_COMPRESS_H
#define _CANADA_COMPRESS_H

#include <arpa/inet.h>

#define CANCOMP_ERR	-1  /* unrecoverable error (malloc fails) */
#define CANCOMP_SUCCESS	 0  /* success */
#define CANCOMP_NOT_20   1  /* number of samples not divisible by 20 */
#define CANCOMP_CORRUPT	 2  /* corrupted call */
#define CANCOMP_EXCEED	 3  /* number of bytes available in compressed data
                             * exceeded during decompression */

int canada_uncompress(unsigned char *b, uint32_t *y, int *n, int m,
                      uint32_t *v0);
int canada_compress(unsigned char *b, uint32_t *y, int *n, int m,
                    uint32_t *v0);

#endif /* ! _CANADA_COMPRESS_H */
