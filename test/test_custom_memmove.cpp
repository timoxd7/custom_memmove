#include <stdio.h>

#include "custom_memmove.hpp"

int main(int argc, char const *argv[]) {
    printf("Args: %i\n", argc);

    for (int i = 0; i < argc; ++i) {
        printf("Arg %i: %s\n", i, argv[i]);
    }

    printf("Hello, World!\n");

    return 0;
}
