#ifndef SV_H
#define SV_H

#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#include "error.h"
#include "io.h"

typedef struct {
    size_t count;
    const char *data;
} sv_t;

#define SV_CSTR(cstr) sv_make(cstr, strlen(cstr))
#define SV_STATIC(literal) { sizeof(literal)-1, (literal)}
#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int) (sv).count, (sv).data

typedef bool(*sv_predicate_t)(char c);

sv_t sv_make(const char *data, size_t count);
bool sv_equal(const sv_t a, const sv_t b);
bool sv_first_is(const sv_t sv, const sv_predicate_t predicate);
bool sv_has_prefix(const sv_t sv, const sv_t prefix);
sv_t sv_take_u64(const sv_t sv, uint64_t *u64);
sv_t sv_take_while(const sv_t sv, const sv_predicate_t predicate);
sv_t sv_drop_n(const sv_t sv, const size_t n);

error_t sv_from_file_path(const char *file_path, sv_t *sv);

static inline bool sv_isspace(char c);
static inline bool sv_isdigit(char c);

#endif // SV_H

#ifdef SV_IMPLEMENTATION

sv_t sv_make(const char *data, size_t count) {
    return (sv_t){ .data = data, .count = count };
}

bool sv_equal(const sv_t a, const sv_t b) {
    if (a.count != b.count)
        return false;

    for (size_t i = 0; i < a.count; i++)
        if (a.data[i] != b.data[i])
            return false;

    return true;
}

bool sv_first_is(const sv_t sv, const sv_predicate_t predicate) {
    if (sv.count < 1)
        return false;

    return predicate(sv.data[0]);
}

bool sv_has_prefix(const sv_t sv, const sv_t prefix) {
    if (prefix.count > sv.count)
        return false;

    return sv_equal(sv_make(sv.data, prefix.count), prefix);
}

sv_t sv_take_u64(const sv_t sv, uint64_t *u64) {
    *u64 = 0;

    size_t i = 0;
    for (; i < sv.count && isdigit(sv.data[i]); ++i)
        *u64 = *u64 * 10 + (uint64_t) sv.data[i] - '0';

    return sv_make(sv.data, i);
}


sv_t sv_take_while(const sv_t sv, const sv_predicate_t predicate) {
    size_t i = 0;
    while (i < sv.count && predicate(sv.data[i])) 
        i++;

    return sv_make(sv.data, i);
}

sv_t sv_drop_n(const sv_t sv, const size_t n) {
    if (sv.count < n) 
        return sv_make(NULL, 0);

    return sv_make(&sv.data[n], sv.count-n);
}

error_t sv_from_file_path(const char *file_path, sv_t *sv) {
    FILE *file = fopen(file_path, "r");
    if (file != NULL) error_make(errno, "could not open file");

    char *data;
    size_t data_count;
    error_t error = io_read_file(file, &data, &data_count);
    if (error_not_nil(error)) return error;

    *sv = sv_make(data, data_count);

    return error_nil;
}

static inline bool sv_isspace(char c) {
    return (bool)isspace((int)c);
}

static inline bool sv_isdigit(char c) {
    return (bool)isdigit((int)c);
}

#endif // SV_IMPLEMENTATION