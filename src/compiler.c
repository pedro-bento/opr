#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "statement.h"
#include "ast.h"
#include "std/error.h"
#include "std/da.h"
#include "std/sv.h"

typedef struct {
    char data[20]; // longest value -9223372036854775808
} int64_string_buffer;

int64_string_buffer int64_to_string(int64_t integer) {
    int64_string_buffer result = {0};
    sprintf(result.data, "%"PRId64, integer);

    if (integer < 0) result.data[0] = '_'; // replace - by _

    return result;
}

statement_t compiler_compile_ast_to_statement(const char *source_file_path, const ast_t *ast) {
    static_assert(AST_KIND_COUNT == 2, "[DEV] make sure you handle the new statement kind");
    switch (ast->kind) {
        case AST_KIND_RANGE: {
            const ast_t *lower = ast->value.dyadic.left;
            const ast_t *upper = ast->value.dyadic.right;

            // we may support ranges with other operand kinds in the future
            ast_assert_kind(source_file_path, lower, AST_KIND_INTEGER);
            ast_assert_kind(source_file_path, upper, AST_KIND_INTEGER);
            
            statement_t result = {0};
            result.kind = STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY;
            for (int64_t i = lower->value.integer; i < upper->value.integer; i++) {
                da_append(&result.value.global_declaration_integer_array, &i);
            }

            int64_string_buffer a = int64_to_string(lower->value.integer);
            int64_string_buffer b = int64_to_string(upper->value.integer);

            char *buffer = malloc(sizeof(char)*64);
            sprintf(buffer, "range_from_%s_to_%s", a.data, b.data);
            result.value.global_declaration_integer_array.declarator = SV_CSTR(buffer);

            return result;
        } break;
            
        default: {
            error_assert(false, "NOT IMPLEMENTED: compiler_compile_ast_to_statement\n");
        }
    }

    assert(false && "UNREACHABLE");
}

void compiler_compile_asts(const char *source_file_path, const asts_t *asts, program_t *program) {
    for (size_t i = 0; i < asts->count; i++) {
        statement_t statement = compiler_compile_ast_to_statement(source_file_path, &asts->items[i]);
        switch (statement.kind) {
        case STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY:
            da_append(&program->global_declarations, &statement);
            break;
        
        default:
            error_assert(false, "NOT IMPLEMENTED: compiler_compile_asts\n");
            break;
        }
    }
}