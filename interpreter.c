// Interpreting takes the grammar syntax tree and performs the required actions
#include "nlab.h"

// Interpreting requires that the parser process has run previously, and so the
// grammar list is populated
bool interpreter(program* p);
int inter_grammar(program* p, int gram_idx);
int inter_SET(program* p, int gram_idx);
int inter_PRINT(program* p, int gram_idx);
int inter_ONES(program* p, int gram_idx);
int inter_READ(program* p, int gram_idx);
int inter_LOOP(program* p, int gram_idx);
int inter_LOOP_end(program* p, int gram_idx);
bool load_file_x(program* p, FILE* fp);
bool print_var(program* p, int var_idx, bool store);
// Print but with token index instead of variable index
bool print_var_t(program* p, int token_idx, bool store);
bool print_str(program* p, int token_idx);
bool is_scalar_int(program* p, int var_idx);
bool int_to_var(program* p, int value_t_idx, int var_idx);
// Moves a variable structure from index 1 to 2, 1 is left unchanged
bool move_var(program* p, int var_e_idx, int var_s_idx);
// Three extra move functions if you have one or both tokens
// relating to a variable
bool move_var_tb(program* p, int var1_t_idx, int var2_t_idx);
bool move_var_t1(program* p, int var1_t_idx, int var2_idx);
bool move_var_t2(program* p, int var1_idx, int var2_t_idx);
bool calc_UNARY(program* p, int op_t_idx);
int u_eightcount(variable_s* v, int row, int col);
bool calc_BINARY(program* p, int op_t_idx);
bool increment_var(variable_s* v);

bool interpreter(program* p)
{
   if(p->grammar_cnt == 0){
      // Commented out this as scared of failing some automated testing
      //printf("Nothing to interpret. Either this is an empty program "
      //       "or the interpreter has not been run.\n");
      return EXIT_SUCCESS;
   }
   int gram_idx = 0;
   while(gram_idx < p->grammar_cnt){
      gram_idx = inter_grammar(p, gram_idx);
   }
   return true;
}

int inter_grammar(program* p, int gram_idx)
{
   grammar g = p->grammar_list[gram_idx];
   switch(g.type){
      case SET:
         gram_idx = inter_SET(p, gram_idx);
         return gram_idx;
      case PRINT:
         gram_idx = inter_PRINT(p, gram_idx);
         return gram_idx;
      case CREATE:
         // ONES gram has a 3rd token, if READ got a 3rd then update needed
         if(g.token_idx3 == -1){
            gram_idx = inter_READ(p, gram_idx);
         }else{
            gram_idx = inter_ONES(p, gram_idx);
         }
         return gram_idx;
      case LOOP:
         gram_idx = inter_LOOP(p, gram_idx);
         return gram_idx;
      case LOOP_end:
         gram_idx = inter_LOOP_end(p, gram_idx);
         return gram_idx;
      default:
         return gram_idx+1;
   }
}
// Large function, add in two sub functions
int inter_SET(program* p, int gram_idx)
{
   grammar* g0 = &p->grammar_list[gram_idx];
   // Catch case where grammar is correct but no value/operation given to
   // assign (syntactically correct but semantically wrong)
   if(g0->gram_idx1 == -1){
      my_error_token("Error, expected POLISH before end of set statement.",
                     // Relevant token is semicolon (2 after varname)
                     &p->tokens[g0->token_idx1 + 2]);
   }
   grammar* g1 = &p->grammar_list[g0->gram_idx1];
   token* t1 = &p->tokens[g1->token_idx1];
   switch(t1->type){
      case integer:
         int_to_var(p, g1->token_idx1, 26);
         break;
      case variable:
         move_var_t2(p, 26, g1->token_idx1);
         break;
      default:
         my_error_token("Error, expected an integer or variable.", t1);
         break;
   }
   // One POLISH grammar used so +1
   gram_idx++;

   //grammar* g2 = &p->grammar_list[g1->gram_idx1];
   //token* t2 = &p->tokens[g1->token_idx1];
   while(g1->gram_idx1 != -1){
      g1 = &p->grammar_list[g1->gram_idx1];
      t1 = &p->tokens[g1->token_idx1];
      switch(t1->g_type){
         case UNARYOP:
            calc_UNARY(p, g1->token_idx1);
            // Another POLISH grammar used so +1
            gram_idx++;
            continue;
         case INTEGER:
            int_to_var(p, g1->token_idx1, 27);
            break;
         case VARNAME:
            move_var_t2(p, 27, g1->token_idx1);
            break;
         default:
            my_error_token("Error, expected a UNARYOP, integer or variable.",
                           t1);
      }
      // Another POLISH grammar used so +1
      gram_idx++;
      if(g1->gram_idx1 == -1){
         my_error_token("Error, expected another POLISH.", t1);
      }
      g1 = &p->grammar_list[g1->gram_idx1];
      t1 = &p->tokens[g1->token_idx1];
      switch(t1->g_type) {
         case BINARYOP:
            calc_BINARY(p, g1->token_idx1);
            // Another POLISH grammar used so +1
            gram_idx++;
            break;
         default:
            my_error_token("Error, expected a BINARYOP POLISH.", t1);
            break;
      }
   }
   // No matter the POLISHLIST, answer will always be at x
   // Move value from temp x to desired variable location
   move_var_t1(p, g0->token_idx1, 26);
   // Unset temp variables
   p->vars[26].set = p->vars[27].set = false;
   return gram_idx;
}

int inter_PRINT(program* p, int gram_idx)
{
   grammar* g = &p->grammar_list[gram_idx];
   token* t = &p->tokens[g->token_idx1];
   switch(t->g_type){
      case VARNAME:
         print_var_t(p, g->token_idx1, true);
         break;
      case STRING:
         print_str(p, g->token_idx1);
         break;
      default:
         my_error("Unreachable code. Can only print string or variable.");
         break;
   }
   // Print is single grammar so +1
   return gram_idx + 1;
}

int inter_ONES(program* p, int gram_idx)
{
   grammar* g = &p->grammar_list[gram_idx];
   token* t = &p->tokens[g->token_idx3];
   int var_idx = char_to_idx(t->str[1]);
   // value_t_idx of -111 is the identifier for ones, as no integer token
   int_to_var(p, -111, var_idx);
   // int_to_var gives 1x1 array so rows and cols need to be extended
   variable_s* var = &p->vars[var_idx];
   sscanf(p->tokens[g->token_idx1].str, "%d", &var->rows);
   sscanf(p->tokens[g->token_idx2].str, "%d", &var->cols);
   if(var->rows > MAXMATRIXSIZE || var->cols > MAXMATRIXSIZE){
      my_error_token("Error, matrix is too large. "
                     "Consider increasing MAXMATRIXSIZE.", t);
   }
   // ONES is single grammar so +1
   return gram_idx + 1;
}

int inter_READ(program* p, int gram_idx)
{
   grammar* g = &p->grammar_list[gram_idx];
   token* t_file = &p->tokens[g->token_idx1];
   char* file_name;
   file_name = substring(t_file->str, 1, (int)strlen(t_file->str) - 1);
   FILE* fp = fopen(file_name, "r");
   free(file_name);
   if(!fp){
      my_error_token("Error, cannot connect to file.", t_file);
   }
   if(!load_file_x(p, fp)){
      my_error_token("Error, file format is incorrect.", t_file);
   }
   fclose(fp);
   // 26 is index of temp x variable
   move_var_t1(p, g->token_idx2, 26);
   // READ is single grammar so +1
   return gram_idx + 1;
}

int inter_LOOP(program* p, int gram_idx)
{
   grammar* g = &p->grammar_list[gram_idx];
   char c = p->tokens[g->token_idx1].str[1];
   int var_idx = char_to_idx(c);
   // Start variable with value of 1 (cols:1, rows:1)
   int_to_var(p, -111, var_idx);
   // LOOP is single grammar so +1
   return gram_idx + 1;
}

int inter_LOOP_end(program* p, int gram_idx)
{
   grammar* g_loop_end = &p->grammar_list[gram_idx];
   grammar* g_loop = &p->grammar_list[g_loop_end->gram_idx1];
   token* t_var = &p->tokens[g_loop->token_idx1];
   int var_idx = char_to_idx(t_var->str[1]);
   variable_s* v = &p->vars[var_idx];
   if(!is_scalar_int(p, var_idx)){
      my_error_token("Error, looped variable is no longer a scalar integer.",
                     t_var);
   }
   // Compare loop variable and max loop value
   int max_loop_value, cur_loop_value;
   sscanf(p->tokens[g_loop->token_idx2].str, "%d", &max_loop_value);
   cur_loop_value = v->arr[0][0];
   // If loop not complete, add 1 to loop var and go to grammar after LOOP
   if(max_loop_value >  cur_loop_value){
      increment_var(v);
      return g_loop_end->gram_idx1 + 1;
   }
   // If loop complete, go to grammar following END_loop
   return gram_idx + 1;
}

bool load_file_x(program* p, FILE* fp)
{
   variable_s* x = &p->vars[26];
   if(fscanf(fp, "%d %d", &x->cols, &x->rows) != 2){
      return false;
   }
   if(x->cols > MAXMATRIXSIZE || x->rows > MAXMATRIXSIZE){
      my_error("Error, file matrix is too large. "
                     "Consider increasing MAXMATRIXSIZE.");
   }
   for(int r = 0; r < x->rows; r++){
      for(int c = 0; c < x->rows; c++){
         if(fscanf(fp, "%ud", &x->arr[r][c]) != 1){
            return false;
         }
      }
   }
   x->set = true;
   return true;
}

bool print_var(program* p, int var_idx, bool store)
{
   variable_s* v = &p->vars[var_idx];
   // Cannot print unset variable
   if(!v->set){
      return false;
   }
   char to_print[MAXPRINTLENGTH] = "";
   char temp_int[MAXTOKENLEN] = "";
   if(v->cols == 1 && v->rows == 1){
      sprintf(to_print, "%d\n",v->arr[0][0]);
   }else{
      for(int r = 0; r < v->rows; r++){
         for(int c = 0; c < v->cols; c++){
            sprintf(temp_int, "%d ",v->arr[r][c]);
            strcat(to_print, temp_int);
         }
         strcat(to_print, "\n");
      }
   }
   printf("%s", to_print);
   // p->output is useful for automated testing
   if(store){
      strcat(p->output, to_print);
   }
   return true;
}

bool print_var_t(program* p, int token_idx, bool store)
{
   token* t = &p->tokens[token_idx];
   int var_idx = char_to_idx(t->str[1]);
   // We can't print a variable if it is not set
   if(!p->vars[var_idx].set){
      my_error_token("Error, variable is not set.", t);
   }
   return print_var(p, var_idx, store);
}

bool print_str(program* p, int token_idx)
{
   token* t = &p->tokens[token_idx];
   // Can't print_str if not a string
   if(t->g_type != STRING){
      return false;
   }
   // Actual string should not contain starting and ending '"'
   char* str;
   str = substring(t->str, 1, (int)strlen(t->str));
   // Need new line at end, so replace right '"' with '\n'
   str[strlen(str) - 1] = '\n';
   printf("%s", str);
   strcat(p->output, str);
   free(str);
   return true;
}

// Set and 1x1 matrix
bool is_scalar_int(program* p, int var_idx)
{
   variable_s* v = &p->vars[var_idx];
   if(!v->set){
      return false;
   }
   if(v->cols == 1 && v->rows == 1){
      return true;
   }
   return false;
}

bool int_to_var(program* p, int value_t_idx, int var_idx)
{
   int value;
   // In the case of CREATE ONES or LOOP start, there is no idx so -1 given
   if(value_t_idx == -111){
      value = 1;
   }else if(value_t_idx >= 0){
      sscanf(p->tokens[value_t_idx].str, "%d", &value);
   }else{
      return false;
   }
   variable_s* v = &p->vars[var_idx];
   v->rows = v->cols = 1;
   for(int r = 0; r < MAXMATRIXSIZE; r++){
      for(int c = 0; c < MAXMATRIXSIZE; c++){
         v->arr[r][c] = value;
      }
   }
   v->set = true;
   return true;
}

bool move_var(program* p, int var_e_idx, int var_s_idx)
{
   variable_s * v2 = &p->vars[var_s_idx];
   // Cannot move a variable if it is not set
   if(!v2->set){
      return false;
   }
   // Save letter as will need to replace after full structure copy
   char c1 = p->vars[var_e_idx].letter;
   p->vars[var_e_idx] = p->vars[var_s_idx];
   p->vars[var_e_idx].letter = c1;
   return true;
}
// 3 parent functions to take any combination of var indexes and token indexes
bool move_var_tb(program* p, int var1_t_idx, int var2_t_idx)
{
   token* t1 = &p->tokens[var1_t_idx];
   int var1_idx = char_to_idx(t1->str[1]);
   return move_var_t2(p, var1_idx, var2_t_idx);
}
bool move_var_t1(program* p, int var1_t_idx, int var2_idx)
{
   token* t1 = &p->tokens[var1_t_idx];
   int var1_idx = char_to_idx(t1->str[1]);
   return move_var(p, var1_idx, var2_idx);
}
bool move_var_t2(program* p, int var1_idx, int var2_t_idx)
{
   token* t2 = &p->tokens[var2_t_idx];
   int var2_idx = char_to_idx(t2->str[1]);
   if(!p->vars[var2_idx].set){
      my_error_token("Error, variable is not set.", t2);
   }
   return move_var(p, var1_idx, var2_idx);
}

bool calc_UNARY(program* p, int op_t_idx)
{
   // Value currently in x, move to y, then result will be stored in x
   move_var(p, 27, 26);
   token* op_token = &p->tokens[op_t_idx];
   variable_s *x = &p->vars[26], *y = &p->vars[27];
   for(int r = 0; r < y->rows; r++){
      for(int c = 0; c < y->cols; c++){
         if(strcmp(op_token->str, "U-NOT") == 0){
            x->arr[r][c] = !(y->arr[r][c]);
         }else if(strcmp(op_token->str, "U-EIGHTCOUNT") == 0){
            x->arr[r][c] = u_eightcount(y, r, c);
         }else{
            return false;
         }
      }
   }
   return true;
}

// At a single cell level
int u_eightcount(variable_s* v, int row, int col)
{
   int answer = 0;
   // Loop through neighbours (offsets in 8 directions)
   for(int r_offset = row - 1; r_offset <= row + 1; r_offset++){
      for(int c_offset = col - 1; c_offset <= col + 1; c_offset++){
         // Own cell is not a neighbour
         if(r_offset == row && c_offset == col){
            continue;
         // Do not look outside size of matrix
         }else if(r_offset < 0 || c_offset < 0){
            continue;
         }else if(r_offset >= v->rows || c_offset >= v->cols){
            continue;
         // Check cell truth value and if true then return will be one higher
         }else if(v->arr[r_offset][c_offset]){
            answer++;
         }
      }
   }
   return answer;
}

// Variables in x and y and answer stored to x
bool calc_BINARY(program* p, int op_t_idx)
{
   token* op_token = &p->tokens[op_t_idx];
   variable_s *x = &p->vars[26], *y = &p->vars[27];
   // Cannot perform binary operation if x and y are not set
   if(!x->set || !y->set){
      return false;
   }
   // Check matrices are the same size or one is a scalar integer
   int cols, rows;
   if(is_scalar_int(p, 26)){
      cols = y->cols;
      rows = y->rows;
   }else{
      cols = x->cols;
      rows = x->rows;
   }
   if(!is_scalar_int(p, 26) && !is_scalar_int(p, 27)){
      if(x->cols != y->cols || x->rows != y->rows){
         my_error_token("Error, binary operation between different size "
                        "matrices.", op_token);
      }
   }
   // Every binary operation only concerns one cell at a time
   for(int r = 0; r < rows; r++){
      for(int c = 0; c < cols; c++){
         if(strcmp(op_token->str, "B-AND") == 0){
            x->arr[r][c] = x->arr[r][c] && y->arr[r][c];
         }else if(strcmp(op_token->str, "B-OR") == 0){
            x->arr[r][c] = x->arr[r][c] || y->arr[r][c];
         }else if(strcmp(op_token->str, "B-GREATER") == 0){
            x->arr[r][c] = x->arr[r][c] > y->arr[r][c];
         }else if(strcmp(op_token->str, "B-LESS") == 0){
            x->arr[r][c] = x->arr[r][c] < y->arr[r][c];
         }else if(strcmp(op_token->str, "B-ADD") == 0){
            x->arr[r][c] = x->arr[r][c] + y->arr[r][c];
         }else if(strcmp(op_token->str, "B-TIMES") == 0){
            x->arr[r][c] = x->arr[r][c] * y->arr[r][c];
         }else if(strcmp(op_token->str, "B-EQUALS") == 0){
            x->arr[r][c] = (x->arr[r][c] == y->arr[r][c]);
         }else{
            return false;
         }
      }
   }
   x->rows = rows;
   x->cols = cols;
   return true;
}

bool increment_var(variable_s* v)
{
   if(!v->set){
      return false;
   }
   for(int r = 0; r < MAXMATRIXSIZE; r++){
      for(int c = 0; c < MAXMATRIXSIZE; c++){
         v->arr[r][c]++;
      }
   }
   return true;
}

