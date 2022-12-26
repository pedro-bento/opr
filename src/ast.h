#ifndef AST_H
#define AST_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "token.h"
#include "std/error.h"

typedef struct ast_t ast_t;

static_assert(TOKEN_KIND_COUNT == 3, "[DEV] do you need to add a new kind in ast_kind_t enum?");
typedef enum {
    AST_KIND_INTEGER = 0,
    AST_KIND_RANGE,
    AST_KIND_PRINT,
    AST_KIND_COUNT,
} ast_kind_t;

typedef struct {
    ast_t *left;
    ast_t *right;
} ast_dyadic_value_t;

typedef struct {
    ast_t *right;
} ast_monadic_value_t;

typedef union {
    int64_t integer;
    ast_dyadic_value_t dyadic;
    ast_monadic_value_t monadic;
} ast_value_t;

struct ast_t {
    ast_kind_t kind;
    ast_value_t value;
    token_position_t position;
};

ast_t *ast_new();
void ast_assert_kind(const char *source_file_path, const ast_t *ast, ast_kind_t kind);
const char *ast_kind_value_as_string(const ast_kind_t kind);
const char *ast_kind_as_string(const ast_t *ast);

typedef struct {
    ast_t *free_ptr;
    ast_t *items;
    size_t count;
    size_t capacity;
} asts_t;

#endif // AST_H

#ifdef AST_IMPLEMENTATION

ast_t *ast_new() {
    ast_t *result = calloc(1, sizeof(ast_t));
    return result;
}

void ast_assert_kind(const char *source_file_path, const ast_t *ast, ast_kind_t kind) {
    error_assert(ast->kind == kind, "%s:%zu:%zu expected <%s> but found <%s>",
        source_file_path,
        ast->position.row,
        ast->position.column,
        ast_kind_value_as_string(kind), 
        ast_kind_as_string(ast)
    );
}

const char *ast_kind_value_as_string(const ast_kind_t kind) {
    static_assert(AST_KIND_COUNT == 3, "[DEV] make sure you handle the new kind in ast_kind_as_string");

    switch (kind) {
        case AST_KIND_INTEGER:  return "Integer";
        case AST_KIND_RANGE:    return "Range";
        case AST_KIND_PRINT:    return "Print";
        default: return "[DEV] ast_kind_as_string: unkown ast kind";
    }
}

const char *ast_kind_as_string(const ast_t *ast) {
    return ast_kind_value_as_string(ast->kind);
}

#endif // AST_IMPLEMENTATION