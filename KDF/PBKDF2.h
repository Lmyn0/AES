#include <stddef.h>
#include <stdint.h>

void PBKDF2(unsigned char *password, size_t plen, unsigned char *salt, size_t slen, uint32_t c, uint32_t dklen, unsigned char *DK);