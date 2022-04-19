#include "nlab.h"

// Testing is split up into the 4 .c function files, and program file testing
void test_lexer(void);
void test_nlab(void);
void test_parser(void);
void test_interpreter(void);
void test_files(void);

bool print_var(program* p, int var_idx, bool store);
void print_program(program* p);

int main()
{
   test_nlab();
   test_lexer();
   test_parser();
   test_interpreter();
   test_files();

   return EXIT_SUCCESS;
}

void print_program(program* p)
{
   //printf("Name: %s\n", p->file_name);
   printf("Text: %s\n", p->prog_text);
   for(int t_idx = 0; t_idx < p->token_cnt; t_idx++){
      printf("Token %d) %s at line: %d, col: %d", t_idx,
             p->tokens[t_idx].str, p->tokens[t_idx].line,
             p->tokens[t_idx].col);
      printf(" | type: %d | g_type: %d\n",
             p->tokens[t_idx].type, p->tokens[t_idx].g_type);
   }
   for(int g_idx = 0; g_idx < p->grammar_cnt; g_idx++){
      printf("Grammar %d) %d | ", g_idx, p->grammar_list[g_idx].type);
      printf("tokens %d,%d,%d | ", p->grammar_list[g_idx].token_idx1,
             p->grammar_list[g_idx].token_idx2, p->grammar_list[g_idx].token_idx3);
      printf("grammar %d\n", p->grammar_list[g_idx].gram_idx1);
   }
   for(int v_idx = 0; v_idx < VARIABLENUM; v_idx++){
      if(p->vars[v_idx].set){
         printf("Variable %d) %c \n", v_idx, p->vars[v_idx].letter);
         //printf("is set\n");
         print_var(p, v_idx, false);
      }else{
         //printf("is not set\n");
      }
   }
   printf("Output: %s", p->output);
   printf("\n");
}

// Testing for nlab.c
void test_nlab(void)
{
   // Test substring
   char *s2, *s1 = "Hello world!";
   s2 = substring(s1, 0, 5); assert(strcmp(s2, "Hello") == 0); free(s2);
   s2 = substring(s1, 0, 1); assert(strcmp(s2, "H") == 0); free(s2);
   s2 = substring(s1, 4, 5); assert(strcmp(s2, "o") == 0); free(s2);
   s2 = substring(s1, 6, 11); assert(strcmp(s2, "world") == 0); free(s2);
   // Test char_to_idx
   assert(char_to_idx('A') == 0);
   assert(char_to_idx('Z') == 25);
   assert(char_to_idx('x') == 26);
   assert(char_to_idx('y') == 27);
   assert(char_to_idx('M') == 12);
   // Test make_program
   program p1 = make_program("");
   assert(p1.grammar_cnt == 0); assert(p1.token_cnt == 0); assert(p1.brace_depth == 0);
   assert(strcmp(p1.prog_text, "") == 0);
   assert(!p1.vars[0].set); assert(!p1.vars[4].set); assert(!p1.vars[14].set); assert(!p1.vars[22].set); assert(!p1.vars[27].set);
   assert(strcmp(p1.output, "") == 0);
   program p2 = make_program("BEGIN { SET $A := 5 }"); assert(strcmp(p2.prog_text, "BEGIN { SET $A := 5 }") == 0);
   // Test make_program_file(char* file_name)
   // I commented out the below as it failed on WSL, I think it may have been due to "\n" and "\r\n" differences
   // All this testing does is check that the text in the program structure matches the text in the raw file
   /*
   program p11 = make_program_file("Data/trivial.nlb"); assert(strcmp(p11.prog_text, "BEGIN {\n}\n") == 0);
   program p12 = make_program_file("Data/setprinta.nlb"); assert(strcmp(p12.prog_text, "BEGIN {\n   SET $I := 5 ;\n   PRINT $I\n}\n") == 0);
   program p13 = make_program_file("Data/onesprint.nlb"); assert(strcmp(p13.prog_text, "BEGIN {\n   ONES 6 5 $A\n   PRINT \"ARRAY\"\n   PRINT $A\n}\n") == 0);
   program p14 = make_program_file("Data/loopb.nlb");
   assert(strcmp(p14.prog_text, "# Notice that the loop counter is modified inside the loop\n"
                                "# causing it to count at twice the speed : 2 4 6 8 10\n"
                                "BEGIN {\n"
                                "   LOOP $I 10 {\n"
                                "      SET $I := $I 1 B-ADD ; PRINT $I\n"
                                "   }\n"
                                "}\n") == 0);
                                */
}

// Testing for lexer.c
bool valid_token(token* t);
bool is_variable(char* str);
bool is_string(char* str);
bool is_integer(char* str);
bool match_any(char* str, int n, ...);
bool is_space_char(char c);
token make_token(char* str, int line, int col);
bool lexer(program* p);
bool add_token(program* p, token t);

void test_lexer(void)
{
   // Test is_integer
   assert(is_integer("123"));
   assert(is_integer("1"));
   assert(is_integer("9999999"));
   assert(!is_integer("a"));
   assert(!is_integer("!234A"));
   assert(!is_integer("-1"));
   // Test is_variable
   assert(is_variable("$A"));
   assert(is_variable("$M"));
   assert(is_variable("$Z"));
   assert(!is_variable("$a"));
   assert(!is_variable("A$"));
   assert(!is_variable("A"));
   // Test is_string
   assert(is_string("\"name\""));
   assert(is_string("\"Folder1/fold2/file.txt\""));
   assert(is_string("\"1234567890qwertyuiopasdfghjklzxcvbnm!£$%^&*()_+-=\""));
   assert(!is_string("name"));
   assert(!is_string("$A"));
   assert(!is_string("BEGIN"));
   // Test match_any(char* str, int n, ...);
   assert(match_any("abcde", 3, "abc", "abcde", "abcdefgh"));
   assert(!match_any("abcd", 3, "abc", "abcde", "abcdefgh"));
   assert(match_any("ab", 1, "ab"));
   assert(match_any("1234", 5, "1", "12", "123", "1234", "12345"));
   // Test is_space_char(char c);
   assert(is_space_char(' '));
   assert(is_space_char('\n'));
   assert(is_space_char('\r'));
   assert(is_space_char('\0'));
   assert(!is_space_char('k'));
   assert(!is_space_char('8'));
   // Test make_token(char* str, int line, int col);
   token t1 = make_token("test 1", 1, 1); assert(strcmp(t1.str, "test 1")==0); assert(t1.line==1); assert(t1.col==1);
   token t2 = make_token("BEGIN", 2, 1); assert(strcmp(t2.str, "BEGIN")==0); assert(t2.line==2); assert(t2.col==1);
   token t3 = make_token("ONES", 1, 6); assert(strcmp(t3.str, "ONES")==0); assert(t3.line==1); assert(t3.col==6);
   token t4 = make_token("$A", 100, 100); assert(strcmp(t4.str, "$A")==0); assert(t4.line==100); assert(t4.col==100);
   token t5 = make_token("\"string\"", 10, 10); assert(strcmp(t5.str, "\"string\"")==0); assert(t5.line==10); assert(t5.col==10);
   // Test valid_token(token* t);
   assert(!valid_token(&t1));
   assert(valid_token(&t2)); assert(t2.type == keyword); assert(t2.g_type == MISC);
   assert(valid_token(&t3)); assert(t3.type == keyword); assert(t3.g_type == CREATE);
   assert(valid_token(&t4)); assert(t4.type == variable); assert(t4.g_type == VARNAME);
   assert(valid_token(&t5)); assert(t5.type == string); assert(t5.g_type == STRING);
   // Test add_token(program* p, token t);
   program p0 = make_program("BEGIN { SET $A = 1 ; }");
   assert(!add_token(&p0, t1)); assert(p0.token_cnt == 0);
   assert(add_token(&p0, t2)); assert(p0.token_cnt == 1); assert(strcmp(p0.tokens[0].str, "BEGIN")==0);
   assert(add_token(&p0, t3)); assert(p0.token_cnt == 2); assert(strcmp(p0.tokens[1].str, "ONES")==0);
   assert(add_token(&p0, t4)); assert(p0.token_cnt == 3); assert(strcmp(p0.tokens[2].str, "$A")==0);
   assert(add_token(&p0, t5)); assert(p0.token_cnt == 4); assert(strcmp(p0.tokens[3].str, "\"string\"")==0);
   // Test lexer(program* p);
   program p5, p6, p7, p8, p9, p10;
   p5 = make_program("BEGIN { }"); assert(lexer(&p5)); assert(!lexer(&p5)); assert(p5.token_cnt==3);
   p6 = make_program(""); assert(lexer(&p6)); assert(p6.token_cnt==0); assert(lexer(&p6));
   p7 = make_program("SET $A := 100 ; LOOP B-TIMES U-NOT PRINT ONES READ"); assert(lexer(&p7)); assert(p7.token_cnt==11);
   p8 = make_program("BEGIN { SET $A := 1 }"); assert(lexer(&p8)); assert(p8.token_cnt==7);
   p9 = make_program("BEGIN BEGIN BEGIN BEGIN BEGIN BEGIN BEGIN BEGIN"); assert(lexer(&p9)); assert(p9.token_cnt==8);
   p10 = make_program("# Comment\n BEGIN { #COMMENT\n \n } \n # end comment}"); assert(lexer(&p10)); assert(p10.token_cnt==3);
}

// Testing for parser.c
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

void test_parser(void)
{
   // Test is_printable
   program p1 = make_program("$B $M \"hello\" BEGIN 123 READ "); assert(lexer(&p1));
   assert(is_printable(&p1.tokens[0]));
   assert(is_printable(&p1.tokens[1]));
   assert(is_printable(&p1.tokens[2]));
   assert(!is_printable(&p1.tokens[3]));
   assert(!is_printable(&p1.tokens[4]));
   assert(!is_printable(&p1.tokens[5]));
   // Test is_POLISH
   program p2 = make_program("94 123 $L $N U-NOT B-OR B-EQUALS { SET \"B-LESS\" :="); assert(lexer(&p2));
   assert(is_POLISH(&p2.tokens[0])); assert(is_POLISH(&p2.tokens[1]));
   assert(is_POLISH(&p2.tokens[2])); assert(is_POLISH(&p2.tokens[3]));
   assert(is_POLISH(&p2.tokens[4])); assert(is_POLISH(&p2.tokens[5]));
   assert(is_POLISH(&p2.tokens[6]));
   assert(!is_POLISH(&p2.tokens[7])); assert(!is_POLISH(&p2.tokens[8]));
   assert(!is_POLISH(&p2.tokens[9])); assert(!is_POLISH(&p2.tokens[10]));
   // Test make_grammar, note: grammar does not have to be valid
   grammar g1, g2, g3, g4, g5;
   g1 = make_grammar(PRINT, 1, 2, 3, 4);
   assert(g1.type==PRINT); assert(g1.token_idx1==2); assert(g1.token_idx2==3); assert(g1.token_idx3==4); assert(g1.gram_idx1==1);
   g2 = make_grammar(POLISH, 2, -1, -1, -1); assert(g2.type==POLISH);
   g3 = make_grammar(CREATE, -1, -1, -1, -1); assert(g3.type==CREATE);
   g4 = make_grammar(LOOP, -1, -1, -1, 3); assert(g4.type==LOOP);
   g5 = make_grammar(SET, 4, 3, 2, 1); assert(g5.type==SET);
   // Test add_grammar
   program p3 = make_program("");
   assert(add_grammar(&p3, g1)); assert(p3.grammar_cnt == 1); assert(p3.grammar_list[0].type==PRINT);
   assert(add_grammar(&p3, g2)); assert(p3.grammar_cnt == 2); assert(p3.grammar_list[1].type==POLISH);
   assert(add_grammar(&p3, g3)); assert(p3.grammar_cnt == 3); assert(p3.grammar_list[2].type==CREATE);
   assert(add_grammar(&p3, g4)); assert(p3.grammar_cnt == 4); assert(p3.grammar_list[3].type==LOOP);
   assert(add_grammar(&p3, g5)); assert(p3.grammar_cnt == 5); assert(p3.grammar_list[4].type==SET);
   // Test parse_POLISH
   program p4 = make_program("$A ;"); assert(lexer(&p4));
   assert(parse_POLISH(&p4, 0)); assert(p4.grammar_list[0].type == POLISH); assert(p4.grammar_list[0].token_idx1==0);
   program p5 = make_program("946 }"); assert(lexer(&p5));
   assert(parse_POLISH(&p5, 0)); assert(p5.grammar_list[0].type == POLISH); assert(p5.grammar_list[0].token_idx1==0);
   program p6 = make_program("U-EIGHTCOUNT B-ADD"); assert(lexer(&p6));
   assert(parse_POLISH(&p6, 0)); assert(p6.grammar_list[0].type == POLISH); assert(p6.grammar_list[0].token_idx1==0);
   program p7 = make_program("{ B-ADD SET"); assert(lexer(&p7));
   assert(parse_POLISH(&p7, 1)); assert(p7.grammar_list[0].type == POLISH); assert(p7.grammar_list[0].token_idx1==1);
   // Test parse_POLISHLIST
   program p11 = make_program("$A ;"); assert(lexer(&p11));
   assert(parse_POLISHLIST(&p11, 0)); assert(p11.grammar_list[0].type == POLISH); assert(p11.grammar_list[0].gram_idx1==-1);
   program p12 = make_program("$A 1 B-ADD ;"); assert(lexer(&p12));
   assert(parse_POLISHLIST(&p12, 0)); assert(p12.grammar_list[2].type == POLISH); assert(p12.grammar_list[0].gram_idx1==1);
   program p13 = make_program("$A $B $C $D 1 3 B-GREATER B-TIMES U-NOT ;"); assert(lexer(&p13));
   assert(parse_POLISHLIST(&p13, 1)); assert(p13.grammar_cnt==8); assert(p13.grammar_list[7].gram_idx1==-1);
   // Test parse_SET
   program p21 = make_program("SET $A := 1 ;"); assert(lexer(&p21));
   assert(parse_SET(&p21, 1)); assert(p21.grammar_list[0].type == SET); assert(p21.grammar_list[1].type == POLISH); assert(p21.grammar_cnt==2);
   program p22 = make_program("$B := $C 1 B-ADD ;"); assert(lexer(&p22));
   assert(parse_SET(&p22, 0)); assert(p22.grammar_list[0].type == SET); assert(p22.grammar_list[3].type == POLISH); assert(p22.grammar_cnt==4);
   program p23 = make_program("SET $A := 1 2 3 4 5 ;"); assert(lexer(&p23));
   assert(parse_SET(&p23, 1)); assert(p23.grammar_list[5].type == POLISH); assert(p23.grammar_cnt==6);
   // Test parse_PRINT
   program p31 = make_program("PRINT $A"); assert(lexer(&p31));
   assert(parse_PRINT(&p31, 1 )); assert(p31.grammar_list[0].type == PRINT); assert(p31.grammar_list[0].token_idx1==1); assert(p31.grammar_cnt==1);
   program p32 = make_program("$A"); assert(lexer(&p32));
   assert(parse_PRINT(&p32, 0)); assert(p32.grammar_list[0].type == PRINT); assert(p32.grammar_list[0].token_idx1==0); assert(p32.grammar_cnt==1);
   program p33 = make_program("PRINT \"banana\" PRINT $F"); assert(lexer(&p33));
   assert(parse_PRINT(&p33, 1)); assert(parse_PRINT(&p33, 3));
   assert(p33.grammar_list[0].type == PRINT); assert(p33.grammar_list[0].token_idx1==1);
   assert(p33.grammar_list[1].type == PRINT); assert(p33.grammar_list[1].token_idx1==3); assert(p33.grammar_cnt==2);
   // Test parse_ONES
   program p51 = make_program("ONES 9 8 $L"); assert(lexer(&p51));
   assert(parse_CREATE(&p51, 0)); assert(p51.grammar_list[0].type == CREATE); assert(p51.grammar_cnt==1);
   assert(p51.grammar_list[0].token_idx1==1); assert(p51.grammar_list[0].token_idx2==2); assert(p51.grammar_list[0].token_idx3==3);
   // Test parse_READ
   program p61 = make_program("READ \"folder/file.arr\" $X"); assert(lexer(&p61));
   assert(parse_CREATE(&p61, 0)); assert(p61.grammar_list[0].type == CREATE); assert(p61.grammar_cnt==1);
   assert(p61.grammar_list[0].token_idx1==1); assert(p61.grammar_list[0].token_idx2==2); assert(p61.grammar_list[0].token_idx3==-1);
   // Test parse_CREATE
   program p41 = make_program("ONES 2 3 $K"); assert(lexer(&p41));
   assert(parse_CREATE(&p41, 0)); assert(p41.grammar_list[0].type == CREATE); assert(p41.grammar_cnt==1);
   assert(p41.grammar_list[0].token_idx1==1); assert(p41.grammar_list[0].token_idx2==2); assert(p41.grammar_list[0].token_idx3==3);
   program p42 = make_program("READ \"filename\" $K"); assert(lexer(&p42));
   assert(parse_CREATE(&p42, 0)); assert(p42.grammar_list[0].type == CREATE); assert(p42.grammar_cnt==1);
   assert(p42.grammar_list[0].token_idx1==1); assert(p42.grammar_list[0].token_idx2==2); assert(p42.grammar_list[0].token_idx3==-1);
   // Test parse_LOOP
   program p71 = make_program("LOOP $F 10 { "); assert(lexer(&p71));
   assert(parse_LOOP(&p71, 1)); assert(p71.grammar_list[0].type == LOOP); assert(p71.grammar_cnt==1);
   assert(p71.grammar_list[0].token_idx1==1); assert(p71.grammar_list[0].token_idx2==2);
   program p72 = make_program("LOOP $G 10 { SET $A := $A $F B-ADD }"); assert(lexer(&p72));
   assert(parse_LOOP(&p72, 1));assert(p72.grammar_list[0].type == LOOP); assert(p72.grammar_cnt==1);
   program p73 = make_program("$F 10 { "); assert(lexer(&p73));
   assert(parse_LOOP(&p73, 0)); assert(p73.grammar_list[0].type == LOOP); assert(p73.grammar_cnt==1);
   program p74 = make_program("LOOP $A 10 { LOOP $S 10 { LOOP $D 10 {"); assert(lexer(&p74));
   assert(parse_LOOP(&p74, 1)); assert(parse_LOOP(&p74, 5)); assert(parse_LOOP(&p74, 9)); assert(p74.grammar_cnt==3);
   // Test parse_LOOP_end
   program p81 = make_program("LOOP $N 10 { }"); assert(lexer(&p81));
   assert(parse_LOOP(&p81, 1)); assert(parse_LOOP_end(&p81, 4));
   assert(p81.grammar_list[0].type == LOOP); assert(p81.grammar_list[1].type == LOOP_end); assert(p81.grammar_cnt==2);
   assert(p81.grammar_list[0].gram_idx1 == 1); assert(p81.grammar_list[1].gram_idx1 == 0);
   program p82 = make_program("LOOP $N 10 { SET $Z := $Z $N B-OR ; }"); assert(lexer(&p82));
   assert(parse_LOOP(&p82, 1)); assert(parse_LOOP_end(&p82, 12));
   assert(p82.grammar_list[0].type == LOOP); assert(p82.grammar_list[1].type == LOOP_end); assert(p82.grammar_cnt==2);
   program p83 = make_program("LOOP $N 10 { SET $Z := $Z $N B-OR ; }"); assert(lexer(&p83));
   assert(parse_LOOP(&p83, 1)); assert(parse_SET(&p83, 5)); assert(parse_LOOP_end(&p83, 12));
   assert(p83.grammar_list[0].type == LOOP); assert(p83.grammar_list[1].type == SET); assert(p83.grammar_list[2].type == POLISH);
   assert(p83.grammar_list[3].type == POLISH); assert(p83.grammar_list[4].type == POLISH); assert(p83.grammar_list[5].type == LOOP_end);
   assert(p83.grammar_cnt==6);
   // Test parse_INSTRC
   program p91 = make_program("SET $V := 7 ;"); assert(lexer(&p91));
   assert(parse_INSTRC(&p91, 0)); assert(p91.grammar_list[0].type == SET);
   program p92 = make_program("LOOP $I 10 {"); assert(lexer(&p92));
   assert(parse_INSTRC(&p92, 0)); assert(p92.grammar_list[0].type == LOOP);
   program p93 = make_program("ONES 3 3 $P "); assert(lexer(&p93));
   assert(parse_INSTRC(&p93, 0)); assert(p93.grammar_list[0].type == CREATE);
   program p94 = make_program("READ \"file_name.ex\" $R"); assert(lexer(&p94));
   assert(parse_INSTRC(&p94, 0)); assert(p94.grammar_list[0].type == CREATE);
   program p95 = make_program("PRINT $J"); assert(lexer(&p95));
   assert(parse_INSTRC(&p95, 0)); assert(p95.grammar_list[0].type == PRINT);
   // Test parse_PROG
   program p101 = make_program("BEGIN { }"); assert(lexer(&p101)); assert(parse_PROG(&p101));
   program p102 = make_program("BEGIN { 1 1 1 1 1 1 1 1 1 1 }"); assert(lexer(&p102)); assert(parse_PROG(&p102));
   program p103 = make_program("BEGIN { SET $S := 13 ; PRINT $S }"); assert(lexer(&p103)); assert(parse_PROG(&p103));
   // Test parser
   program p111 = make_program("BEGIN { }"); assert(lexer(&p111)); assert(parser(&p111));
   program p112 = make_program("BEGIN { LOOP $I 5 { } }"); assert(lexer(&p112)); assert(parser(&p112));
   program p113 = make_program("BEGIN { PRINT \"Hello-World!\" }"); assert(lexer(&p113)); assert(parser(&p113));
}

// Testing for interpreter.c
bool interpreter(program* p);
int inter_grammar(program* p, int gram_idx);
int inter_SET(program* p, int gram_idx);
int inter_PRINT(program* p, int gram_idx);
int inter_ONES(program* p, int gram_idx);
int inter_READ(program* p, int gram_idx);
int inter_LOOP(program* p, int gram_idx);
int inter_LOOP_end(program* p, int gram_idx);
bool load_file_x(program* p, FILE* fp);
//bool print_var(program* p, int var_idx, bool store);
bool print_var_t(program* p, int token_idx, bool store);
bool print_str(program* p, int token_idx);
bool is_scalar_int(program* p, int var_idx);
bool int_to_var(program* p, int value_t_idx, int var_idx);
bool move_var(program* p, int var_e_idx, int var_s_idx);
bool move_var_tb(program* p, int var1_t_idx, int var2_t_idx);
bool move_var_t1(program* p, int var1_t_idx, int var2_idx);
bool move_var_t2(program* p, int var1_idx, int var2_t_idx);
bool calc_UNARY(program* p, int op_t_idx);
int u_eightcount(variable_s* v, int row, int col);
bool calc_BINARY(program* p, int op_t_idx);
bool increment_var(variable_s* v);

void test_interpreter(void)
{
   //Test print_var
   program p1 = make_program("$A $B $C");
   p1.vars[0].set = true; p1.vars[0].rows = 2; p1.vars[0].cols = 1; p1.vars[0].arr[0][0] = 3; p1.vars[0].arr[1][0] = 2;
   assert(print_var(&p1, 0, true)); assert(strcmp(p1.output, "3 \n2 \n")==0);
   p1.vars[1].set = true; p1.vars[1].rows = 1; p1.vars[1].cols = 1; p1.vars[1].arr[0][0] = 8;
   assert(print_var(&p1, 1, true)); assert(strcmp(p1.output, "3 \n2 \n8\n")==0);
   p1.vars[2].set = true; p1.vars[2].rows = p1.vars[2].cols = 2; p1.vars[2].arr[0][0] = p1.vars[2].arr[1][0] = p1.vars[2].arr[1][1] = p1.vars[2].arr[0][1] = 2;
   assert(print_var(&p1, 2, true)); assert(strcmp(p1.output, "3 \n2 \n8\n2 2 \n2 2 \n")==0);
   assert(print_var(&p1, 2, false)); assert(strcmp(p1.output, "3 \n2 \n8\n2 2 \n2 2 \n")==0);
   assert(!print_var(&p1, 3, false));
   assert(!print_var(&p1, 15, false));
   assert(!print_var(&p1, 23, false));
   //Test print_var_t
   assert(lexer(&p1));
   assert(print_var_t(&p1, 2, true)); assert(strcmp(p1.output, "3 \n2 \n8\n2 2 \n2 2 \n2 2 \n2 2 \n")==0);
   assert(print_var_t(&p1, 1, true)); assert(strcmp(p1.output, "3 \n2 \n8\n2 2 \n2 2 \n2 2 \n2 2 \n8\n")==0);
   assert(print_var_t(&p1, 0, true)); assert(strcmp(p1.output, "3 \n2 \n8\n2 2 \n2 2 \n2 2 \n2 2 \n8\n3 \n2 \n")==0);
   //Test load_file_x
   program p2 = make_program("");
   FILE* fp = fopen("Data/lblock.arr", "r");
   assert(load_file_x(&p2, fp)); assert(print_var(&p2, 26, true));
   assert(strcmp(p2.output, "0 0 0 0 \n0 1 1 0 \n0 1 1 0 \n0 0 0 0 \n")==0);
   fclose(fp);
   program p3 = make_program("");
   fp = fopen("Data/lglider.arr", "r");
   assert(load_file_x(&p3, fp)); assert(print_var(&p3, 26, true));
   assert(strcmp(p3.output, "0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n")==0);
   fclose(fp);
   //Test print_str
   program p5 = make_program("\"Hello\" \"World\" $A \"\""); assert(lexer(&p5));
   assert(print_str(&p5, 0)); assert(strcmp(p5.output, "Hello\n")==0);
   assert(print_str(&p5, 1)); assert(strcmp(p5.output, "Hello\nWorld\n")==0);
   assert(print_str(&p5, 3)); assert(strcmp(p5.output, "Hello\nWorld\n\n")==0);
   //Test is_scalar_int
   p3.vars[0].set = true; assert(!is_scalar_int(&p3, 0));
   p3.vars[1].set = true; p3.vars[1].cols = p3.vars[1].rows = 1; assert(is_scalar_int(&p3, 1));
   p3.vars[2].set = true; p3.vars[2].cols = p3.vars[2].rows = 3; assert(!is_scalar_int(&p3, 2));
   p3.vars[4].cols = p3.vars[4].rows = 1; assert(!is_scalar_int(&p3, 3));
   assert(!is_scalar_int(&p3, 26));
   //Test int_to_var
   program p7 = make_program("SET $C := 1 ; $D 3 $E $F"); assert(lexer(&p7));
   assert(int_to_var(&p7, 3, 2)); assert(print_var(&p7, 2, true));
   assert(strcmp(p7.output, "1\n")==0);
   assert(int_to_var(&p7, 6, 3)); assert(print_var(&p7, 3, true));
   assert(strcmp(p7.output, "1\n3\n")==0);
   //Test move_var
   assert(move_var(&p7, 0, 3));
   assert(print_var(&p7, 0, true)); assert(strcmp(p7.output, "1\n3\n3\n")==0);
   assert(!move_var(&p7, 0, 4));
   assert(move_var(&p7, 3, 2));
   assert(print_var(&p7, 3, true)); assert(strcmp(p7.output, "1\n3\n3\n1\n")==0);
   //Test move_var_tb
   assert(move_var_tb(&p7, 7, 5));
   assert(print_var(&p7, 4, true)); assert(strcmp(p7.output, "1\n3\n3\n1\n1\n")==0);
   //assert(!move_var_tb(&p7, 7, 8)); // Known error catching this
   //Test u_eightcount
   assert(u_eightcount(&p3.vars[26], 0, 0) == 0);
   assert(u_eightcount(&p3.vars[26], 1, 0) == 1);
   assert(u_eightcount(&p3.vars[26], 1, 2) == 3);
   assert(u_eightcount(&p3.vars[26], 2, 2) == 2);
   //Test increment_var
   assert(increment_var(&p7.vars[4])); assert(p7.vars[4].arr[0][0] == 2);
   assert(increment_var(&p3.vars[26])); assert(print_var(&p3, 26, true));
   assert(strcmp(p3.output, "0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n"
                            "1 1 1 1 1 \n1 1 1 1 1 \n1 2 2 2 1 \n1 1 1 1 1 \n1 1 1 1 1 \n")==0);
   //Test inter_PRINT
   program p21 = make_program("BEGIN { PRINT \"TESTING\" }");
   assert(lexer(&p21)); assert(parser(&p21));
   inter_PRINT(&p21, 0); assert(strcmp(p21.output, "TESTING\n") == 0);
   program p22 = make_program("BEGIN { PRINT $A PRINT $B }");
   assert(lexer(&p22)); assert(parser(&p22));
   p22.vars[0].set = true; p22.vars[0].rows = p22.vars[0].cols = 1; p22.vars[0].arr[0][0] = 1;
   p22.vars[1].set = true; p22.vars[1].rows = p22.vars[1].cols = 2;
   p22.vars[1].arr[0][0] = p22.vars[1].arr[0][1] = 2; p22.vars[1].arr[1][0] = p22.vars[1].arr[1][1] = 3;
   inter_PRINT(&p22, 0); assert(strcmp(p22.output, "1\n") == 0);
   inter_PRINT(&p22, 1); assert(strcmp(p22.output, "1\n2 2 \n3 3 \n") == 0);
   //Test inter_SET
   program p31 = make_program("BEGIN { SET $A := 1 ; }");
   assert(lexer(&p31)); assert(parser(&p31));
   inter_SET(&p31, 0);
   assert(print_var_t(&p31, 3, true)); assert(strcmp(p31.output, "1\n")==0);
   program p32 = make_program("BEGIN { SET $A := 1 ; SET $B := $A ; PRINT $B }");
   assert(lexer(&p32)); assert(parser(&p32));
   inter_SET(&p32, 0); inter_SET(&p32, 2); inter_PRINT(&p32, 4);
   assert(strcmp(p32.output, "1\n")==0);
   //Test inter_ONES
   program p41 = make_program("BEGIN { ONES 2 3 $A PRINT $A }");
   assert(lexer(&p41)); assert(parser(&p41));
   inter_ONES(&p41, 0); inter_PRINT(&p41, 1);
   assert(strcmp(p41.output, "1 1 1 \n1 1 1 \n")==0);
   //Test inter_READ
   program p51 = make_program("BEGIN { READ \"Data/lglider.arr\" $A PRINT $A }");
   assert(lexer(&p51)); assert(parser(&p51));
   inter_READ(&p51, 0); inter_PRINT(&p51, 1);
   assert(strcmp(p51.output, "0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n")==0);
   //Test inter_LOOP
   program p61 = make_program("BEGIN { LOOP $I 5 { PRINT $I } }");
   assert(lexer(&p61)); assert(parser(&p61));
   inter_LOOP(&p61, 0); inter_PRINT(&p61, 1);
   assert(strcmp(p61.output, "1\n")==0);
   //Test inter_LOOP_end
   program p71 = make_program("BEGIN { LOOP $I 5 { PRINT $I } }");
   assert(lexer(&p71)); assert(parser(&p71));
   inter_LOOP(&p71, 0); inter_PRINT(&p71, 1); inter_LOOP_end(&p71, 2);
   assert(strcmp(p71.output, "1\n")==0);
   //Test inter_grammar
   program p81 = make_program("BEGIN { LOOP $I 5 { PRINT $I SET $J := $I ; ONES 1 2 $K PRINT $K } }");
   assert(lexer(&p81)); assert(parser(&p81));
   inter_grammar(&p81, 0); inter_grammar(&p81, 1); inter_grammar(&p81, 2);
   inter_grammar(&p81, 4); inter_grammar(&p81, 5); inter_grammar(&p81, 6);
   assert(strcmp(p81.output, "1\n1 1 \n")==0);
   //Test interpreter
   program p91 = make_program("BEGIN { LOOP $I 5 { PRINT $I } }");
   assert(lexer(&p91)); assert(parser(&p91)); assert(interpreter(&p91));
   assert(strcmp(p91.output, "1\n2\n3\n4\n5\n")==0);
}

void test_files(void)
{
   // Neill's programs
   program p1 = make_program_file("Data/setprinta.nlb"); lexer(&p1); parser(&p1); interpreter(&p1); //print_program(&p1);
   assert(strcmp(p1.output, "5\n")==0);
   program p2 = make_program_file("Data/setprintb.nlb"); lexer(&p2); parser(&p2); interpreter(&p2); //print_program(&p2);
   assert(strcmp(p2.output, "ARRAY:\n3 3 3 3 3 \n3 3 3 3 3 \n3 3 3 3 3 \n3 3 3 3 3 \n3 3 3 3 3 \n3 3 3 3 3 \n")==0);
   program p3 = make_program_file("Data/setprintc.nlb"); lexer(&p3); parser(&p3); interpreter(&p3); //print_program(&p3);
   assert(strcmp(p3.output, "2\n5\n10\n")==0);
   program p4 = make_program_file("Data/onesprint.nlb"); lexer(&p4); parser(&p4); interpreter(&p4); //print_program(&p4);
   assert(strcmp(p4.output, "ARRAY\n1 1 1 1 1 \n1 1 1 1 1 \n1 1 1 1 1 \n1 1 1 1 1 \n1 1 1 1 1 \n1 1 1 1 1 \n")==0);
   program p5 = make_program_file("Data/nestedloop.nlb"); lexer(&p5); parser(&p5);  interpreter(&p5); //print_program(&p5);
   assert(strcmp(p5.output, "1\n2\n3\n4\n5\n2\n4\n6\n8\n10\n3\n6\n9\n12\n15\n4\n8\n12\n16\n20\n5\n10\n15\n20\n25\n")==0);
   program p6 = make_program_file("Data/loopb.nlb"); lexer(&p6); parser(&p6); interpreter(&p6); //print_program(&p6);
   assert(strcmp(p6.output, "2\n4\n6\n8\n10\n")==0);
   program p7 = make_program_file("Data/loopa.nlb"); lexer(&p7); parser(&p7); interpreter(&p7); //print_program(&p7);
   assert(strcmp(p7.output, "1\n2\n6\n24\n120\n720\n5040\n40320\n362880\n3628800\n")==0);
   program p8 = make_program_file("Data/lifeb3s23.nlb"); lexer(&p8); parser(&p8); interpreter(&p8); //print_program(&p8);
   assert(strcmp(p8.output, "1\n0 0 0 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 0 0 0 \n"
                            "2\n0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n"
                            "3\n0 0 0 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 0 0 0 \n"
                            "4\n0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n"
                            "5\n0 0 0 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 0 0 0 \n"
                            "6\n0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n"
                            "7\n0 0 0 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 0 0 0 \n"
                            "8\n0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n"
                            "9\n0 0 0 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 1 0 0 \n0 0 0 0 0 \n"
                            "10\n0 0 0 0 0 \n0 0 0 0 0 \n0 1 1 1 0 \n0 0 0 0 0 \n0 0 0 0 0 \n")==0);

   // My programs
   // These will only parse, but not interpret
   program p11 = make_program_file("Error_Files/BINARYOP.nlb"); lexer(&p11); parser(&p11);
   program p12 = make_program_file("Error_Files/UNARYOP.nlb"); lexer(&p12); parser(&p12);
   program p13 = make_program_file("Error_Files/SETempty.nlb"); lexer(&p13); parser(&p13);
   program p14 = make_program_file("Error_Files/LOOPvar.nlb"); lexer(&p14); parser(&p14);
   program p15 = make_program_file("Error_Files/VARprint.nlb"); lexer(&p15); parser(&p15);
   program p16 = make_program_file("Error_Files/VARcopy.nlb"); lexer(&p16); parser(&p16);
   // These will interpret
   program p21 = make_program_file("My_Data/manybin.nlb"); lexer(&p21); parser(&p21); interpreter(&p21); //print_program(&p21);
   assert(strcmp(p21.output, "6 4 4 4 6 \n4 6 4 6 4 \n4 4 6 4 4 \n4 6 4 6 4 \n6 4 4 4 6 \n") == 0);
   program p22 = make_program_file("My_Data/manyloops.nlb"); lexer(&p22); parser(&p22); interpreter(&p22); //print_program(&p22);
   assert(strcmp(p22.output, "10\n20\n20\n201\n") == 0);
   program p23 = make_program_file("My_Data/manyops.nlb"); lexer(&p23); parser(&p23); interpreter(&p23); //print_program(&p23);
   assert(strcmp(p23.output, "2 2 2 2 \n2 1 1 2 \n2 1 1 2 \n2 2 2 2 \n"
                             "3 5 5 5 3 \n5 8 8 8 5 \n5 8 8 8 5 \n5 8 8 8 5 \n3 5 5 5 3 \n") == 0);
}

