#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

#include "custom_memmove.hpp"

static bool _testDestSmaller(const std::size_t chunkSize, const std::size_t n);
static bool _testSrcSmaller(const std::size_t chunkSize, const std::size_t n);
static bool _testEqual(const std::size_t chunkSize, const std::size_t n);
static bool _testNoOverlap(const std::size_t chunkSize, const std::size_t n);

static uint64_t _overallRuntimeStd = 0;
static uint64_t _overallRuntimeCustom = 0;
static uint64_t _overallRuntimeNaive = 0;

static void* std_memmove_func(void* dest, const void* src, std::size_t n) {
    const auto start = std::chrono::high_resolution_clock::now();
    auto returnVal = std::memmove(dest, src, n);
    const auto end = std::chrono::high_resolution_clock::now();

    _overallRuntimeStd += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    return returnVal;
}

static void* custom_memmove_func(void* dest, const void* src, std::size_t n) {
    const auto start = std::chrono::high_resolution_clock::now();
    auto returnVal = custom::memmove(dest, src, n);
    const auto end = std::chrono::high_resolution_clock::now();

    _overallRuntimeCustom +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    return returnVal;
}

static void* custom_memmove_naive_func(void* dest, const void* src, std::size_t n) {
    const auto start = std::chrono::high_resolution_clock::now();
    auto returnVal = custom::memmove_naive(dest, src, n);
    const auto end = std::chrono::high_resolution_clock::now();

    _overallRuntimeNaive +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    return returnVal;
}

/**
 * @brief Does some test cases and compares if it behaves like std::memmove.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(const int argc, char const* const argv[]) {
    std::cout << "Args: " << argc << std::endl;

    for (int i = 0; i < argc; ++i) {
        std::cout << "Arg " << i << ": " << argv[i] << std::endl;
    }

    std::size_t chunkSizeBegin = 1;
    std::size_t chunkSizeEnd = 1000;

    std::size_t nBegin = 1;
    std::size_t nEnd = 1000;

#define RUN_TEST(test, name, chunkSize, n)                                                       \
    if (!test(chunkSize, n)) {                                                                   \
        std::cout << "[test " << name << "] failed at chunkSize " << chunkSize << " and n " << n \
                  << std::endl;                                                                  \
        return 1;                                                                                \
    }

    for (std::size_t chunkSize = chunkSizeBegin; chunkSize <= chunkSizeEnd; ++chunkSize) {
        for (std::size_t n = nBegin; n <= nEnd; ++n) {
            RUN_TEST(_testDestSmaller, "destSmaller", chunkSize, n);
            RUN_TEST(_testSrcSmaller, "srcSmaller", chunkSize, n);
            RUN_TEST(_testEqual, "equal", chunkSize, n);
            RUN_TEST(_testNoOverlap, "noOverlap", chunkSize, n);
        }
    }

    // Print the overall runtime
    const float secondsStd = _overallRuntimeStd / 1e9;
    const float secondsCustom = _overallRuntimeCustom / 1e9;
    const float secondsNaive = _overallRuntimeNaive / 1e9;

    std::cout << "Overall runtime std: " << secondsStd << " s" << std::endl;
    std::cout << "Overall runtime custom: " << secondsCustom << " s" << std::endl;
    std::cout << "Overall runtime naive: " << secondsNaive << " s" << std::endl;

    return 0;
}

struct TestReturn {
    uint8_t* memory;
    std::size_t memorySize;
};

static bool _runTest(const std::size_t chunkSize, const std::size_t n,
                     TestReturn (*testFunction)(const std::size_t, const std::size_t,
                                                void* (*memmove_func)(void*, const void*,
                                                                      std::size_t))) {
    const TestReturn stdMemmove = testFunction(chunkSize, n, std_memmove_func);
    const TestReturn customMemmove = testFunction(chunkSize, n, custom_memmove_func);
    const TestReturn customMemmoveNaive = testFunction(chunkSize, n, custom_memmove_naive_func);

    std::vector<TestReturn> testReturns = {stdMemmove, customMemmove, customMemmoveNaive};

    // Check if the memory is valid
    std::size_t expectedSize = stdMemmove.memorySize;
    for (const TestReturn& testReturn : testReturns) {
        if (testReturn.memory == nullptr || testReturn.memorySize != expectedSize ||
            testReturn.memorySize == 0) {
            // Clean up memory of others
            for (const TestReturn& testReturn : testReturns) {
                if (testReturn.memory != nullptr) delete[] testReturn.memory;
            }

            return false;
        }
    }

    // Compare the memory
    bool equal = true;
    for (unsigned int i = 1; i < testReturns.size(); ++i) {
        if (std::memcmp(testReturns.front().memory, testReturns[i].memory,
                        testReturns.front().memorySize) != 0) {
            equal = false;
            break;
        }
    }

    // Clean up memory
    for (const TestReturn& testReturn : testReturns) {
        delete[] testReturn.memory;
    }

    return equal;
}

static TestReturn _runWithDestSmaller(const std::size_t chunkSize, const std::size_t n,
                                      void* (*memmove_func)(void*, const void*, std::size_t)) {
    uint8_t* const memory = new uint8_t[n + chunkSize];
    if (memory == nullptr) return {nullptr, 0};

    uint8_t* const dest = memory;
    uint8_t* const src = memory + chunkSize;

    // Set the memory
    for (std::size_t i = 0; i < n + chunkSize; ++i) {
        memory[i] = i % 256;
    }

    memmove_func(dest, src, n);

    return {memory, n + chunkSize};
}

static bool _testDestSmaller(const std::size_t chunkSize, const std::size_t n) {
    return _runTest(chunkSize, n, _runWithDestSmaller);
}

static TestReturn _runWithSrcSmaller(const std::size_t chunkSize, const std::size_t n,
                                     void* (*memmove_func)(void*, const void*, std::size_t)) {
    uint8_t* const memory = new uint8_t[n + chunkSize];
    if (memory == nullptr) return {nullptr, 0};

    uint8_t* const dest = memory + chunkSize;
    uint8_t* const src = memory;

    // Set the memory
    for (std::size_t i = 0; i < n + chunkSize; ++i) {
        memory[i] = i % 256;
    }

    memmove_func(dest, src, n);

    return {memory, n + chunkSize};
}

static bool _testSrcSmaller(const std::size_t chunkSize, const std::size_t n) {
    return _runTest(chunkSize, n, _runWithSrcSmaller);
}

static TestReturn _runWithEqual(const std::size_t chunkSize, const std::size_t n,
                                void* (*memmove_func)(void*, const void*, std::size_t)) {
    (void)chunkSize;

    uint8_t* const memory = new uint8_t[n];
    if (memory == nullptr) return {nullptr, 0};

    uint8_t* const dest = memory;
    uint8_t* const src = memory;

    // Set the memory
    for (std::size_t i = 0; i < n; ++i) {
        memory[i] = i % 256;
    }

    memmove_func(dest, src, n);

    return {memory, n};
}

static bool _testEqual(const std::size_t chunkSize, const std::size_t n) {
    return _runTest(chunkSize, n, _runWithEqual);
}

static TestReturn _runWithNoOverlap(const std::size_t chunkSize, const std::size_t n,
                                    void* (*memmove_func)(void*, const void*, std::size_t)) {
    (void)chunkSize;

    // Adding some spacing of 4 bytes
    uint8_t* const memory = new uint8_t[(n * 2) + 4];
    if (memory == nullptr) {
        std::cout << "Memory allocation failed" << std::endl;
        return {nullptr, 0};
    }

    uint8_t* const dest = memory;
    uint8_t* const src = memory + n + 4;

    // Set the memory
    for (std::size_t i = 0; i < (n * 2) + 4; ++i) {
        memory[i] = i % 256;
    }

    memmove_func(dest, src, n);

    return {memory, (n * 2) + 4};
}

static bool _testNoOverlap(const std::size_t chunkSize, const std::size_t n) {
    return _runTest(chunkSize, n, _runWithNoOverlap);
}
