#include <stdio.h>
#include <stdlib.h>

void ffiprintf_str(unsigned char *c, long clen, unsigned char *a, long alen) {
    printf("%s", (char *)clen);
    fflush(stdout);
}

void ffiprintf_int(unsigned char *c, long clen, unsigned char *a, long alen) {
    printf("%ld", clen);
    fflush(stdout);
}