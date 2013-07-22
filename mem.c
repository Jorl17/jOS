#include <mem.h>

void memcpy(void* dst, const void* src, uint32_t len)
{
        uint8_t* dst_ = (uint8_t*)dst;
        uint8_t* src_ = (uint8_t*)src;
                
        while ( len-- ) *dst_++ = *src_++;
}
void memset(void* dst, uint8_t val, uint32_t len)
{
        uint8_t* dst_ = (uint8_t*)dst;
        while (len--) *dst_++ = val;
}
