#ifndef IO_H
#define IO_H

#include <stdlib.h>
#include <errno.h>

#include "error.h"

error_t io_file_count(FILE *file, size_t *count);
error_t io_file_read_count(FILE *file, char *content, const size_t content_count);
error_t io_read_file(FILE *file, char **content, size_t *content_count);

#endif // IO_H

#ifdef IO_IMPLEMENTATION

error_t io_file_count(FILE *file, size_t *count) {
    long saved_position = ftell(file);
    if (saved_position < 0) return error_make(errno, "could not save current position indicator");

    if (fseek(file, 0, SEEK_END) < 0) return error_make(errno, "could not move position indicator to end of file");

    long end_position = ftell(file);
    if (end_position < 0) return error_make(errno, "could not get position indicator at end of file");

    if (fseek(file, 0, saved_position) < 0) return error_make(errno, "could restore position indicator");

    *count = (size_t)end_position;

    return error_nil;
}

error_t io_file_read_count(FILE *file, char *content, const size_t content_count) {
    size_t n = fread(content, sizeof(char), content_count, file);
    if (n < content_count && !feof(file)) return error_make(ferror(file), "could not read file");

    return error_nil;
}

error_t io_read_file(FILE *file, char **content, size_t *content_count) {
    error_t error;

    error = io_file_count(file, content_count);
    if (error_not_nil(error)) return error;

    *content = malloc(*content_count);
    if (content == NULL) return error_make(errno, "could not allocate memory for file content");

    error = io_file_read_count(file, *content, *content_count);
    if (error_not_nil(error)) return error;

    return error_nil;
}

#endif // IO_IMPLEMENTATION