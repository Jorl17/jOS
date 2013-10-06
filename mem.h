#ifndef MEM_H
#define MEM_H
#include <stdinc.h>
void memcpy(void* dst, const void* src, uint32_t len);
void memset(void* dst, uint8_t val, uint32_t len);
#endif
