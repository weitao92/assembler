#include "table.h"
#ifndef PRINTER_H
#define PRINTER_H
/**
This file is responsible for the printing of instructions.
**/

void printR(ParseResult* result, FILE *out);

void printI(ParseResult* result, FILE *out);

void printSRA(ParseResult* result, FILE *out);

void printJ(ParseResult* result, FILE *out);

void printBLEZ(ParseResult* result, FILE *out);

void printLUI(ParseResult* result, FILE *out);

void printNOP(FILE *out);

void printSYS(FILE *out);

#endif