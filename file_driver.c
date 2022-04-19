#include "nlab.h"

// Only need these three functions to go from program text to output
bool lexer(program* p);
bool parser(program* p);
bool interpreter(program* p);

int main(int argc, char* argv[])
{
   if(argc != 2){
      my_error("Incorrect syntax!\n"
               "Correct usage is: parse/interp file_name\n");
   }
   program p = make_program_file(argv[1]);
   lexer(&p);
   parser(&p);
   // This is defined in the Makefile
#ifdef LETSINTERP
   interpreter(&p);
#endif
   return EXIT_SUCCESS;
}