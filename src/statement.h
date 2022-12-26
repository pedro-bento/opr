#ifndef STATEMENT_H
#define STATEMENT_H

#include <inttypes.h>
#include <stdlib.h>

#include "ast.h"
#include "std/sv.h"

static_assert(AST_KIND_COUNT == 2, "[DEV] do you need to add a new kind in statement_kind_t enum?");
typedef enum {
    STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY = 0,
    STATEMENT_KIND_COUNT,
} statement_kind_t;

typedef struct {
    int64_t *free_ptr;
    int64_t *items;
    size_t count;
    size_t capacity;
    sv_t declarator;
} statement_global_declaration_integer_array_value_t;

typedef union {
    statement_global_declaration_integer_array_value_t global_declaration_integer_array;
} statement_value_t;

typedef struct {
    statement_kind_t kind;
    statement_value_t value;
} statement_t;

typedef struct {
    statement_t *free_ptr;
    statement_t *items;
    size_t count;
    size_t capacity;
} statements_t;

typedef struct {
    statements_t global_declarations;
} program_t;

#endif // STATEMENT_H

#ifdef STATEMENT_IMPLEMENTATION

#endif // STATEMENT_IMPLEMENTATION