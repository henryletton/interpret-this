#include "nlab.h"

// This is closely based on Neill's "on_error" function in "general.h"
void my_error(const char* str)
{
   fprintf(stderr, "%s\n", str);
   exit(EXIT_FAILURE);
}

// This extends the standard error to print token information
void my_error_token(const char* str, token* t)
{
   fprintf(stderr, "Error identified at line: %d, col: %d",
           t->line, t->col);
   fprintf(stderr, " for token: %s.\n", t->str);
   my_error(str);
}

// This is closely based on Neill's "ncalloc" function in "general.h"
void* my_calloc(int n, size_t sz)
{
   void* mc = calloc(n, sz);
   if(!mc){
      my_error("Unable to calloc space");
   }
   return mc;
}

// idx1 is the first index and up until idx2 is captured (but not including)
char* substring(char* full_str, int idx1, int idx2)
{
   // Catch cases where indexes are not valid
   assert(idx1 >= 0);
   assert(idx2 <= (int)strlen(full_str));
   assert(idx2 > idx1);
   // When extracting string, need space for null character (+1)
   char* str = my_calloc(idx2 - idx1 + 1, sizeof(char));
   strncpy(str, &full_str[idx1], idx2 - idx1);
   return str;
}

// index as in program.vars
int char_to_idx(char c)
{
   // Catch temp variables x,y
   if(c == 'x'){
      return 26;
   }else if(c == 'y'){
      return 27;
   }
   // Only other accepted characters are the 26 upper-case letters
   assert(c >= 'A');
   assert(c <= 'Z');
   // c - 'A' gives 0 for 'A', 1 for 'B' ... 25 for 'Z'
   return c - 'A';
}

program make_program(char* text)
{
   program p;
   // Zero entire string contents
   memset(p.prog_text, 0, sizeof(p.prog_text));
   strcpy(p.prog_text, text);
   p.output[0] = '\0';
   p.token_cnt = 0;
   p.grammar_cnt = 0;
   p.brace_depth = 0;
   char letter = 'A';
   for(int v_idx = 0; v_idx < ALPHASIZE; v_idx++){
      p.vars[v_idx].letter = letter++;
      p.vars[v_idx].set = false;
      p.vars[v_idx].cols = p.vars[v_idx].rows = 0;
   }
   // Temp variables x,y are also set up
   p.vars[26].letter = 'x';
   p.vars[27].letter = 'y';
   p.vars[26].set = p.vars[27].set = false;
   return p;
}

program make_program_file(char* file_name)
{
   program p = make_program("");
   FILE* fp = fopen(file_name, "r");
   char error[MAXLENFILENAME];
   if(!fp){
      sprintf(error, "Could not connect to file: %s", file_name);
      my_error(error);
   }
   if(fread(p.prog_text, MAXLENFILETEXT, 1, fp)){
      sprintf(error, "Could not read from file: %s", file_name);
      my_error(error);
   }
   fclose(fp);
   return p;
}

