#include <stdlib.h>
#include <string.h>

char *escape_JSON(const char *input) {
    size_t len = strlen(input);
    char *output = malloc(len * 2 + 1);
    char *pOutput = output;
    for (size_t i = 0; i < len; ++i) {
        if (input[i] == '\"') { *pOutput++ = '\\'; *pOutput++ = '\"'; }
        else if (input[i] == '\n') { *pOutput++ = '\\'; *pOutput++ = 'n'; }
        else if (input[i] == '\\') { *pOutput++ = '\\'; *pOutput++ = '\\'; }
        else { *pOutput++ = input[i]; }
    }
    *pOutput = '\0';
    return output;
}

// size_t is a data type in C and C++ used to represent the size of objects in bytes.
// It is an unsigned integer type, meaning it can only represent non-negative values, and its size can vary depending on the architecture of the system.
// - wikipedia