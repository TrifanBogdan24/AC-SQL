#pragma once
#include <string.h>
#include <ctype.h>

/* Inlatura (in-place) spatiile de la in inceput si final de string */
static inline void trim(char *str)
{
    if (!str)
        return;
    while (strlen(str) > 0 && isspace(str[0]))
        strcpy(str, str + 1);
    while (strlen(str) > 0 && isspace(str[strlen(str) - 1]))
        str[strlen(str) - 1] = '\0';
}

