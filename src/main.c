#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "writter.c"
#include "compiler.c"
#include "parser.c"
#include "lexer.c"

#define STATEMENT_IMPLEMENTATION
#include "statement.h"

#define AST_IMPLEMENTATION
#include "ast.h"

#define TOKEN_IMPLEMENTATION
#include "token.h"

#define STD_IMPLEMENTATION
#include "std/std.c"

const char *shift_args(int *argc, char ***argv) {
    error_assert(*argc > 0, "empty arguments cannot be shifted");

    char *result = **argv;
    *argc -= 1;
    *argv += 1;

    return result;
}

#define INFO1
#define INFO2

int main(int argc, char **argv) {
    const char *this_name = shift_args(&argc, &argv);

    error_assert(argc >= 1, "Usage: %s <input>\nERROR: no input file provided\n", this_name);
    const char *source_file_path = shift_args(&argc, &argv);

    sv_t source;
    error_t error = sv_from_file_path(source_file_path, &source);
    error_panic_on(error, "ERROR: could not read file <%s>", source_file_path);

    tokens_t tokens = {0};
    lexer_error_t ret = tokens_from_sv(&tokens, source);
    error_assert(!ret.is_error, "%s:%zu:%zu could not tokenize character: %c\n", source_file_path, ret.position.current_row, ret.position.current_column, ret.current_char);

    asts_t asts = {0};
    parser_return_t parser_ret = parser_asts_from_tokens(&tokens, &asts);
    {
        token_position_t position = parser_ret.current_token.position;
        error_assert(!parser_ret.is_error, "%s:%zu:%zu could not parse token: %s\n", source_file_path, position.row, position.column, token_kind_as_string(&parser_ret.current_token));
    }

    program_t program = {0};
    compiler_compile_asts(source_file_path, &asts, &program);

    FILE *c_file = fopen("temp.c", "w");
    write_program_to_c_file(c_file, &program);

#ifdef INFO1
    fprintf(stdout, "--------------------------------\n");
    fprintf(stdout, "source: %zu\n", source.count);
    fprintf(stdout, "tokens: %zu\n", tokens.count);
#ifdef INFO2
    for (size_t i = 0; i < tokens.count; i++)
        fprintf(stdout, "  %s:"TOKEN_Fmt"\n", source_file_path, TOKEN_Arg(&tokens.items[i]));
#endif
    fprintf(stdout, "asts: %zu\n", asts.count);
    fprintf(stdout, "program global declarations: %zu\n", program.global_declarations.count);
    fprintf(stdout, "--------------------------------\n");
#endif

    return 0;
}