#include <stdio.h>

void ffiprint_char(unsigned char *c, long clen, unsigned char *a, long alen) {
    printf("%c", (char)(long)c);
    fflush(stdout);
}

void ffiprint_call_count(unsigned char *c, long clen, unsigned char *a, long alen) {
    printf("Pancake quicksort calls: %ld\n", (long)c);
    fflush(stdout);
}