# Test makefile was adapted from one provided by Harry Field at the beginning of COMSM1201
CC= gcc
COMMON= -Wall -Wextra -Wfloat-equal -Wvla -pedantic -std=c99 #-Werror
DEBUG= -g3
SANITIZE= $(COMMON) -fsanitize=undefined -fsanitize=address $(DEBUG)
VALGRIND= $(COMMON) $(DEBUG)
PRODUCTION= $(COMMON) -O2
LDLIBS= -lm
CFLAGS= $(COMMON)

all: test parse interp

test: testing testing_s testing_v

testing: nlab.c lexer.c parser.c interpreter.c testing.c
	$(CC) nlab.c lexer.c parser.c interpreter.c testing.c $(PRODUCTION) -o testing -lm

testing_s: nlab.c lexer.c parser.c interpreter.c testing.c
	$(CC) nlab.c lexer.c parser.c interpreter.c testing.c $(SANITIZE) -o testing_s -lm

testing_v: nlab.c lexer.c parser.c interpreter.c testing.c
	$(CC) nlab.c lexer.c parser.c interpreter.c testing.c $(VALGRIND) -o testing_v -lm

parse: nlab.c lexer.c parser.c interpreter.c file_driver.c
	$(CC) nlab.c lexer.c parser.c interpreter.c file_driver.c $(PRODUCTION) -o parse -lm

interp: nlab.c lexer.c parser.c interpreter.c file_driver.c
	$(CC) nlab.c lexer.c parser.c interpreter.c file_driver.c $(PRODUCTION) -DLETSINTERP -o interp -lm

clean:
	rm testing testing_s testing_v parse interp

# For all .nlb error files, run them to compare file to the expected errors
errors:
	cat Error_Files/token.nlb; ./interp Error_Files/token.nlb; printf "\n"; \
	cat Error_Files/token2.nlb; ./interp Error_Files/token2.nlb; printf "\n"; \
    cat Error_Files/token3.nlb; ./interp Error_Files/token3.nlb; printf "\n"; \
    cat Error_Files/token4.nlb; ./interp Error_Files/token4.nlb; printf "\n"; \
	cat Error_Files/POLISH.nlb; ./interp Error_Files/POLISH.nlb; printf "\n"; \
    cat Error_Files/missingarr.nlb; ./interp Error_Files/missingarr.nlb; printf "\n"; \
    cat Error_Files/fileformat.nlb; ./interp Error_Files/fileformat.nlb; printf "\n"; \
    cat Error_Files/endbrace.nlb; ./interp Error_Files/endbrace.nlb; printf "\n"; \
    cat Error_Files/endbrace2.nlb; ./interp Error_Files/endbrace2.nlb; printf "\n"; \
    cat Error_Files/BEGIN1.nlb; ./interp Error_Files/BEGIN1.nlb; printf "\n"; \
    cat Error_Files/BEGIN2.nlb; ./interp Error_Files/BEGIN2.nlb; printf "\n"; \
    cat Error_Files/notINSTRUC.nlb; ./interp Error_Files/notINSTRUC.nlb; printf "\n"; \
    cat Error_Files/unmatchedrbrace.nlb; ./interp Error_Files/unmatchedrbrace.nlb; printf "\n"; \
    cat Error_Files/print.nlb; ./interp Error_Files/print.nlb; printf "\n"; \
    cat Error_Files/SETVARNAME.nlb; ./interp Error_Files/SETVARNAME.nlb; printf "\n"; \
    cat Error_Files/SETassign.nlb; ./interp Error_Files/SETassign.nlb; printf "\n"; \
    cat Error_Files/ONESrow.nlb; ./interp Error_Files/ONESrow.nlb; printf "\n"; \
    cat Error_Files/ONEScol.nlb; ./interp Error_Files/ONEScol.nlb; printf "\n"; \
    cat Error_Files/ONESvar.nlb; ./interp Error_Files/ONESvar.nlb; printf "\n"; \
    cat Error_Files/READfile.nlb; ./interp Error_Files/READfile.nlb; printf "\n"; \
    cat Error_Files/READvarname.nlb; ./interp Error_Files/READvarname.nlb; printf "\n"; \
    cat Error_Files/LOOPvarname.nlb; ./interp Error_Files/LOOPvarname.nlb; printf "\n"; \
    cat Error_Files/LOOPint.nlb; ./interp Error_Files/LOOPint.nlb; printf "\n"; \
    cat Error_Files/LOOPlbrace.nlb; ./interp Error_Files/LOOPlbrace.nlb; printf "\n"; \
    cat Error_Files/SETempty.nlb; ./interp Error_Files/SETempty.nlb; printf "\n"; \
    cat Error_Files/SETfirst.nlb; ./interp Error_Files/SETfirst.nlb; printf "\n"; \
    cat Error_Files/UNARYOP.nlb; ./interp Error_Files/UNARYOP.nlb; printf "\n"; \
    cat Error_Files/BINARYOP.nlb; ./interp Error_Files/BINARYOP.nlb; printf "\n"; \
    cat Error_Files/READfile.nlb; ./interp Error_Files/READfile.nlb; printf "\n"; \
    cat Error_Files/LOOPvar.nlb; ./interp Error_Files/LOOPvar.nlb; printf "\n"; \
    cat Error_Files/VARprint.nlb; ./interp Error_Files/VARprint.nlb; printf "\n"; \
    cat Error_Files/VARcopy.nlb; ./interp Error_Files/VARcopy.nlb; printf "\n"; \
    cat Error_Files/BINARYOPsize.nlb; ./interp Error_Files/BINARYOPsize.nlb; printf "\n";

