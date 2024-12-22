#include <cstring>

namespace custom {

/**
 * @brief Custom implementation of std::memmove to overcome DRAM issues and dynamic memory
 * allocation on some ESP devices. This implementation is full executed without the usage of
 * dynamic memory allocation.
 *
 * This function copies n bytes from memory area src to memory area dest. The memory areas may
 * overlap: copying takes place as though the bytes in src are first copied into a temporary
 * array that does not overlap src or dest, and the bytes are then copied from the temporary
 * array to dest. However, some optimizations are done and no temporary array is used at all.
 * Furthermore, the function relies on std::memcpy and chunks, to optimize the copying process.
 *
 * NOTE: This function gets slower as the memory areas overlap more, as more and smaller chunks will
 * be produced and result in more std::memcpy calls.
 *
 * NOTE: If you enabled optimizations, the compiler will most likely optimize the naive function as
 * good as or even better than this function. So only use this function if you really need to. You
 * can set the following define to always fall back to the naive implementation:
 *
 *     #define CONFIG_CUSTOM_MEMMOVE_FALLBACK_TO_NAIVE
 *
 * @param dest
 * @param src
 * @param n
 * @return void*
 */
void* memmove(void* dest, const void* src, std::size_t n);

/**
 * @brief Naive implementation of custom::memmove. Only use if you really need to, this is
 * essentially a for loop copying one byte at a time, which reduces performance significantly.
 *
 * @param dest
 * @param src
 * @param n
 * @return void*
 */
void* memmove_naive(void* dest, const void* src, std::size_t n);

}   // namespace custom
