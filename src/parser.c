#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "token.h"
#include "ast.h"
#include "std/da.h"

typedef struct {
    bool is_error;
    token_t current_token;
} parser_return_t;

bool parser_parse_sentence(tokens_t *tokens, ast_t *sentence);
size_t parser_try_parse_dyadic_from_tokens(tokens_t *tokens, ast_t *dyadic);
size_t parser_try_parse_niladic_from_tokens(tokens_t *tokens, ast_t *niladic);
size_t parser_try_parse_atom_from_tokens(tokens_t *tokens, ast_t *atom);

// Grammar
// 
// atom =
//  | Integer
// 
// unary =
//  | TODO
//
// binary =
//  | ..
//
// niladic =
//  | atom
//  | monadic niladic
//  | niladic dyadic niladic
//
// monadic =
//  | unary
//  | unary monadic
//
// dyadic =
//  | binary
//  | monadic dyadic monadic
// 
// sentence = niladic
//
parser_return_t parser_asts_from_tokens(tokens_t *tokens, asts_t *asts) {
    while (tokens->count > 0) {
        ast_t sentence = {0};
        if(!parser_parse_sentence(tokens, &sentence)) {
            return (parser_return_t){
                .is_error = true,
                .current_token = tokens->items[0],
            };
        }
        da_append(asts, &sentence);
    }

    return (parser_return_t){ .is_error = false };
}

bool parser_parse_sentence(tokens_t *tokens, ast_t *sentence) {
    size_t niladic_diff = parser_try_parse_niladic_from_tokens(tokens, sentence);    
    if (niladic_diff == 0) {
        return false;
    }
    tokens_commit_shift(tokens);

    return true;
}

size_t parser_try_parse_dyadic_from_tokens(tokens_t *tokens, ast_t *dyadic) {
    // TODO: monadic dyadic monadic

    if (tokens->count < 1) return 0;

    static_assert(TOKEN_KIND_COUNT == 3 , "[DEV] new token kinds may add new dyadic verbs");
    if (tokens_current(tokens).kind != TOKEN_KIND_DOUBLE_DOT) return 0;

    token_t token = tokens_temp_shift(tokens);
    dyadic->kind = AST_KIND_RANGE; // Double Dot in the dyadic context is range
    dyadic->position = token.position;

    return 1;
}

size_t parser_try_parse_monadic_from_tokens(tokens_t *tokens, ast_t *monadic) {
    if (tokens->count < 1) return 0;

    static_assert(TOKEN_KIND_COUNT == 3 , "[DEV] new token kinds may add new monadic verbs");
    if (tokens_current(tokens).kind != TOKEN_KIND_DOT_P) return 0;

    // unary
    token_t token = tokens_temp_shift(tokens);
    monadic->kind = AST_KIND_PRINT; // Dot P in the monadic context is print
    monadic->position = token.position;

    // unary monadic

    // TODO: create is_monadic(...)
    if (tokens_current(tokens).kind != TOKEN_KIND_DOT_P) return 1; // we correctly parsed one monadic verb

    ast_t *right = ast_new();
    size_t right_diff = parser_try_parse_monadic_from_tokens(tokens, right);
    if (right_diff == 0) {
        free(right);
        return 0;
    }

    monadic->value.monadic.right = right;

    return 1 + right_diff;
}

size_t parser_try_parse_niladic_from_tokens(tokens_t *tokens, ast_t *niladic) {
    // monadic niladic
    ast_t monadic = {0};
    size_t monadic_diff = parser_try_parse_monadic_from_tokens(tokens, &monadic);
    if (monadic_diff > 0) {
        ast_t *right = ast_new();
        size_t right_diff = parser_try_parse_niladic_from_tokens(tokens, right);
        if (right_diff > 0) {
            monadic.value.monadic.right = right;
            *niladic = monadic;
            return monadic_diff + right_diff;
        } else {
            tokens_temp_unshift(tokens, monadic_diff);
            free(right);
            // keep exec, atom or niladic dyadic niladic may succeed
        } 
    }

    // atom
    ast_t *left = ast_new();
    size_t atom_diff = parser_try_parse_atom_from_tokens(tokens, left);
    if (atom_diff == 0) {
        free(left);
        return 0; // i.e. failed to parse simple atom
    }

    *niladic = *left;

    // niladic dyadic niladic
    ast_t dyadic = {0};
    size_t dyadic_diff = parser_try_parse_dyadic_from_tokens(tokens, &dyadic);
    if (dyadic_diff == 0) {
        free(left);
        return atom_diff; // i.e. failed to parse niladic with dyadic in the midle
    }

    ast_t *right = ast_new();
    size_t right_diff = parser_try_parse_niladic_from_tokens(tokens, right);
    if (right_diff == 0) {
        tokens_temp_unshift(tokens, dyadic_diff);
        free(left);
        free(right);
        return atom_diff; // i.e. failed to parse niladic with dyadic in the midle
    }

    dyadic.value.dyadic.left = left;
    dyadic.value.dyadic.right = right;
    *niladic = dyadic;

    return atom_diff + dyadic_diff + right_diff;
}

size_t parser_try_parse_atom_from_tokens(tokens_t *tokens, ast_t *atom) {
    if (tokens->count < 1) return 0;

    static_assert(TOKEN_KIND_COUNT == 3 , "[DEV] new token kinds may add new nouns");
    if (tokens_current(tokens).kind != TOKEN_KIND_INTEGER) return 0;

    token_t token = tokens_temp_shift(tokens);
    atom->kind = AST_KIND_INTEGER;
    atom->value.integer = token.value.integer;
    atom->position = token.position;

    return 1;
}