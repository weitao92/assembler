#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "table.h"
#include "printer.h"
#include "preprocess.h"
#include "label.h"

#define NUMBER 9

void parseASM(const char* const pASM, FILE *out, int* address1);

int parseWord(char* line, FILE *out, int previous);

void Rtype(const char* const pASM, ParseResult* result, FILE *out);

void Itype(const char* const pASM, ParseResult* result, FILE *out);

void SRA(const char* const pASM, ParseResult* result, FILE *out);

void LUI(const char* const pASM, ParseResult* result, FILE *out);

void LW(const char* const pASM, ParseResult* result, FILE *out);

void BNE(const char* const pASM, ParseResult* result, FILE *out);

void BLEZ(const char* const pASM, ParseResult* result, FILE *out);

void J(const char* const pASM, ParseResult* result, FILE *out);

char* createIMM(int input);

char* createSHAMT(int input);

char* createLONG(int32_t input);

void Register(ParseResult* result, char* registerTemp, int index);

void handleText(FILE* preprocessed, FILE* out, int* address);

void handleData(FILE* preprocessed, FILE* out);

void InsertLabel(labelList *list, label* lab);

int findLabel(char* line, labelList* list);

void handleDLable(FILE* preprocessed, int* address);

void handleTLabel(FILE* preprocessed, int* address);

void printbincharpad(char c, FILE *out);

//static table for registers.
static RegMapping registers[32] = {
{"zero", "00000", 0},
{"at", "00001", 1},
{"v0", "00010", 2},
{"v1", "00011", 3},
{"a0", "00100", 4},
{"a1", "00101", 5},
{"a2", "00110", 6},
{"a3", "00111", 7},
{"t0", "01000", 8},
{"t1", "01001", 9},
{"t2", "01010", 10},
{"t3", "01011", 11},
{"t4", "01100", 12},
{"t5", "01101", 13},
{"t6", "01110", 14},
{"t7", "01111", 15},
{"s0", "10000", 16},
{"s1", "10001", 17},
{"s2", "10010", 18},
{"s3", "10011", 19},
{"s4", "10100", 20},
{"s5", "10101", 21},
{"s6", "10110", 22},
{"s7", "10111", 23},
{"t8", "11000", 24},
{"t9", "11001", 25},
{"k0", "11010", 26},
{"k1", "11011", 27},
{"gp", "11100", 28},
{"sp", "11101", 29},
{"fp", "11110", 30},
{"ra", "11111", 31}
};

//table for opCodes.
static OpCode opCodes = {
"100011","101011","001111","000000","001000","001001","000000",
"001100","011100","000000","000000","001101","000000","000000",
"001010","000000","000000","000100","000110","000001","000101","000010",
};

//table for functs.
static funct functs = {
"100000","100100","000010","100111",
"100101","000000","101010","000011","100010",
};

static labelList list;

static FILE *in;
static FILE *out;
static FILE *preprocessed;
static FILE *temp;

int main( int argc, char* argv[] )
{
	char* input;
	char* output;
	bool symbolTable = false;

	if (strcmp("-symbols", argv[1]) != 0)
	{
		input = argv[1];
		output = argv[2];
	}
	else{
		input = argv[2];
		output = argv[3];
		symbolTable = true;
	}
	

	in = fopen(input, "r");
	out = fopen(output, "w");
	
	if (in == NULL || out == NULL){
		printf("input file or ouput file does not exist");
		return 0;
	}	

	preprocessed = fopen("pre.txt", "w+");

	preProcess(in, preprocessed);
	rewind(preprocessed);

	char line[256];
	fgets(line, 256, preprocessed);

	int Daddress = 8192;
	int Taddress = 0;

	if(line != NULL)//first handle the labels.
	{
			char line1[6];
			line1[0] = line[0];
			line1[1] = line[1];
			line1[2] = line[2];	
			line1[3] = line[3];
			line1[4] = line[4];
			line1[5] = '\0';



			if(strstr(line1, ".data") != NULL)//normal case.
			{
				handleDLable(preprocessed, &Daddress);
				handleTLabel(preprocessed, &Taddress);
			}
			else//case for extra credit.
			{
				handleTLabel(preprocessed, &Taddress);
				handleDLable(preprocessed, &Daddress);
			}
	}

	if(symbolTable)//print the symbol table.
	{
		int size = list.size;
		for(int i = 0; i < size; i++)
		{
			label lab = list.list[i];
			fprintf(out, "0x");
			fprintf(out, "%8s\t%s\n", lab.hex, lab.name);
		}
	}
	else
	{
		rewind(preprocessed);
		//char line[256];
		fgets(line, 256, preprocessed);
		Taddress = 0;

		if(line != NULL)
		{
				char linei[6];
				linei[0] = line[0];
				linei[1] = line[1];
				linei[2] = line[2];	
				linei[3] = line[3];
				linei[4] = line[4];
				linei[5] = '\0';

				if(strstr(linei, ".data") != NULL)//handle normal case and print data & text sections.
				{
					temp = fopen("data.txt", "w+");
					handleData(preprocessed, temp);
					handleText(preprocessed, out, &Taddress);
					rewind(temp);
					char line2[256];
					while(fgets(line2, 256, temp) != NULL)
					{
						fprintf(out, "%s", line2);
					}
					fclose(temp);
					remove("data.txt");
				}
				else//handle extra credit case and print data & text sections.
				{
					handleText(preprocessed, out, &Taddress);
					handleData(preprocessed, out);
				}
		}
	}

	remove("pre.txt");
	fclose(in);
	fclose(out);
	fclose(preprocessed);
	return 0;
}

//handle the text section label.
void handleTLabel(FILE* preprocessed, int* address)
{
	char line[256];

	while(fgets(line, 256, preprocessed) != NULL)
	{
		char* prefix = ".data";
		if(strncmp(line, prefix, 5) == 0)
		{
			break;
		}
		else
		{
			if(strstr(line, ":") != NULL)
			{
				label lab;
				lab.add = *address;

				char* colon = strchr(line, ':');
				int colonPos = colon - line;
				strncpy(lab.name, line, colonPos);
				lab.name[colonPos] = '\0';

				sprintf(lab.hex, "%08X", *address);
				lab.hex[8] = '\0';
				InsertLabel(&list, &lab);
			}
			else
			{
				if((strstr(line, "ble") != NULL && strstr(line, "blez") == NULL)
					|| (strstr(line, "blt") != NULL && strstr(line, "bltz") == NULL))
				{
					*address += 8;
				}
				else
				{
					*address += 4;
				}
			}

		}
	}
}

//handle the data section lable.
void handleDLable(FILE* preprocessed, int* address)
{
	char line[256];

	while(fgets(line, 256, preprocessed) != NULL)
	{
		char* prefix = ".text";
		if(strncmp(line, prefix, 5) == 0)
		{
			break;
		}
		else
		{
			label lab;
			lab.add = *address;
			
			char* colon = strchr(line, ':');
			int colonPos = colon - line;
			strncpy(lab.name, line, colonPos);
			lab.name[colonPos] = '\0';
			sprintf(lab.hex, "%08X", *address);
			lab.hex[8] = '\0';
			InsertLabel(&list, &lab);

			char* dot = strchr(line, '.');
			int dotPos = dot - line;
			if(line[dotPos+1] == 'a')
			{
				char* token;
				token = strtok(line, "\"");
				token = strtok(NULL, "\"");
				int length = strlen(token);
				*address += length + 1;
				while((*address) % 4 != 0)
				{
					*address += 1;
				}
			}
			else
			{
				//count number of ':'
				int count = 0;
				bool several = false;
				for(int i = 0; i < strlen(line); i++)
				{
					if(line[i] == ':')
					{
						count++;
						if(count == 2)
						{
							several = true;
							char* index = strchr(line, '.');
					        char* colon = strchr(index, ':');
					        int colonPos = (int)(colon - index);
							int strLength = strlen(index);
							char var1[strLength - 1 - colonPos];
							int counter1 = 0;
							for(int i = colonPos+1; i < strLength; i++)
							{
								var1[counter1++] = index[i];
							}
							count = atoi(var1);
							*address += 4*count;
							break;
						}
					}
				}

				if(several)
				{
					continue;
				}
				else
				{
					count = 0;
					for(int i = 0; i < strlen(line); i++)
					{
						if(line[i] == ',')
						{
							count++;
						}
					}
					if(count != 0)
					{
						*address += 4*(count+1);
					}
					else
					{
						*address += 4;
					}
				}
			}
		}
	}
}

//insert one label into the static label list.
void InsertLabel(labelList *list, label* lab)
{
	if (list->size != 40)
	{
		list->list[(list->size)++] = *lab;
	}		
}

//find a specific label in the label list.
int findLabel(char* line, labelList* list)
{
	int lineLen = strlen(line);
	for (int i = 0; i < list->size;  i++)
	{
		if (strncmp(line, (list->list[i]).name, lineLen - 1) == 0)
		{
			return i;
		}
	}
	return -1;
}

//handle text section.
void handleText(FILE* preprocessed, FILE* out, int* address)
{
	char line[256];

	while(fgets(line, 256, preprocessed) != NULL)
	{
		char* prefix = ".data";
		if(strncmp(line, prefix, 5) == 0)
		{
			break;
		}
		else
		{
			if(strstr(line, ":") != NULL)
			{
				continue;
			}
			*address+=4;
			parseASM(line, out, address);		
		}
	}

	fprintf(out, "\n");
}

//handle data section.
void handleData(FILE* preprocessed, FILE* out)
{
	char line[256];
	int previous = 0;
	while(fgets(line, 256, preprocessed) != NULL)
	{
		char* prefix = ".text";
		if(strncmp(line, prefix, 5) == 0)
		{
			break;
		}
		else
		{
			previous = parseWord(line, out, previous);
		}
	}
}

//a special method i learned online that can print single character as binary.
void printbincharpad(char c, FILE *out)
{
    for (int i = 7; i >= 0; --i)
    {
        fprintf(out, "%d", (int)((c  >> i) & 1));
    }
}

//main method which handle a single line of data section, print either multiple intergers or ascii characters.
int parseWord(char* line, FILE *out, int previous)//others as ascii, 0 as word.
{
	char* dot = strchr(line, '.');
	int dotPos = dot - line;
	char* IMMRESULT;
	if(line[dotPos+1] == 'a')//ascii part
	{
		char* token = strtok(line, "\"");
		token = strtok(NULL, "\"");
		int size = strlen(token);
		int count = previous;
				
		for(int i = 0; i < size; i++)
		{
			char single = token[i];
			printbincharpad(single, out);
			count++;
			if(count == 4)
			{
				fprintf(out, "\n");
				count = 0;
			}
		}

		fprintf(out, "%s", "00000000");
		count++;
		if(count == 4)
		{
			fprintf(out, "\n");
			count = 0;
		}

		return count;
		
	}
	else{//handle the .word
		if(previous != 0)
		{
			int difference = 4 - previous;
			for(int j = 0; j < difference; j++)
			{
				fprintf(out, "%s", "00000000");
			}
			fprintf(out, "\n");
		}
//count number of ':'
			int count = 0;
			int number;
			
			for(int i = 0; i < strlen(line); i++)
			{
				if(line[i] == ':')
				{
					count++;
					if(count == 2)
					{						
						char* index = strchr(line, '.');
				        char* colon = strchr(index, ':');
				        int colonPos = (int)(colon - index);
						int strLength = strlen(index);
						char var1[strLength - 1 - colonPos];
						int counter1 = 0;
						for(int i = colonPos+1; i < strLength; i++)
						{
							var1[counter1++] = index[i];
						}
						count = atoi(var1);
						char var2[colonPos - 5];
						int counter2 = 0;
						for(int j = 5; j < colonPos; j++)
						{
							var2[counter2++] = index[j];
						}
						number = atoi(var2);
						
						IMMRESULT = createLONG(number);
						for(int j = 0; j < count; j++)
						{				
							fprintf(out, "%32s\n", IMMRESULT);
						}
						return 0;
					}
				}
			}

			count = 0;
			for(int i = 0; i < strlen(line); i++)//count number of ','
			{
				if(line[i] == ',')
				{
					count++;
				}
			}
			if(count == 0)
			{
				char* index = strchr(line, '.');
		        int periodPos = (int)(index - line);

				int strLength = strlen(line);
			        char var[strLength - (periodPos + 5)];
			        int counter = 0;
				for (int i = periodPos + 5; i < strLength; i++)
				{
					var[counter++] = line[i];
				}
				number = atoi(var);
				IMMRESULT = createLONG(number);				
				fprintf(out, "%32s\n", IMMRESULT);

			}
			else
			{
			char* index = strchr(line, '.');
		        int periodPos = (int)(index - line);

				int strLength = strlen(line);
			        char var[strLength - (periodPos + 5)];
			        int counter = 0;
				for (int i = periodPos + 5; i < strLength; i++)
				{
					var[counter++] = line[i];
				}
				char* token1;
				token1 = strtok(var, ",");
				number = atoi(token1);
				IMMRESULT = createLONG(number);					
				fprintf(out, "%32s\n", IMMRESULT);
			for(int i = 1; i < count; i++)
			{
				token1 = strtok(NULL, ",");
				
				number = atoi(token1);
				IMMRESULT = createLONG(number);					
				fprintf(out, "%32s\n", IMMRESULT);
			}
				token1 = strtok(NULL, "\n");
				number = atoi(token1);
				IMMRESULT = createLONG(number);						
				fprintf(out, "%32s\n", IMMRESULT);
			}
		}
		if(IMMRESULT != NULL)
		{
			free(IMMRESULT);
		}
		return 0;
}

//main method which handle all the mips32 instructions.
void parseASM(const char* const pASM, FILE *out, int* address1)
{
	ParseResult* result = calloc(1, sizeof(ParseResult));
	result->Imm = 0;
	result->rd = 255;
	result->rs = 255;
	result->rt = 255;
	result->address = *address1;

	result->Opcode = NULL;            
   	result->Funct = NULL;             
   	result->RD = NULL;                
   	result->RS = NULL;
   	result->RT = NULL;
   	result->IMM = NULL;   
   	result->SHAMT = NULL;

	char linei[6];
	linei[0] = pASM[0];
	linei[1] = pASM[1];
	linei[2] = pASM[2];	
	linei[3] = pASM[3];
	linei[4] = pASM[4];
	linei[5] = '\0';

	if (strstr(linei, "lw") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.lw);
		LW(pASM, result, out);
	}
	else if (strstr(linei, "sw") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.sw);
		LW(pASM, result, out);
	}
	else if (strstr(linei, "lui") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.lui);
		LUI(pASM, result, out);
	}
	else if (strstr(linei, "addiu") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.addiu);
		Itype(pASM, result, out);
	}
	else if (strstr(linei, "addi") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.addi);
		Itype(pASM, result, out);
	}
	else if (strstr(linei, "la") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		char* real = calloc(100, sizeof(char));
		real[0] = 'a';
		real[1] = 'd';
		real[2] = 'd';
		real[3] = 'i';
		char* line[256];
		strcpy((char*)line, pASM);
		char* token;

		token = strtok((char*)line, ",");
		int length = strlen(token);
		char* register1 = calloc(5, sizeof(char));
		int count = 0;
		for(int i = 2; i < length; i++)
		{
			register1[count++] = token[i];
		}
		strcat(real, register1);
		char* zero = ",$zero,";
		strcat(real, zero);
		token = strtok(NULL, ",");
		strcat(real, token);

		strcpy(result->Opcode, opCodes.addi);
		Itype(real, result, out);
		free(real);
		free(register1);
	}
	else if (strstr(linei, "add") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.add);
   		strcpy(result->Funct,functs.add);
		Rtype(pASM, result, out);
	}
	else if (strstr(linei, "nor") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.nor);
   		strcpy(result->Funct,functs.nor);
		Rtype(pASM, result, out);
	}
	else if (strstr(linei, "andi") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.andi);
		Itype(pASM, result, out);
	}
	else if (strstr(linei, "and") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.And);
   		strcpy(result->Funct,functs.And);
		Rtype(pASM, result, out);
	}
	else if (strstr(linei, "ori") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.ori);
		Itype(pASM, result, out);
	}
	else if (strstr(linei, "bne") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.bne);
		BNE(pASM, result, out);
	}
	else if (strstr(linei, "beq") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.beq);
		BNE(pASM, result, out);
	}
	else if (strstr(linei, "sra") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.sra);
   		strcpy(result->Funct,functs.sra);
		SRA(pASM, result, out);
	}
	else if (strstr(linei, "sll") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.sll);
   		strcpy(result->Funct,functs.sll);
		SRA(pASM, result, out);
	}
	else if (strstr(linei, "or") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.Or);
   		strcpy(result->Funct,functs.Or);
		Rtype(pASM, result, out);
	}
	else if (strstr(linei, "sub") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.sub);
   		strcpy(result->Funct,functs.sub);
		Rtype(pASM, result, out);
	}
	else if (strstr(linei, "j") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.j);
		J(pASM, result, out);
	}
	else if (strstr(linei, "blez") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.blez);
		BLEZ(pASM, result, out);
	}
	else if (strstr(linei, "bltz") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.bltz);
		BLEZ(pASM, result, out);
	}	
	else if (strstr(linei, "slti") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.slti);
		Itype(pASM, result, out);
	}
	else if (strstr(linei, "li") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		char* real = calloc(100, sizeof(char));
		real[0] = 'a';
		real[1] = 'd';
		real[2] = 'd';
		real[3] = 'i';
		real[4] = 'u';
		char* line[256];
		strcpy((char*)line, pASM);
		char* token;

		token = strtok((char*)line, ",");
		int length = strlen(token);
		char* register1 = calloc(5, sizeof(char));
		int count = 0;
		for(int i = 2; i < length; i++)
		{
			register1[count++] = token[i];
		}
		strcat(real, register1);
		char* zero = ",$zero,";
		strcat(real, zero);
		token = strtok(NULL, ",");
		strcat(real, token);

		strcpy(result->Opcode, opCodes.addiu);
		Itype(real, result, out);
		free(real);
		free(register1);
	}
	else if (strstr(linei, "slt") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.slt);
   		strcpy(result->Funct,functs.slt);
		Rtype(pASM, result, out);
	}
	else if (strstr(linei, "mul") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		strcpy(result->Opcode, opCodes.mul);
   		strcpy(result->Funct,functs.mul);
		Rtype(pASM, result, out);
	}
	else if (strstr(linei, "nop") != NULL){
		printNOP(out);
	}
	else if (strstr(linei, "sysc") != NULL){
		printSYS(out);
	}
	else if (strstr(linei, "ble") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		char* real = calloc(100, sizeof(char));
		real[0] = 's';
		real[1] = 'l';
		real[2] = 't';
		real[3] = '$';
		real[4] = 'a';
		real[5] = 't';
		real[6] = ',';
		char* line[256];
		strcpy((char*)line, pASM);
		char* token;

		token = strtok((char*)line, ",");
		int length = strlen(token);
		char* register1 = calloc(5, sizeof(char));
		int count = 0;
		for(int i = 3; i < length; i++)
		{
			register1[count++] = token[i];
		}
		
		token = strtok(NULL, ",");
		char* register2 = calloc(5, sizeof(char));
		strcpy(register2, token);
		token = strtok(NULL, ",");
		char* offset = calloc(10, sizeof(char));
		strcpy(offset, token);
		strcat(real, register2);
		strcat(real, ",");
		strcat(real, register1);

		strcpy(result->Opcode, opCodes.slt);
		strcpy(result->Funct,functs.slt);
		Rtype(real, result, out);
		free(real);

		real = calloc(100, sizeof(char));
		real[0] = 'b';
		real[1] = 'e';
		real[2] = 'q';
		real[3] = '$';
		real[4] = 'a';
		real[5] = 't';
		real[6] = ',';
		real[7] = '$';
		real[8] = 'z';
		real[9] = 'e';
		real[10] = 'r';
		real[11] = 'o';
		real[12] = ',';
		strcat(real, offset);
		*address1 += 4;
		parseASM(real, out, address1);
		free(real);
		free(register1);
		free(register2);
		free(offset);
	}
	else if (strstr(linei, "blt") != NULL){
		result->Opcode = calloc(7, sizeof(char));
		result->Funct = calloc(7, sizeof(char));
		char* real = calloc(100, sizeof(char));
		real[0] = 's';
		real[1] = 'l';
		real[2] = 't';
		real[3] = '$';
		real[4] = 'a';
		real[5] = 't';
		real[6] = ',';
		char* line[256];
		strcpy((char*)line, pASM);
		char* token;

		token = strtok((char*)line, ",");
		int length = strlen(token);
		char* register1 = calloc(5, sizeof(char));
		int count = 0;
		for(int i = 3; i < length; i++)
		{
			register1[count++] = token[i];
		}
		
		token = strtok(NULL, ",");
		char* register2 = calloc(5, sizeof(char));
		strcpy(register2, token);
		token = strtok(NULL, ",");
		char* offset = calloc(10, sizeof(char));
		strcpy(offset, token);
		strcat(real, register1);
		strcat(real, ",");
		strcat(real, register2);

		strcpy(result->Opcode, opCodes.slt);
		strcpy(result->Funct,functs.slt);
		Rtype(real, result, out);
		free(real);

		real = calloc(100, sizeof(char));
		real[0] = 'b';
		real[1] = 'n';
		real[2] = 'e';
		real[3] = '$';
		real[4] = 'a';
		real[5] = 't';
		real[6] = ',';
		real[7] = '$';
		real[8] = 'z';
		real[9] = 'e';
		real[10] = 'r';
		real[11] = 'o';
		real[12] = ',';
		strcat(real, offset);
		*address1 += 4;
		parseASM(real, out, address1);
		free(real);
		free(register1);
		free(register2);
		free(offset);
	}
	else
	{
		fprintf(out, "\n");
	}

	clearResult(result);
	free(result);
}

//handle R type.
void Rtype(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	result->shamt = 0;
	result->SHAMT = calloc(6, sizeof(char));
	strcpy(result->SHAMT, "00000");

	char* token;

	token = strtok(p, "$");

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp1[5];
		temp1[4] = '\0';
		memcpy(temp1, token, 4);
		Register(result, temp1, 1);
	}
	else
	{
		char temp1[3];
		temp1[2] = '\0';
		memcpy(temp1, token, 2);
		Register(result, temp1, 1);
	}	


	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp2[5];
		temp2[4] = '\0';
		memcpy(temp2, token, 4);
		Register(result, temp2, 2);
	}
	else
	{
		char temp2[3];
		temp2[2] = '\0';
		memcpy(temp2, token, 2);
		Register(result, temp2, 2);
	}
	

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp3[5];
		temp3[4] = '\0';
		memcpy(temp3, token, 4);
		Register(result, temp3, 3);
	}
	else
	{
		char temp3[3];
		temp3[2] = '\0';
		memcpy(temp3, token, 2);
		Register(result, temp3, 3);
	}
	

	printR(result, out);
}

//handle normal I type.
void Itype(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	char b[100];
	strcpy(b, pASM);

	char* token;

	token = strtok(p, "$");

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp1[5];
		temp1[4] = '\0';
		memcpy(temp1, token, 4);
		Register(result, temp1, 3);
	}
	else
	{
		char temp1[3];
		temp1[2] = '\0';
		memcpy(temp1, token, 2);
		Register(result, temp1, 3);
	}
	

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp2[5];
		temp2[4] = '\0';
		memcpy(temp2, token, 4);
		Register(result, temp2, 2);
	}
	else
	{
		char temp2[3];
		temp2[2] = '\0';
		memcpy(temp2, token, 2);
		Register(result, temp2, 2);
	}
	

	char* token1;

	token1 = strtok(b, ",");
	token1 = strtok(NULL, ",");
	token1 = strtok(NULL, ",");
	int index = findLabel(token1, &list);

	if(index == -1)
	{
		result->Imm = atoi(token1);

		result->IMM = calloc(17, sizeof(char));
		char* IMMRESULT;
		IMMRESULT = createIMM(result->Imm);
		strcpy(result->IMM, IMMRESULT);
		if(IMMRESULT != NULL)
		{
			free(IMMRESULT);
		}
	}
	else
	{
		label lab = list.list[index];

		int gap = lab.add;


		result->IMM = calloc(17, sizeof(char));
		char* IMMRESULT;
		IMMRESULT = createIMM(gap);
		strcpy(result->IMM, IMMRESULT);
		if(IMMRESULT != NULL)
		{
			free(IMMRESULT);
		}
	}

	printI(result, out);
}

//handle SRA instruction.
void SRA(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	char b[100];
	strcpy(b, pASM);

	char* token;

	token = strtok(p, "$");

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp1[5];
		temp1[4] = '\0';
		memcpy(temp1, token, 4);
		Register(result, temp1, 1);
	}
	else
	{
		char temp1[3];
		temp1[2] = '\0';
		memcpy(temp1, token, 2);
		Register(result, temp1, 1);
	}
	

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp2[5];
		temp2[4] = '\0';
		memcpy(temp2, token, 4);
		Register(result, temp2, 3);
	}
	else
	{
		char temp2[3];
		temp2[2] = '\0';
		memcpy(temp2, token, 2);
		Register(result, temp2, 3);
	}
	

	char* token1;

	token1 = strtok(b, ",");
	token1 = strtok(NULL, ",");
	token1 = strtok(NULL, ",");

		result->shamt = atoi(token1);

		result->SHAMT = calloc(6, sizeof(char));
		char* IMMRESULT;
		IMMRESULT = createSHAMT(result->shamt);
		strcpy(result->SHAMT, IMMRESULT);

	if(IMMRESULT != NULL)
	{
		free(IMMRESULT);
	}
	

	printSRA(result, out);
}

//handle bne and beq.
void BNE(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	char b[100];
	strcpy(b, pASM);

	char* token;

	token = strtok(p, "$");

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp1[5];
		temp1[4] = '\0';
		memcpy(temp1, token, 4);
		Register(result, temp1, 2);
	}
	else
	{
		char temp1[3];
		temp1[2] = '\0';
		memcpy(temp1, token, 2);
		Register(result, temp1, 2);
	}
	

	token = strtok(NULL, "$");
	if(strstr(token, "zero") != NULL)
	{
		char temp2[5];
		temp2[4] = '\0';
		memcpy(temp2, token, 4);
		Register(result, temp2, 3);
	}
	else
	{
		char temp2[3];
		temp2[2] = '\0';
		memcpy(temp2, token, 2);
		Register(result, temp2, 3);
	}
	

	char* token1;

	token1 = strtok(b, ",");
	token1 = strtok(NULL, ",");
	token1 = strtok(NULL, ",");
	int index = findLabel(token1, &list);
	label lab = list.list[index];

	int gap = (lab.add - result->address) / 4;


	result->IMM = calloc(17, sizeof(char));
	char* IMMRESULT;
	IMMRESULT = createIMM(gap);
	strcpy(result->IMM, IMMRESULT);
	printI(result, out);
	if(IMMRESULT != NULL)
	{
		free(IMMRESULT);
	}
}

//handle blez and bltz.
void BLEZ(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	char b[100];
	strcpy(b, pASM);
	char* token;

	token = strtok(p, "$");

	token = strtok(NULL, ",");
	if(strstr(token, "zero") != NULL)
	{
		char temp1[5];
		temp1[4] = '\0';
		memcpy(temp1, token, 4);
		Register(result, temp1, 2);
	}
	else
	{
		char temp1[3];
		temp1[2] = '\0';
		memcpy(temp1, token, 2);
		Register(result, temp1, 2);
	}
	

	char* token1;

	token1 = strtok(b, ",");
	token1 = strtok(NULL, ",");
	int index = findLabel(token1, &list);
	label lab = list.list[index];

	int gap = (lab.add - result->address) / 4;


	result->IMM = calloc(17, sizeof(char));
	char* IMMRESULT;
	IMMRESULT = createIMM(gap);
	strcpy(result->IMM, IMMRESULT);
	printBLEZ(result, out);
	if(IMMRESULT != NULL)
	{
		free(IMMRESULT);
	}
}

//handle J.
void J(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	int length = strlen(p);
	char* address = calloc(length, sizeof(char));
	int count = 0;
	for(int i = 1; i < length; i++)
	{
		address[count++] = pASM[i];
	}

	int index = findLabel(address, &list);
	label lab = list.list[index];
	int gap = (lab.add - 0) / 4;
	result->IMM = calloc(17, sizeof(char));
	char* IMMRESULT;
	IMMRESULT = createIMM(gap);
	strcpy(result->IMM, IMMRESULT);
	printJ(result, out);
	free(address);
	if(IMMRESULT != NULL)
	{
		free(IMMRESULT);
	}
}

//handle LUI.
void LUI(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	char b[100];
	strcpy(b, pASM);
	char* token;

	token = strtok(p, "$");

	token = strtok(NULL, ",");
	char temp1[3];
	temp1[2] = '\0';
	memcpy(temp1, token, 2);
	Register(result, temp1, 3);


	char* token1;

	token1 = strtok(b, ",");
	token1 = strtok(NULL, ",");

	result->Imm = atoi(token1);

	result->IMM = calloc(20, sizeof(char));
	char* IMMRESULT;
	IMMRESULT = createIMM(result->Imm);
	strcpy(result->IMM, IMMRESULT);
	printLUI(result, out);
	if(IMMRESULT != NULL)
	{
		free(IMMRESULT);
	}
}

//handle LW and SW.
void LW(const char* const pASM, ParseResult* result, FILE *out)
{
	char p[100];
	strcpy(p, pASM);
	char b[100];
	strcpy(b, pASM);
	char* token;

	token = strtok(p, "$");
	token = strtok(NULL, "$");
	char temp1[3];
	temp1[2] = '\0';
	memcpy(temp1, token, 2);
	Register(result, temp1, 3);
	char d[100];
	strcpy(d, token);

	if(strchr(d, '(') == NULL)
	{
		char* token1;

		token1 = strtok(token, ",");
		token1 = strtok(NULL, ",");
		result->RS = calloc(6, sizeof(char));
		strcpy(result->RS, "00000");

		int index = findLabel(token1, &list);
		label lab = list.list[index];

		int gap = (lab.add - 0);


		result->IMM = calloc(17, sizeof(char));
		char* IMMRESULT;
		IMMRESULT = createIMM(gap);
		strcpy(result->IMM, IMMRESULT);
		printI(result, out);
		if(IMMRESULT != NULL)
		{
			free(IMMRESULT);
		}
	}
	else
	{
		token = strtok(NULL, "$");
		char temp2[3];
		temp2[2] = '\0';
		memcpy(temp2, token, 2);
		Register(result, temp2, 2);

		char* comma = strchr(pASM, ',');
		char* bracket = strchr(comma, '(');
		int bracketPos = bracket - comma;
		char* var = calloc(bracketPos - 1, sizeof(char));
		int counter = 0;
		for(int i = 1; i < bracketPos; i++)
		{
			var[counter++] = comma[i];
		}
		result->Imm = atoi(var);

		result->IMM = calloc(20, sizeof(char));
		char* IMMRESULT;
		IMMRESULT = createIMM(result->Imm);
		strcpy(result->IMM, IMMRESULT);
		free(var);
		printI(result, out);
		if(IMMRESULT != NULL)
		{
			free(IMMRESULT);
		}
	}

	
}

//convert an integer into a 5 bits long binary form.
char* createSHAMT(int input)
{
	char* initial = calloc(6, sizeof(char));
	initial[5] = '\0';
	
		int sign = 1;
		if(input < 0)
		{
			sign = -1;
			input = -1*input;
		}

		for(int index = 4; index >= 0; index--)
		{
			int temp = input % 2;
			initial[index] = temp + '0';
			input = input / 2;
		}

		if(sign == -1)
		{
			for(int i = 4; i > 0; i--)
			{
				if(initial[i] == '1')
				{
					for(int j = i - 1; j >= 0; j--)
					{
						if(initial[j] == '0')
						{
							initial[j] = '1';
						}
						else
						{
							initial[j] = '0';
						}
					}

					break;
				}
			}
		}
	

	return initial;
}

//convert an integer into a 32 bits long binary form.
char* createLONG(int32_t input)
{
	char* initial = calloc(33, sizeof(char));
	initial[32] = '\0';

	if(input == -2147483648)
	{
		initial = "10000000000000000000000000000000";
	}
	else
	{
		int sign = 1;
		if(input < 0)
		{
			sign = -1;
			input = -1*input;
		}

		for(int index = 31; index >= 0; index--)
		{
			int temp = input % 2;
			initial[index] = temp + '0';
			input = input / 2;
		}

		if(sign == -1)
		{
			for(int i = 31; i > 0; i--)
			{
				if(initial[i] == '1')
				{
					for(int j = i - 1; j >= 0; j--)
					{
						if(initial[j] == '0')
						{
							initial[j] = '1';
						}
						else
						{
							initial[j] = '0';
						}
					}

					break;
				}
			}
		}
	}

	return initial;
}

//convert an integer into a 16 bits long binary form.
char* createIMM(int input)
{
	char* initial = calloc(17, sizeof(char));
	initial[16] = '\0';

	if(input == -32768)
	{
		initial = "1000000000000000";
	}
	else
	{
		int sign = 1;
		if(input < 0)
		{
			sign = -1;
			input = -1*input;
		}

		for(int index = 15; index >= 0; index--)
		{
			int temp = input % 2;
			initial[index] = temp + '0';
			input = input / 2;
		}

		if(sign == -1)
		{
			for(int i = 15; i > 0; i--)
			{
				if(initial[i] == '1')
				{
					for(int j = i - 1; j >= 0; j--)
					{
						if(initial[j] == '0')
						{
							initial[j] = '1';
						}
						else
						{
							initial[j] = '0';
						}
					}

					break;
				}
			}
		}
	}

	return initial;
}

//method which handle all the registers.
void Register(ParseResult* result, char* registerTemp, int index)
{
	uint8_t id = 0;
	char* registerBits = calloc(7, sizeof(char));

	for(int i = 0; i < 32; i++)
	{
		if(strcmp(registerTemp, registers[i].regName) == 0)
		{
			id = registers[i].regDecimal;
			strcpy(registerBits, registers[i].regNumber);
			break;
		}
	}

	if(index == 1)
	{
		result->RD = calloc(7, sizeof(char));
		result->rd = id;
		strcpy(result->RD, registerBits);
	}
	else if(index == 2)
	{
		result->RS = calloc(7, sizeof(char));
		result->rs = id;
		strcpy(result->RS, registerBits);
	}
	else if(index == 3)
	{
		result->RT = calloc(7, sizeof(char));
		result->rt = id;
		strcpy(result->RT, registerBits);
	}

	free(registerBits);
}