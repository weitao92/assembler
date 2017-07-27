#include <stdio.h>
#include "printer.h"

//print R tyle instruction
void printR(ParseResult* result, FILE *out)
{
	fprintf(out, "%6s%5s%5s%5s%5s%6s\n", result->Opcode, result->RS, result->RT, result->RD, result->SHAMT,
		result->Funct);
}

//print I tyle instruction
void printI(ParseResult* result, FILE *out)
{
	fprintf(out, "%6s%5s%5s%16s\n", result->Opcode, result->RS, result->RT, 
		result->IMM);
}

//print LUI and LW tyle instruction
void printLUI(ParseResult* result, FILE *out)
{
	fprintf(out, "%6s%5s%5s%16s\n", result->Opcode, "00000", result->RT, result->IMM);
}

//print SRA tyle instruction
void printSRA(ParseResult* result, FILE *out)
{
	fprintf(out, "%6s%5s%5s%5s%5s%6s\n", result->Opcode, "00000", result->RT, result->RD, result->SHAMT,
		result->Funct);
}

//print J instruction
void printJ(ParseResult* result, FILE *out)
{
	fprintf(out, "%6s%10s%16s\n", result->Opcode, "0000000000", result->IMM);
}

//print BLEZ tyle instruction
void printBLEZ(ParseResult* result, FILE *out)
{
	fprintf(out, "%6s%5s%5s%16s\n", result->Opcode, result-> RS, "00000", result->IMM);
}

//special case for NOP instruction.
void printNOP(FILE *out)
{
	for(int i = 0; i < 32; i++)
	{
		fprintf(out, "0");
	}
	fprintf(out, "\n");
}

//special case for sysmcall instruction.
void printSYS(FILE *out)
{
	for(int i = 0; i < 28; i++)
	{
		fprintf(out, "0");
	}
	fprintf(out, "1100\n");
}