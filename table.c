#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

/**
This function free my parseresult which is a struct type i build for MIPS32 instruction.
**/
void clearResult(ParseResult* const pPR)
{
	if(pPR->Opcode != NULL)
	{
		free(pPR->Opcode);
	}
	if(pPR->RD!= NULL)
	{
		free(pPR->RD);
	}
	if(pPR->RS!= NULL)
	{
		free(pPR->RS);
	}
	if(pPR->RT!= NULL)
	{
		free(pPR->RT);
	}
	if(pPR->IMM != NULL)
	{
		free(pPR->IMM);
	}
	if(pPR->Funct!= NULL)
	{
		free(pPR->Funct);
	}
	if(pPR->SHAMT!= NULL)
	{
		free(pPR->SHAMT);
	}
}