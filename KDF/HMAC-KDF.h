#include <stddef.h>

void HMAC(unsigned char *HK, size_t HK_length, unsigned char *text, size_t text_length, unsigned char output_sha[32]);
