// Lexing the program takes text and creates an array of tokens

#include "nlab.h"

bool valid_token(token* t);
bool is_variable(char* str);
bool is_string(char* str);
bool is_integer(char* str);
bool match_any(char* str, int n, ...);
bool is_space_char(char c);
token make_token(char* str, int line, int col);
bool lexer(program* p);
bool add_token(program* p, token t);

bool valid_token(token* t)
{
   if(strcmp(t->str, "{") == 0){
      t->type = lbrace;
      t->g_type = MISC;
      return true;
   }else if(strcmp(t->str, "}") == 0){
      t->type = rbrace;
      t->g_type = MISC;
      return true;
   }else if(strcmp(t->str, "SET") == 0){
      t->type = keyword;
      t->g_type = SET;
      return true;
   }else if(strcmp(t->str, "LOOP") == 0){
      t->type = keyword;
      t->g_type = LOOP;
      return true;
   }else if(strcmp(t->str, "PRINT") == 0){
      t->type = keyword;
      t->g_type = PRINT;
      return true;
   }else if(match_any(t->str, 2, "ONES", "READ")){
      t->type = keyword;
      t->g_type = CREATE;
      return true;
   }else if(strcmp(t->str, "BEGIN") == 0){
      t->type = keyword;
      t->g_type = MISC;
      return true;
   }else if(match_any(t->str, 2, "U-NOT", "U-EIGHTCOUNT")){
      t->type = keyword;
      t->g_type = UNARYOP;
      return true;
   }else if(match_any(t->str, 7, "B-AND", "B-OR", "B-GREATER", "B-LESS", "B-ADD",
                      "B-TIMES", "B-EQUALS")){
      t->type = keyword;
      t->g_type = BINARYOP;
      return true;
   }else if(is_variable(t->str)){
      t->type = variable;
      t->g_type = VARNAME;
      return true;
   }else if(is_string(t->str)){
      t->type = string;
      t->g_type = STRING;
      return true;
   }else if(strcmp(t->str, ";") == 0){
      t->type = semicolon;
      t->g_type = MISC;
      return true;
   }else if(strcmp(t->str, ":=") == 0){
      t->type = assignment;
      t->g_type = MISC;
      return true;
   }else if(is_integer(t->str)){
      t->type = integer;
      t->g_type = INTEGER;
      return true;
   }
   return false;
}

bool is_variable(char* str)
{
   if(strlen(str) != 2){
      return false;
   }
   if(str[0] != '$'){
      return false;
   }
   if(isupper(str[1])){
      return true;
   }
   return false;
}

bool is_string(char* str)
{
   int len = strlen(str);
   // Allow empty string
   if(len < 2){
      return false;
   }
   if(str[0] != '\"' || str[len - 1] != '\"'){
      return false;
   }
   // Do not check for spaces as dealt with in separate functions
   return true;
}

//
bool is_integer(char* str)
{
   int len = strlen(str);
   if(len < 1){
      return false;
   }
   for(int idx = 0; idx < len; idx++){
      if(!isdigit(str[idx])){
         return false;
      }
   }
   return true;
}

// Does a given string str match any of the other strings (which there are n)
bool match_any(char* str, int n, ...)
{
   va_list args;
   va_start(args, n);
   for(int i = 0; i < n; i++){
      if(strcmp(str, va_arg(args, char*)) == 0){
         va_end(args);
         return true;
      }
   }
   va_end(args);
   return false;
}

// A character that separates tokens
bool is_space_char(char c)
{
   if(c == ' '){
      return true;
   }
   if(c == '\n'){
      return true;
   }
   if(c == '\r'){
      return true;
   }
   // Needed for last token if not followed by any char above
   // i.e. if closing } had no new line following
   if(c == '\0'){
      return true;
   }
   return false;
}

token make_token(char* str, int line, int col)
{
   token t;
   strcpy(t.str, str);
   t.line = line;
   t.col = col;
   return t;
}

bool lexer(program* p)
{
   // Do not allow a rerun of lexer (if tokens already exist)
   if(p->token_cnt > 0){
      return false;
   }
   int s_idx = 0, e_idx = 1, len = strlen(p->prog_text);
   int line = 1, col = 1;
   char* str;
   token t;
   while(s_idx < len){
      if(p->prog_text[s_idx] == '\n'){
         line++;
         // Start at zero as will always be incremented a few lines below to 1
         col = 0;
      }
      // Ignore comments (lines after #)
      if(p->prog_text[s_idx] == '#'){
         if(p->prog_text[e_idx] == '\n' || e_idx >= len){
            s_idx = e_idx++;
         }else{
            e_idx++;
         }
      // If starting character is space char, then increment and set end after
      }else if(is_space_char(p->prog_text[s_idx])){
         e_idx = ++s_idx + 1;
         col++;
      // If end character is not a space, then increment
      }else if(!is_space_char(p->prog_text[e_idx])){
         e_idx++;
      // If start is not a space and end is, then can lex as token
      }else{
         str = substring(p->prog_text, s_idx, e_idx);
         t = make_token(str, line, col);
         if(!add_token(p, t)){
            my_error_token("Not a valid token.", &t);
         }
         col = col + strlen(str);
         free(str);
         s_idx = e_idx++;
      }
   }
   return true;
}

bool add_token(program* p, token t)
{
   if(!valid_token(&t)){
      return false;
   }
   if(p->token_cnt >= MAXTOKENNUM){
      my_error("Too many program tokens. Consider increasing MAXTOKENNUM.");
   }
   p->tokens[p->token_cnt] = t;
   p->token_cnt++;
   return true;
}

