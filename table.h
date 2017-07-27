#ifndef TABLE_H
#define TABLE_H
#include <inttypes.h>
#include <stdio.h>

struct _RegMapping { // register name to number
char* regName; // symbolic name as C-string
char* regNumber; // string for binary representation
uint8_t regDecimal; //int for decimal representation
};
typedef struct _RegMapping RegMapping;

struct _OpCode {
char* lw;
char* sw;
char* lui;
char* add;
char* addi;
char* addiu;
char* And;
char* andi;
char* mul;
char* nor;
char* Or;
char* ori;
char* sll;
char* slt;
char* slti;
char* sra;
char* sub;
char* beq;
char* blez;
char* bltz;
char* bne;
char* j;
};

typedef struct _OpCode OpCode;

struct _funct {
char* add;
char* And;
char* mul;
char* nor;
char* Or;
char* sll;
char* slt;
char* sra;
char* sub;
};

typedef struct _funct funct;



/**  Represents the possible field values for a MIPS32 machine instruction.
 * 
 *   A ParseResult object is said to be proper iff:
 * 
 *     - Each of the char* members is either NULL or points to a zero-
 *       terminated C-string.
 *     - If ASMInstruction is not NULL, the contents of the array represent
 *       a MIPS32 assembly instruction.
 *     - If ASMInstruction is not NULL, the other fields are set to properly
 *       represent the corrsponding fields of the MIPS32 assembly instruction
 *       stored in ASMInstruction.
 *     - Each field that is not relevant to the MIPS32 assembly instruction
 *       is set as described in the comments below.
 */
struct _ParseResult {
   // The assembly code portion
   //   These are malloc'd zero-terminated C-string 
   
   //   The following are integer values
   int  Imm;            // the immediate field, as a signed integer
   uint8_t  rd;             // the three register fields, as small unsigned integers;
   uint8_t  rs;             //   255 if not present
   uint8_t  rt;
   int  shamt;
   int address;

   // The computed machine code portion
   //   These are malloc'd zero-terminated C-strings
   char* Opcode;            // the opcode field bits
   char* Funct;             // the funct field bits
   char* RD;                // the bit representations of the register numbers; 
   char* RS;                //   NULL if not specified in the assembly instruction
   char* RT;
   char* IMM;
   char* SHAMT;              // the bit representation of the immediate;
                            //   NULL if not present
};

typedef struct _ParseResult ParseResult;

/**  Frees the dynamic content of a ParseResult object.
 * 
 *   Pre:  pPR points to a proper ParseResult object.
 *   Post: All of the dynamically-allocated arrays in *pPR have been
 *         deallocated.
 *         *pPR is proper.
 * 
 *   Comments:
 *     -  The function has no information about whether *pPR has been
 *        allocated dynamically, so it cannot risk attempting to 
 *        deallocate *pPR.
 *     -  The function is intended to provide the user with a simple
 *        way to free memory; the user may or may not reuse *pPR.  So,
 *        the function does set the pointers in *pPR to NULL.
 */
void clearResult(ParseResult* const pPR);

#endif