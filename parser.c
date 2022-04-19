// Parsing takes the token list and creates a grammar syntax tree
#include "nlab.h"

// Parsing requires that the lexer process has run previously, and so the
// token list is populated
bool parser(program* p);
bool parse_PROG(program* p);
int parse_INSTRC(program* p, int token_idx);
int parse_SET(program* p, int token_idx);
int parse_POLISHLIST(program* p, int token_idx);
int parse_POLISH(program* p, int token_idx);
bool is_POLISH(token* t);
int parse_CREATE(program* p, int token_idx);
int parse_ONES(program* p, int token_idx);
int parse_READ(program* p, int token_idx);
int parse_LOOP(program* p, int token_idx);
int parse_LOOP_end(program* p, int token_idx);
bool is_printable(token* t);
int parse_PRINT(program* p, int token_idx);
grammar make_grammar(grammar_t type, int g_idx1,
                     int t_idx1, int t_idx2, int t_idx3);
bool add_grammar(program* p, grammar g);

grammar make_grammar(grammar_t type, int g_idx1,
                     int t_idx1, int t_idx2, int t_idx3)
{
   grammar g;
   g.type = type;
   g.token_idx1 = t_idx1;
   g.token_idx2 = t_idx2;
   g.token_idx3 = t_idx3;
   g.gram_idx1 = g_idx1;
   return g;
}

bool add_grammar(program* p, grammar g)
{
   // Note, there should always be less grammar than tokens
   if(p->grammar_cnt >= MAXTOKENNUM){
      my_error("Too much program grammar. Consider increasing MAXTOKENNUM.");
   }
   p->grammar_list[p->grammar_cnt] = g;
   p->grammar_cnt++;
   return true;
}

bool parser(program* p)
{
   // Do not allow a rerun of parser (if grammar already exist)
   if(p->grammar_cnt > 0){
      return false;
   }
   parse_PROG(p);
   p->brace_depth = 1;
   // First two tokens and last token are checked in parse_PROG function
   // so start at index 2 and finish before token_cnt - 1
   int token_idx = 2;
   while(token_idx < p->token_cnt - 1){
      token_idx = parse_INSTRC(p, token_idx);
   }
   if(p->brace_depth > 1){
      my_error("Error, reached end of program while still expecting "
               "right brace.");
   }
   return true;
}

// Every program needs to start the same way "BEGIN {"
// and end with "}"
bool parse_PROG(program* p)
{
   if(strcmp(p->tokens[0].str, "BEGIN") != 0){
      my_error_token("Error, expected keyword BEGIN at program start.",
                     &p->tokens[0]);
   }else if(p->tokens[1].type != lbrace){
      my_error_token("Error, left brace should follow BEGIN.",
                     &p->tokens[1]);
   }else if(p->tokens[p->token_cnt - 1].type != rbrace){
      my_error_token("Error, right brace should end program.",
                     &p->tokens[p->token_cnt - 1]);
   }
   return true;
}

int parse_INSTRC(program* p, int token_idx)
{
   token* t = &p->tokens[token_idx];
   switch(t->g_type){
      case SET:
         token_idx = parse_SET(p, token_idx + 1);
         return token_idx;
      case PRINT:
         token_idx = parse_PRINT(p, token_idx + 1);
         return token_idx;
      case CREATE:
         token_idx = parse_CREATE(p, token_idx);
         return token_idx;
      case LOOP:
         token_idx = parse_LOOP(p, token_idx + 1);
         p->brace_depth++;
         return token_idx;
      default:
         // LOOP end is not an official grammar but caught here
         if(t->type == rbrace){
            p->brace_depth--;
            if(p->brace_depth < 1){
               my_error_token("Error, unmatched right brace.",
                              &p->tokens[token_idx]);
            }
            token_idx = parse_LOOP_end(p, token_idx);
            return token_idx;
         }
         my_error_token("Error, expected one of the following: "
                        "SET, LOOP, ONES, READ, PRINT, }", t);
         return token_idx;
   }
}

int parse_PRINT(program* p, int token_idx)
{
   token* t = &p->tokens[token_idx];
   if(!is_printable(t)){
      my_error_token("Error, expected either variable or string.", t);
   }
   grammar g = make_grammar(PRINT, -1, token_idx, -1, -1);
   add_grammar(p, g);
   // +1 as next token after variable
   return token_idx + 1;
}

bool is_printable(token* t)
{
   if(t->type == variable){
      return true;
   }
   if(t->type == string){
      return true;
   }
   return false;
}

int parse_SET(program* p, int token_idx)
{
   token* t0 = &p->tokens[token_idx];
   token* t1 = &p->tokens[token_idx + 1];
   token* t2 = &p->tokens[token_idx + 2];
   grammar g;
   if(t0->type != variable){
      my_error_token("Error, expected a VARNAME.", t0);
   }
   if(t1->type != assignment){
      my_error_token("Error, expected an assignment (:=).", t1);
   }
   if(t2->type == semicolon){
      g = make_grammar(SET, -1, token_idx, -1, -1);
      add_grammar(p, g);
      // +3 as $A := 2 foramt with 3 tokens
      return token_idx + 3;
   }else{
      g = make_grammar(SET, p->grammar_cnt + 1, token_idx, -1, -1);
      add_grammar(p, g);
   }
   // First POLISHLIST starts after ":=" hence +2
   token_idx = parse_POLISHLIST(p, token_idx + 2);
   return token_idx;
}

int parse_POLISHLIST(program* p, int token_idx)
{
   token t = p->tokens[token_idx];
   while(t.type != semicolon){
      token_idx = parse_POLISH(p, token_idx);
      t = p->tokens[token_idx];
   }
   // +1 as next grammar after last POLISH
   return token_idx + 1;
}

int parse_POLISH(program* p, int token_idx)
{
   token t0 = p->tokens[token_idx];
   token t1 = p->tokens[token_idx + 1];
   grammar g;
   if(!is_POLISH(&t0)){
      my_error_token("Error, expected a valid POLISH "
                     "(i.e. integer, varname, UNARYOP, BINARYOP).", &t0);
   }
   if(t1.type == semicolon){
      g = make_grammar(POLISH, -1, token_idx, -1, -1);
   }else{
      g = make_grammar(POLISH, p->grammar_cnt + 1, token_idx, -1, -1);
   }
   add_grammar(p, g);
   // +1 as POLISH is only one token
   return token_idx + 1;
}

bool is_POLISH(token* t)
{
   switch (t->g_type){
      case VARNAME:
      case INTEGER:
      case UNARYOP:
      case BINARYOP:
         return true;
      default:
         return false;
   }
}

int parse_CREATE(program* p, int token_idx)
{
   token* t = &p->tokens[token_idx];
   if(strcmp(t->str, "ONES") == 0){
      return parse_ONES(p, token_idx + 1);
   }else if(strcmp(t->str, "READ") == 0){
      return parse_READ(p, token_idx + 1);
   }
   my_error("Unreachable code. CREATE is either ONES or READ.");
   return token_idx + 1;
}

int parse_ONES(program* p, int token_idx)
{
   token* t0 = &p->tokens[token_idx];
   token* t1 = &p->tokens[token_idx + 1];
   token* t2 = &p->tokens[token_idx + 2];
   grammar g;
   if(t0->g_type != INTEGER){
      my_error_token("Error, expected an integer for row number.", t0);
   }
   if(t1->g_type != INTEGER){
      my_error_token("Error, expected an integer for column number.", t1);
   }
   if(t2->g_type != VARNAME){
      my_error_token("Error, expected a variable name.", t2);
   }
   g = make_grammar(CREATE, -1, token_idx, token_idx + 1, token_idx + 2);
   add_grammar(p, g);
   // +3 as 3 parts to ONES grammar with information
   return token_idx + 3;
}

int parse_READ(program* p, int token_idx)
{
   token* t0 = &p->tokens[token_idx];
   token* t1 = &p->tokens[token_idx + 1];
   grammar g;
   if(t0->g_type != STRING){
      my_error_token("Error, expected a string containing file "
                     "and file directory.", t0);
   }
   if(t1->g_type != VARNAME){
      my_error_token("Error, expected a variable name.", t1);
   }
   g = make_grammar(CREATE, -1, token_idx, token_idx + 1, -1);
   add_grammar(p, g);
   // +2 as 2 parts to READ grammar with information
   return token_idx + 2;
}

int parse_LOOP(program* p, int token_idx)
{
   token* t0 = &p->tokens[token_idx];
   token* t1 = &p->tokens[token_idx + 1];
   token* t2 = &p->tokens[token_idx + 2];
   if(t0->g_type != VARNAME){
      my_error_token("Error, expected a variable name.", t0);
   }
   if(t1->g_type != INTEGER){
      my_error_token("Error, expected a loop integer.", t1);
   }
   if(t2->type != lbrace){
      my_error_token("Error, expected a left brace.", t2);
   }
   grammar g;
   g = make_grammar(LOOP, -1, token_idx, token_idx + 1, -1);
   add_grammar(p, g);
   // Already accounted for "LOOP" so <VARNAME>, <INTEGER>, "{" mean +3
   return token_idx + 3;
}

int parse_LOOP_end(program* p, int token_idx)
{
   // Find the index of the corresponding LOOP start
   int cur_g_idx = p->grammar_cnt - 1;
   grammar* cur_g = &p->grammar_list[cur_g_idx];
   // If LOOP grammar link != -1, it will already have been paired with an end
   while(cur_g->type != LOOP || cur_g->gram_idx1 != -1){
      cur_g = &p->grammar_list[--cur_g_idx];
      if(cur_g_idx < 0){
         my_error_token("Error, unmatched right brace.",
                        &p->tokens[token_idx]);
      }
   }
   // Now paired set grammar link to LOOP_end location
   p->grammar_list[cur_g_idx].gram_idx1 = p->grammar_cnt;
   grammar g = make_grammar(LOOP_end, cur_g_idx, -1, -1, -1);
   add_grammar(p, g);
   // LOOP end is } token so +1
   return token_idx + 1;
}

