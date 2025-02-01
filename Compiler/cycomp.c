#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FILENAME 256

_Bool instr = false;

// Structure for keyword mappings
typedef struct {
    const char* cstar_keyword;
    const char* c_keyword;
} KeywordMapping;

// Keyword mappings for C* to C translation
KeywordMapping mappings[] = {
    {"?xhport","include <"},
    {":\n", ".h>\n"},
    {"2reviewed", "_Bool"},
    {"mreviewed", "float"},
    {"lreviewed", "long"},
    {"oprint", "printf"},
    {" @?build ", "struct "},
    {"nonreviewed", "void"},
    {"reviewed", "int"},
    {"|>",")"},
    {"<|","("},
    {"@?bye","int* ptr = nullptr;\n *ptr = 0;"},
    {"@\\","#"},
    {"<|>","()"},
};

// Function to translate C* keywords to C equivalents
void translate_cstar_to_c(const char* input, const char* output) {
    FILE* infile = fopen(input, "r");
    FILE* outfile = fopen(output, "w");

    if (!infile || !outfile) {
        printf("Error opening files.\n");
        exit(1);
    }

    char line[8192];
    while (fgets(line, sizeof(line), infile)) {
        // To understand if in a string
        for (char* p = line; *p; p++) {
            if (*p == '"' && (p == line || *(p - 1) != '\\')) {
                instr = !instr; // Toggle the in_string flag
            }

            // Check if in a string, to prevent printing struct instead of @?build, or for others.
            if (!instr) {
                // Replace each keyword in the line
                for (size_t i = 0; i < sizeof(mappings) / sizeof(mappings[0]); i++) {
                    char* pos = strstr(p, mappings[i].cstar_keyword);
                    while (pos && !instr) {
                        // Replace the keyword with its C equivalent
                        memcpy(pos, mappings[i].c_keyword, strlen(mappings[i].c_keyword));
                        // Shift the rest of the line to the left if the replacement is shorter
                        if (strlen(mappings[i].cstar_keyword) > strlen(mappings[i].c_keyword)) {
                            memmove(pos + strlen(mappings[i].c_keyword),
                                    pos + strlen(mappings[i].cstar_keyword),
                                    strlen(pos + strlen(mappings[i].cstar_keyword)) + 1);
                        }
                        pos = strstr(pos + strlen(mappings[i].c_keyword), mappings[i].cstar_keyword);
                    }
                }
            }
        }

        // Write the modified line to the output file
        fprintf(outfile, "%s", line);
    }

    fclose(infile);
    fclose(outfile);
}

// Function to compile C code with the specified compiler
void compile_with_scc(const char* compiler, const char* c_file, const char* output_exe) {
    char command[512];
    snprintf(command, sizeof(command), "%s -o %s %s", compiler, output_exe, c_file);
    int result = system(command);
    if (result != 0) {
        printf("Compilation failed with error code %d.\n", result);
        exit(1);
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <source.cy> [-scc=compiler] [-aarch] [-keep-c]\n", argv[0]);
        return 1;
    }

    // Validate input file extension
    const char* extension = strrchr(argv[1], '.');
    if (!extension || strcmp(extension, ".cy") != 0) {
        printf("\033[1;41mError:\033[1;0m Input file must have a .cy extension.\n");
        return 1;
    }

    char c_output[] = "output.c";
    char exe_output[] = "program";
    char compiler[MAX_FILENAME] = "gcc";
    int aarch = 0, keep_c = 0;

    // Parse command-line arguments
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "-scc=", 5) == 0) {
            strcpy(compiler, argv[i] + 5);
        } else if (strcmp(argv[i], "-aarch") == 0) {
            aarch = 1;
        } else if (strcmp(argv[i], "-keep-c") == 0) {
            keep_c = 1;
        }
    }

    // Handle ARM architecture rules
    if (aarch) {
        if (strcmp(compiler, "tcc") == 0) {
            printf("\033[1;41mError:\033[1;0m tcc is not available for aarch/ARM devices.\n");
            return 1;
        }
        strcpy(compiler, "gcc");
    } else if (strcmp(compiler, "tcc") != 0) {
        strcpy(compiler, "clang");
    }

    // Translate C* to C
    printf("Translating C* to C...\n");
    translate_cstar_to_c(argv[1], c_output);

    // Compile the generated C code
    printf("Compiling with %s...\n", compiler);
    compile_with_scc(compiler, c_output, exe_output);

    // Clean up intermediate C file if not requested to keep it
    if (!keep_c) {
        remove(c_output);
    }

    printf("Compilation finished! Run './%s' to execute.\n", exe_output);
    return 0;
}
