# Custom memmove

This is a simple implementation of memmove, but without std::memmove.

## Why?

On ESP32, the std::memmove might allocate dynamic RAM, which is not always good and might produce issues. This implementation is stack-only.

## How?

You can either use it as it is, or experiment with optimization levels as these impact performance and might even let the naive implementation perform better than the advanced (normal) custom implementation.

## Performance?

Tested on an Apple Silicon Mac and clang:

Optimization -O1, -Og

    Overall runtime std: 0.0664123 s
    Overall runtime custom: 0.094024 s
    Overall runtime naive: 0.254656 s

Optimization -O2, -O3, -Os

    Overall runtime std: 0.0651672 s
    Overall runtime custom: 0.0908536 s
    Overall runtime naive: 0.0706607 s

Optimization -O0

    Overall runtime std: 0.0982345 s
    Overall runtime custom: 0.145582 s
    Overall runtime naive: 0.661143 s

## GCC?

You can try gcc too, but then you need to add `-lstdc++` to the compiler args. However, gcc is almost the same, but the O2 results are like the O1 results compared to clang.
