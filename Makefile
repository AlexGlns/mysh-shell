
mysh:  mysh_main.o str_Parser.o
	cc mysh_main.o  str_Parser.o -o mysh
str_Parser.o: str_Parser.c str_Parser.h
	cc -c str_Parser.c
mysh_main.o:str_Parser.h mysh_main.c
	cc -c mysh_main.c
clean: 
	rm -f mysh\
		  str_Parser.o\
		  mysh_main.o

