#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int errnum;
    const char *message;
} error_t;

error_t error_nil = {0};

error_t error_make(const int errnum, const char *message);
bool error_not_nil(const error_t error);

void error_assert(bool condition, const char *format, ...);
void error_panic_on(const error_t error, const char *format, ...);

#endif // ERROR_H

#ifdef ERROR_IMPLEMENTATION

error_t error_make(const int errnum, const char *message) {
    return (error_t){ .errnum = errnum, .message = message };
}

bool error_not_nil(const error_t error) {
    return error.errnum != 0 || error.message != NULL;
}

void error_assert(bool condition, const char *format, ...) {
    if (condition) return;

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(1);
}

void error_panic_on(const error_t error, const char *format, ...) {
    if (!error_not_nil(error)) return;

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    if (error.errnum != 0) fprintf(stderr, " %s", strerror(error.errnum));
    if (error.message != NULL) fprintf(stderr, " %s", error.message);

    fprintf(stderr, "\n");
}

#endif // ERROR_IMPLEMENTATION