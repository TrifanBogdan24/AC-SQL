#pragma once
#include <string.h>
#include <ctype.h>

/* Inlatura (in-place) spatiile de la in inceput si final de string */
static inline void trim(char *str)
{
    if (!str)
        return;

    // elimina spatiile de la inceput
    while (*str && isspace((unsigned char)*str))
        memmove(str, str + 1, strlen(str));

    // elimina spatiile de la final
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

