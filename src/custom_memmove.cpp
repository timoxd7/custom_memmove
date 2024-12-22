#include "custom_memmove.hpp"

#include <stdint.h>

namespace custom {

#ifndef CONFIG_CUSTOM_MEMMOVE_FALLBACK_TO_NAIVE
static inline std::size_t _calcChunkSize(const void* const dest, const void* const src) {
    if (dest > src) return (uint8_t*)dest - (uint8_t*)src;
    return (uint8_t*)src - (uint8_t*)dest;
}
#endif

void* memmove(void* const d, const void* const s, std::size_t n) {
#ifdef CONFIG_CUSTOM_MEMMOVE_FALLBACK_TO_NAIVE
#warning "Using naive implementation of custom::memmove as with optimize this might be faster"
    return memmove_naive(d, s, n);
#else
    // Special case: no work to do
    if (d == nullptr || s == nullptr || n == 0 || d == s) return d;

    // Setting the pointers to uint8_t* to allow pointer arithmetic
    uint8_t* dest = (uint8_t*)d;
    const uint8_t* src = (const uint8_t*)s;

    // We calculate the distance between the source and destination pointers and then use this as
    // chunk size. Then we copy the chunks from the source to the destination, each at a time.
    std::size_t chunkSize = _calcChunkSize(dest, src);

    // Choose direction
    if (dest > src) {
        // Copy from the end to the beginning
        dest += n;
        src += n;
        while (n > 0) {
            if (n < chunkSize) chunkSize = n;

            dest -= chunkSize;
            src -= chunkSize;

            std::memcpy(dest, src, chunkSize);

            n -= chunkSize;
        }
    } else {
        // Copy from the beginning to the end
        while (n > 0) {
            if (n < chunkSize) chunkSize = n;
            std::memcpy(dest, src, chunkSize);

            dest += chunkSize;
            src += chunkSize;

            n -= chunkSize;
        }
    }

    return d;
#endif
}

void* memmove_naive(void* d, const void* s, std::size_t n) {
    // Special case: no work to do
    if (d == nullptr || s == nullptr || n == 0 || d == s) return d;

    // Setting the pointers to uint8_t* to allow pointer arithmetic
    uint8_t* dest = (uint8_t*)d;
    const uint8_t* src = (const uint8_t*)s;

    // Check for overlap
    if ((dest + n < src) || (src + n < dest)) {
        // No overlap, we can use std::memcpy
        std::memcpy(dest, src, n);
        return d;
    }

    // -> Overlap, we need to copy byte by byte
    if (dest < src) {
        // Overlapping, but dest is before src
        for (std::size_t i = 0; i < n; ++i) {
            dest[i] = src[i];
        }

    } else if (src < dest) {
        // Overlapping, but src is before dest
        for (std::size_t i = n; i > 0; --i) {
            dest[i - 1] = src[i - 1];
        }
    }

    return d;
}

}   // namespace custom
