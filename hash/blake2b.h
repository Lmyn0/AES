#include <stdint.h>
#include <stddef.h>

extern unsigned char output[64];

void blake2b(unsigned char *message, size_t mlen, size_t nn);