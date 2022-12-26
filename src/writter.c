#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "statement.h"
#include "std/sv.h"
#include "std/da.h"

typedef struct {
    sv_t *free_ptr;
    sv_t *items;
    size_t count;
    size_t capacity;
} names_t;

bool name_exists_in(names_t names, sv_t name) {
    for (size_t i = 0; i < names.count; i++)
        if (sv_equal(names.items[i], name))
            return true;
    return false;
}

names_t global_declarations_names = {0};

void write_global_declaration_to_c_file(FILE *file, const statement_t *global_declaration) {
    static_assert(TOKEN_KIND_COUNT == 3, "[DEV] make sure you handle any new global declaration kind: write_global_declaration_to_c_file\n");
    switch (global_declaration->kind) {
        case STATEMENT_KIND_GLOBAL_DECLARATION_INTEGER_ARRAY: {
            statement_global_declaration_integer_array_value_t array = global_declaration->value.global_declaration_integer_array;

            // global declarations are allways constant, we can reuse names
            if (name_exists_in(global_declarations_names, array.declarator)) return;
            da_append(&global_declarations_names, &array.declarator);

            fprintf(file, "const Integer "SV_Fmt"[%"PRId64"] = {", SV_Arg(array.declarator), array.count);
            for (size_t i = 0; i < array.count; i++) {
                fprintf(file, "%"PRId64, array.items[i]);
                if (i < array.count-1) fprintf(file, ", ");
            }
            fprintf(file, "};\n");
            fprintf(file, "const Integer "SV_Fmt" = sizeof("SV_Fmt") / sizeof(*"SV_Fmt");\n", SV_Arg(array.declarator_count), SV_Arg(array.declarator), SV_Arg(array.declarator));

        } break;
        
        default:
            assert(false && "not implemented yet!");
            break;
    }
}

void write_main_body_statement_to_c_file(FILE *file, const statement_t *statement) {
    static_assert(TOKEN_KIND_COUNT == 3, "[DEV] make sure you handle any new global declaration kind: write_global_declaration_to_c_file\n");
    switch (statement->kind) {
        case STATEMENT_KIND_PRINT_INTEGER_ARRAY: {
            statement_call_print_integer_array_value_t print = statement->value.call_print_integer_array;

            fprintf(file, "\tfprintf(stdout, \"{\");\n");
            fprintf(file, "\tfor (Integer i = 0; i < "SV_Fmt"; i++) {\n", SV_Arg(print.variable_count));
            fprintf(file, "\t\tfprintf(stdout, \"%%lld\", "SV_Fmt"[i]);\n", SV_Arg(print.variable));
            fprintf(file, "\t\tif (i < "SV_Fmt"-1) fprintf(stdout, \", \");\n", SV_Arg(print.variable_count));
            fprintf(file, "\t}\n");
            fprintf(file, "\tfprintf(stdout, \"}\\n\");\n");

        } break;
        
        default:
            assert(false && "not implemented yet!");
            break;
    }
}

void write_program_to_c_file(FILE *file, const program_t *program) {
    fprintf(file, "#include <stdio.h>\n\n");
    fprintf(file, "typedef signed long long int Integer;\n\n");

    for (size_t i = 0; i < program->global_declarations.count; i++) {
        write_global_declaration_to_c_file(file, &program->global_declarations.items[i]);
    }

    fprintf(file, "\nint main(void) {\n");
    for (size_t i = 0; i < program->main_body.count; i++) {
        write_main_body_statement_to_c_file(file, &program->main_body.items[i]);
        fprintf(file, "\n");
    }
    fprintf(file, "\treturn 0;\n}\n");
}