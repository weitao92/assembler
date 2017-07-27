assembler: assembler.c preprocess.c preprocess.h printer.c printer.h label.h table.c table.h
	gcc -std=c99 -o assemble -Wall assembler.c preprocess.c printer.c table.c

