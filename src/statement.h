#ifndef STATEMENT_H
#define STATEMENT_H

#include <inttypes.h>
#include <stdlib.h>

#include "ast.h"
#include "std/sv.h"

static_assert(AST_KIND_COUNT == 3, "[DEV] do you need to add a new kind in statement_kind_t enum?");
typedef enum {
    STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY = 0,
    STATEMENT_KIND_PRINT_INTEGER_ARRAY,
    STATEMENT_KIND_COUNT,
} statement_kind_t;

typedef struct {
    int64_t *free_ptr;
    int64_t *items;
    size_t count;
    size_t capacity;
    sv_t declarator;
    sv_t declarator_count;
} statement_global_declaration_integer_array_value_t;

typedef struct {
    sv_t variable;
    sv_t variable_count;
} statement_call_print_integer_array_value_t;

typedef union {
    statement_global_declaration_integer_array_value_t global_declaration_integer_array;
    statement_call_print_integer_array_value_t call_print_integer_array;
} statement_value_t;

typedef struct {
    statement_kind_t kind;
    statement_value_t value;
    token_position_t position;
} statement_t;

typedef struct {
    statement_t *free_ptr;
    statement_t *items;
    size_t count;
    size_t capacity;
} statements_t;

typedef struct {
    statements_t global_declarations;
    statements_t main_body;
} program_t;

typedef const char *(*statement_predicate_t)(const statement_t *statement);

const char* statement_is_printable(const statement_t *statement);

void statement_assert_predicate(const char *source_file_path, const statement_t *statement, statement_predicate_t predicate);

#endif // STATEMENT_H

#ifdef STATEMENT_IMPLEMENTATION

const char* statement_is_printable(const statement_t *statement) {
    static_assert(STATEMENT_KIND_COUNT == 2, "[DEV] is your new statement printable?");
    switch (statement->kind) {
        case STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY:
            return NULL;

        default: return "expected a printable sentence";
    }
}

void statement_assert_predicate(const char *source_file_path, const statement_t *statement, statement_predicate_t predicate) {
    const char *result = predicate(statement);
    error_assert(result == NULL, "%s:%zu:%zu %s",
        source_file_path,
        statement->position.row,
        statement->position.column,
        result
    );
}

#endif // STATEMENT_IMPLEMENTATION