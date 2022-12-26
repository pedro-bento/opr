#include <stdbool.h>

#include "token.h"
#include "std/sv.h"
#include "std/da.h"

typedef struct {
    sv_t ngraph;
    token_t token;
} primitive_t;

// lexer_primitives must be sorted by ngraph length
const primitive_t lexer_primitives[] = {
    { SV_STATIC(".."), TOKEN_STATIC(TOKEN_KIND_DOUBLE_DOT) },
};
const size_t lexer_primitives_count = (sizeof(lexer_primitives) / sizeof(lexer_primitives[0]));

typedef struct {
    size_t current_row;
    size_t current_column;
} lexer_position_t;

typedef struct {
    bool is_error;
    char current_char;
    lexer_position_t position;
} lexer_error_t;

lexer_error_t tokens_from_sv(tokens_t *tokens, sv_t sv);

bool lexer_try_take_primitive_from_sv(lexer_position_t *position, sv_t *sv, token_t *primitive);
bool lexer_try_take_integer_from_sv(lexer_position_t *position, sv_t *sv, token_t *integer);
void lexer_token_with_position(token_t *token, const lexer_position_t *position);
void lexer_sv_advance_n(lexer_position_t *position, sv_t *sv, size_t n);
void lexer_advance_with_char(lexer_position_t *position, char c);

lexer_error_t tokens_from_sv(tokens_t *tokens, sv_t sv) {
    lexer_position_t position = {
        .current_row = 1,
        .current_column = 1,
    };

    while (sv.count > 0) {
        bool taken;
        token_t token;

        sv_t spaces = sv_take_while(sv, sv_isspace);
        lexer_sv_advance_n(&position, &sv, spaces.count);

        taken = lexer_try_take_primitive_from_sv(&position, &sv, &token);
        if (taken) {
            da_append(tokens, &token);
            continue;
        }

        taken = lexer_try_take_integer_from_sv(&position, &sv, &token);
        if (taken) {
            da_append(tokens, &token);
            continue;
        }

        if (sv.count > 0) {
            return (lexer_error_t){ 
                .is_error = true,
                .current_char = sv.data[0], 
                .position = position,
            };
        }
    }

    return (lexer_error_t){ .is_error = false };
}

bool lexer_try_take_integer_from_sv(lexer_position_t *position, sv_t *sv, token_t *integer) {
    if (!sv_first_is(*sv, sv_isdigit)) return false;

    uint64_t value;
    sv_t u64 = sv_take_u64(*sv, &value);

    *integer = TOKEN_INTEGER(value);
    lexer_token_with_position(integer, position);

    lexer_sv_advance_n(position, sv, u64.count);

    return true;
}

bool lexer_try_take_primitive_from_sv(lexer_position_t *position, sv_t *sv, token_t *primitive) {
    for (size_t i = 0; i < lexer_primitives_count; i++) {
        if (sv_has_prefix(*sv, lexer_primitives[i].ngraph)) {
            *primitive = lexer_primitives[i].token;
            lexer_token_with_position(primitive, position);

            lexer_sv_advance_n(position, sv, lexer_primitives[i].ngraph.count);

            return true;
        }
    }
    return false;
}

void lexer_token_with_position(token_t *token, const lexer_position_t *position) {
    token->position.row = position->current_row;
    token->position.column = position->current_column;
}

void lexer_sv_advance_n(lexer_position_t *position, sv_t *sv, size_t n) {
    for (size_t i = 0; i < n && i < sv->count; i++)
        lexer_advance_with_char(position, sv->data[i]);

    *sv = sv_drop_n(*sv, n);
}

void lexer_advance_with_char(lexer_position_t *position, char c) {
    position->current_column++;
    if (c == '\n') {
        position->current_row++;
        position->current_column = 1;
    }
}