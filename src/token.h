#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    TOKEN_KIND_INTEGER = 0,
    TOKEN_KIND_DOUBLE_DOT,
    TOKEN_KIND_DOT_P,
    TOKEN_KIND_COUNT,
} token_kind_t;

typedef union {
    int64_t integer;
} token_value_t;

typedef struct {
    size_t row;
    size_t column;
} token_position_t;

typedef struct {
    token_kind_t kind;
    token_value_t value;
    token_position_t position;
} token_t;

#define TOKEN_STATIC(kind) { kind, .value = {0} }
#define TOKEN_INTEGER(value) (token_t){ .kind = TOKEN_KIND_INTEGER, .value = (token_value_t){ .integer = (int64_t)(value) } }

#define TOKEN_Fmt "%zu:%zu\t%s"
#define TOKEN_Arg(token) (token)->position.row, (token)->position.column, token_kind_as_string(token)

const char *token_kind_as_string(const token_t *token);

typedef struct {
    token_t *free_ptr;
    token_t *items;
    size_t count;
    size_t capacity;
    size_t idx;
} tokens_t;

token_t tokens_temp_shift(tokens_t *tokens);
void tokens_temp_unshift(tokens_t *tokens, size_t diff);
void tokens_commit_shift(tokens_t *tokens);
token_t tokens_current(const tokens_t *tokens);

#endif // TOKEN_H

#ifdef TOKEN_IMPLEMENTATION

const char *token_kind_as_string(const token_t *token) {
    static_assert(TOKEN_KIND_COUNT == 3, "[DEV] make sure you handle the new kind in token_kind_as_string");

    switch (token->kind) {
        case TOKEN_KIND_INTEGER:    return "Integer";
        case TOKEN_KIND_DOUBLE_DOT: return "Double Dot\t(..)";
        case TOKEN_KIND_DOT_P:      return "Dot P\t(.p)";
        default: return "[DEV] token_kind_as_string: unkown token kind";
    }
}

token_t tokens_temp_shift(tokens_t *tokens) {
    token_t result = {0};

    if (tokens->count < 1) return result;
    result = tokens->items[tokens->idx];

    tokens->count--;
    tokens->idx++;

    return result;
}

void tokens_commit_shift(tokens_t *tokens) {
    tokens->items += tokens->idx;
    tokens->idx = 0;
}

void tokens_temp_unshift(tokens_t *tokens, size_t diff) {
    assert(tokens->idx >= diff);
    tokens->idx -= diff;
    tokens->count += diff;
}

token_t tokens_current(const tokens_t *tokens) {
    return tokens->items[tokens->idx];
}

#endif // TOKEN_IMPLEMENTATION