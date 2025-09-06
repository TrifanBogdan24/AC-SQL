#pragma once
#include <stdio.h>
#include <stdlib.h>

static inline void DIE(int condition, char *error_message) {
    if (!condition)
        return;
    fprintf(stderr, "%s", error_message);
    exit(EXIT_FAILURE);
}
