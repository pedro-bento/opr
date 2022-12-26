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

statements_t compiler_compile_ast_to_statements(const char *source_file_path, const ast_t *ast) {
    static_assert(AST_KIND_COUNT == 3, "[DEV] make sure you handle the new statement kind");
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
            sprintf(buffer, "range_from_%s_to_%s_count", a.data, b.data);
            result.value.global_declaration_integer_array.declarator = sv_make(buffer, strlen(buffer)-sizeof("_count")+1);
            result.value.global_declaration_integer_array.declarator_count = SV_CSTR(buffer);

            statements_t statements = {0};
            da_append(&statements, &result);
            return statements;
        } break;

        case AST_KIND_PRINT: {
            statements_t result = {0};

            const statements_t rights = compiler_compile_ast_to_statements(source_file_path, ast->value.monadic.right);
            assert(rights.count == 1);

            const statement_t right = rights.items[0];
            statement_assert_predicate(source_file_path, &right, statement_is_printable);

            da_append(&result, &right);

            static_assert(STATEMENT_KIND_COUNT == 2, "[DEV] is your new statement printable?");
            switch (right.kind) {
                case STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY: {
                    statement_t print = {0};
                    print.kind = STATEMENT_KIND_PRINT_INTEGER_ARRAY;
                    print.value.call_print_integer_array.variable = right.value.global_declaration_integer_array.declarator;
                    print.value.call_print_integer_array.variable_count = right.value.global_declaration_integer_array.declarator_count;

                    da_append(&result, &print);
                    return result;
                } break;

                default:
                    assert(false && "this should be unreachable, check for missing cases!");
            }
        } break;
            
        default: {
            error_assert(false, "NOT IMPLEMENTED: compiler_compile_ast_to_statement\n");
        }
    }

    assert(false && "UNREACHABLE");
}

void compiler_compile_asts(const char *source_file_path, const asts_t *asts, program_t *program) {
    for (size_t i = 0; i < asts->count; i++) {
        statements_t statements = compiler_compile_ast_to_statements(source_file_path, &asts->items[i]);
        for (size_t i = 0; i < statements.count; i++) {
            switch (statements.items[i].kind) {
                case STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY:
                    da_append(&program->global_declarations, &statements.items[i]);
                    break;
                
                case STATEMENT_KIND_PRINT_INTEGER_ARRAY:
                    da_append(&program->main_body, &statements.items[i]);
                    break;

                default:
                    error_assert(false, "NOT IMPLEMENTED: compiler_compile_asts\n");
                    break;
            }
        }
    }
}