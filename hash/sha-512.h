#ifndef SHA512_H
#define SHA512_H

#include <stddef.h>

void sha512(const unsigned char *message, size_t length, unsigned char digest[64]);

#endif