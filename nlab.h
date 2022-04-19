#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// Tokens are a sequence of characters with a location in file and type(s)
#define MAXTOKENLEN 50
#define MAXTOKENNUM 1000
#define MAXLENFILENAME 100
#define MAXLENFILETEXT 10000
#define MAXPRINTLENGTH 10000
#define MAXMATRIXSIZE 10
#define ALPHASIZE 26
// We have two extra variables x,y for use in calculations
#define VARIABLENUM ALPHASIZE+2

// Token types
typedef enum token_t {keyword, variable, string, lbrace, rbrace, semicolon,
              assignment, integer} token_t;

// Different possible grammar as given by nlab.grammar
typedef enum grammar_t {PROG, INSTRCLIST, INSTRC, PRINT, VARNAME, STRING, SET,
              POLISHLIST, POLISH, PUSHDOWN, INTEGER, UNARYOP, BINARYOP,
              // MISC captures things like {, }, [0-9]+, "BEGIN"
              CREATE, ROWS, COLS, FILENAME, LOOP, LOOP_end, MISC} grammar_t;

typedef struct token {
   token_t type;
   grammar_t g_type; // Most tokens have a grammar type too
   int line; // Line and column makes isolating an error easier
   int col;
   char str[MAXTOKENLEN];
} token;

// This is a general purpose structure for any grammar
// At most, a grammar will refer to three tokens and one other grammar
// With new grammar these could easily be extended
struct grammar {
   grammar_t type;
   int gram_idx1;
   int token_idx1;
   int token_idx2;
   int token_idx3;
};
typedef struct grammar grammar;

typedef struct variable_s {
   char letter;
   bool set; // Only true if the value has been defined at some point
   int rows;
   int cols;
   // Fixed size matrix as rows/cols give current used area, and is useful for
   // simplifying certain operations
   unsigned int arr[MAXMATRIXSIZE][MAXMATRIXSIZE];
} variable_s;

typedef struct program {
   char prog_text[MAXLENFILETEXT]; // Text from user or file
   token tokens[MAXTOKENNUM]; // All tokens from lexer
   int token_cnt;
   grammar grammar_list[MAXTOKENNUM]; // All grammar from parser
   int grammar_cnt;
   variable_s vars[VARIABLENUM]; // Space already allocated for all variables
   int brace_depth; // Needed for loop logic
   char output[MAXPRINTLENGTH]; // Output as printed in interpreter
} program;

void my_error(const char* str);
void my_error_token(const char* str, token* t);
void* my_calloc(int n, size_t sz);
char* substring(char* full_str, int idx1, int idx2);
int char_to_idx(char c);
program make_program(char* text);
program make_program_file(char* file_name);
