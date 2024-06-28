// asmm1600
// Microdata 821/1600/MAI Basic Four 13xx
// Armin Diehl 09/2015

#define versionName "Mocrodata 821 and 1600 assembler"
#include "asmx.h"

#define ERRPAR_FF Warning("Address / parameter should be in the range of 0 to ffh")
#define ERRPAR_7FFF Warning("Address / parameter should be in the range of 0 to 7fffh")
#define ERRSHORT Error("Short branch/reference out of range")
#define ERRPAR_WSIZE Error ("Size prefix for variable length opcodes must be in range 1 to 4")
#define SHORT "SHORT"
#define PAGE0 "PAGE0"
#define ERRTOLARGE_SHORT Error("Address / parameter to large for 'SHORT' prefix")
#define ERRNOEFFECT_SHORT Warning ("'SHORT' prefix has no effect")
#define ERROR_PAGE0 Error("Address not valid for page 0")
#define ERROR_SHORTRANGE Error("Address out of range")


enum
{
    o_None,         // No operands
    o_w1,           // word length to 1
    o_w2,
    o_w3,
    o_w4,
    o_oldSyn,
    o_BraRel,       // short branch
    o_Bra16,        // long branch
    o_Imm8,         // one byte as parameter
    o_Imm16,        // two bytes as parameter
    o_MemRef,       // m=0..7 (Section 2)
    o_MemRefV,      // variable, m=0..7 (Section 2)
    o_MemRef01,     // old style mode 0 and 1, with _ appended, blank would conflict with the new syntax
    o_MemRef23,     // old style mode 2 and 3, with * appended
    o_MemRef4,      // old style mode 2 and 3, with - appended
    o_MemRef5,
    o_MemRef6,
    o_MemRef7,
    o_MemRef7Jump,
    o_MemRefJump,   // m=0..7 (Section 2)
    o_InOut,
    o_IBM_OBM


//  o_Foo = o_LabelOp,
};

#define OP_JMP 0x60
#define OP_RJMP 0x68

struct OpcdRec MD1600_opcdTab[] =
{
    {".W1", o_w1,       0x00},
    {".W2", o_w2,       0x00},
    {".W3", o_w3,       0x00},
    {".W4", o_w4,       0x00},
    {"OLDSYN", o_oldSyn,0},
    
    {"HLT", o_None,     0x00},
    {"TRP", o_None,     0x01},
    
    {"ESW", o_None,     0x03},  // is it 02 or 03, 71-1-821-001_1600-21_Aug71=03, 1600_30.pdf=02, need to be tested
    {"DIN", o_None,     0x04},
    {"EIN", o_None,     0x05},
    {"DRT", o_None,     0x06},
    {"ERT", o_None,     0x07},
    {"RO1", o_None,     0x08},
    {"RO2", o_None,     0x09},
    {"RO3", o_None,     0x0A},
    {"RO4", o_None,     0x0B},
    {"SO1", o_None,     0x0C},
    {"SO2", o_None,     0x0D},
    {"SO3", o_None,     0x0E},
    {"SO4", o_None,     0x0F},

    
    /* conditional jumps, all relative 8 bit signed */
    {"JOV", o_BraRel,   0x10},
    {"JAZ", o_BraRel,   0x11},
    {"JBZ", o_BraRel,   0x12},
    {"JXZ", o_BraRel,   0x13},
    {"JAN", o_BraRel,   0x14},
    {"JXN", o_BraRel,   0x15},
    {"JAB", o_BraRel,   0x16},
    {"JAX", o_BraRel,   0x17},
    {"NOV", o_BraRel,   0x18},
    {"NAZ", o_BraRel,   0x19},
    {"NBZ", o_BraRel,   0x1A},
    {"NXZ", o_BraRel,   0x1B},
    {"NAN", o_BraRel,   0x1C},
    {"NXN", o_BraRel,   0x1D},
    {"NAB", o_BraRel,   0x1E},
    {"NAX", o_BraRel,   0x1F},
    {"JEP", o_BraRel,   0x5A},
    
    /* shifts */
    {"LLA", o_Imm8,     0x20},
    {"LLB", o_Imm8,     0x21},
    {"LLL", o_Imm8,     0x22},
    
    {"LRA", o_Imm8,     0x24},
    {"LRB", o_Imm8,     0x25},
    {"LRL", o_Imm8,     0x26},
    {"ALA", o_Imm8,     0x28},
    {"ALB", o_Imm8,     0x29},
    {"ALL", o_Imm8,     0x2A},
    {"ARA", o_Imm8,     0x2C},
    {"ARB", o_Imm8,     0x2D},
    {"ARL", o_Imm8,     0x2E},
    
    {"DAD", o_None,     0x3C},
    {"DSB", o_None,     0x3D},
    
    {"MUL", o_Imm16,    0x3E},
    {"DIV", o_Imm16,    0x3F},
    
    /* input / output operations */
    {"IBA",   o_InOut,    0x31},
    {"IBB",   o_InOut,    0x32},
    {"IBM",   o_IBM_OBM,  0x33},
    {"NOP",   o_None,     0x34},
    {"OBA",   o_InOut,    0x39},
    {"OBB",   o_InOut,    0x3A},
    {"OBM",   o_IBM_OBM,  0x3B},
    
    /* register operate */
    {"ORA", o_None,     0x40},
    {"XRA", o_None,     0x41},
    {"ORB", o_None,     0x42},
    {"XRB", o_None,     0x43},
    {"INX", o_None,     0x44},
    {"DCX", o_None,     0x45},
    {"AWX", o_None,     0x46},
    {"SWX", o_None,     0x47},
    {"INA", o_None,     0x48},
    {"INB", o_None,     0x49},
    {"OCA", o_None,     0x4A},
    {"ORB", o_None,     0x4B},
    {"TAX", o_None,     0x4C},
    {"TBX", o_None,     0x4D},
    {"TXA", o_None,     0x4E},
    {"TXB", o_None,     0x4F},
    
    {"MST", o_Imm16,    0x58},
    {"ADX", o_Imm16,    0x59},
    
    {"EBX", o_None,     0x5B},
    
    /* Stack */
    {"RTN",   o_None,     0x50},
    {"RET",   o_None,     0x50},
    {"CAL",   o_Bra16,    0x51},
    {"CALL",  o_Bra16,    0x51},
    {"PLX",   o_None,     0x52},
    {"POPX",  o_None,     0x52},
    {"PSX",   o_None,     0x53},
    {"PUSHX", o_None,     0x53},
    {"PLA",   o_None,     0x54},
    {"POPA",  o_None,     0x54},
    {"PSA",   o_None,     0x55},
    {"PUSHA", o_None,     0x55},
    {"PLB",   o_None,     0x56},
    {"POPB",  o_None,     0x56},
    {"PSB",   o_None,     0x57},
    {"PUSHB", o_None,     0x57},
    
    /* character/string manipulation */
    {"CLC",   o_None,     0x35},
    {"MOV",   o_None,     0x5C},
    {"GCC",   o_None,     0x5D},
    {"SCH",   o_None,     0x5E},
    {"GAP",   o_None,     0x5F},
    
    /* Memory reference ops */
    {"JR",    o_BraRel   ,OP_JMP | 0x01},
    {"JMP",   o_MemRefJump,OP_JMP},
    {"JMP_",  o_MemRef01  ,OP_JMP},
    
    {"RTJ",   o_MemRefJump,0x68},
    {"RTJ_",  o_MemRef01  ,0x68},
    
    {"IWM",     o_MemRef,   0x70},
    {"IWM_",    o_MemRef01, 0x70},
    
    {"DWM",     o_MemRef,   0x78},
    {"DWM_",    o_MemRef01, 0x78},
    
    {"LDX",     o_MemRef,   0x80},
    {"LDX_",    o_MemRef01, 0x80},
    
    {"STX",     o_MemRef,   0x88},
    {"STX_",    o_MemRef01, 0x88},
    
    {"LDB",     o_MemRef,   0x90},
    {"LDB_",    o_MemRef01, 0x90},
        
    {"STB",     o_MemRef,   0x98},
    {"STB_",    o_MemRef01, 0x98},
    
    {"ADA",     o_MemRef,   0xA0},
    {"ADA_",    o_MemRef01, 0xA0},
    
    {"ADV",     o_MemRefV,  0xA8},
    {"ADV_",    o_MemRef01, 0xA8},
    
    {"SBA",     o_MemRef,   0xB0},
    {"SBA_",    o_MemRef01, 0xB0},
    
    {"SBV",     o_MemRefV,  0xB8},
    {"SBV_",    o_MemRef01, 0xB8},
    
    {"CPA",     o_MemRef,   0xC0},
    {"CPA_",    o_MemRef01, 0xC0},
    
    {"CPV",     o_MemRefV,  0xC8},
    {"CPV_",    o_MemRef01, 0xC8},
    
    {"ANA",     o_MemRef,   0xD0},
    {"ANA_",    o_MemRef01, 0xD0},
    
    {"ANV",     o_MemRef,   0xD8},
    {"ANV_",    o_MemRef01, 0xD8},
    
    {"LDA",     o_MemRef,   0xE0},
    {"LDA_",    o_MemRef01, 0xE0},
    
    {"LDV",     o_MemRefV,  0xE8},
    {"LDV_",    o_MemRef01, 0xE8},
    
    {"STA",     o_MemRef,   0xF0},
    {"STA_",    o_MemRef01, 0xF0},
    
    {"STV",     o_MemRef,   0xF8},
    {"STV_",    o_MemRef01, 0xF8},
    
    

    {"",    o_Illegal,  0}
};

int mdWordLength = 1;
int mdSupportOldSyntax = 1;


// --------------------------------------------------------------

/*
int Get_1802_Reg(void)
{
    Str255  word;
    int     token;
    char    *oldLine;

    oldLine = linePtr;
    token = GetWord(word);

    if (word[0]=='R')
    {
        // R0-R9
        if ('0'<=word[1] && word[1]<='9' && word[2]==0)
            return word[1] - '0';
        // RA-RF
        if ('A'<=word[1] && word[1]<='F' && word[2]==0)
            return word[1] - 'A' + 10;
        // R10-R15
        if (word[1]=='1' && '0'<=word[2] && word[2]<='5' && word[3]==0)
            return word[2] - '0' + 10;
    }

    // otherwise evaluate an expression
    linePtr = oldLine;
    return Eval();
}
*/


void InstrVar(int parm, int val, int wordLength) {
    
    switch (wordLength) {
        case 1: InstrBB(parm | 0x07, val);
                break;
        case 2: InstrBW(parm | 0x07, val);
                break;
        case 3: InstrClear();
                InstrAddB(parm | 0x07);
                InstrAdd3(val);
                break;
        case 4: InstrClear();
                InstrAddB(parm | 0x07);
                InstrAddL(val);
                break;
    }
}

int checkOptionalKeyword2(char * word1, char * word2) {
    char *lineSave;
    Str255 s;
    
    lineSave = linePtr;
    if (GetWord(s) == 0) {
        linePtr = lineSave;
        return 0;
    }
    if (strcmp(word1,s) == 0) return 1;
    if (word2)
      if (strcmp(word2,s) == 0) return 2;
    linePtr = lineSave;
    return 0;
}

int checkOptionalKeyword(char * word) {
    return checkOptionalKeyword2(word,NULL);
}


#define O_SHORT 1
#define O_PAGE0 2

int MD1600_DoCPUOpcode(int typ, int parm)
{
    int     val,val2;
    Str255  s;
    char    *oldLine;
    int     wordLength;
    int     shortOpt;
    
    
    if (mdSupportOldSyntax && (parm >= OP_JMP)) {
        //oldLine = linePtr;
        //GetWord(s);
        //if (strlen(s) == 1) {
        //    switch (s[0]) {
            switch (*linePtr) {
                //case '_':   typ = o_MemRef01;  // old style was blank, not _(not handled here, contained in table)
                //            break;
                case '*':   typ = o_MemRef23; linePtr++;
                            break;
                case '-':   typ = o_MemRef4;  linePtr++;
                            break;
                case '+':   typ = o_MemRef5;  linePtr++;
                            break;
                case '/':   typ = o_MemRef6 ;  linePtr++;
                            break;
                case '=':   if ((parm == OP_JMP) || (parm == OP_RJMP))
                              typ = o_MemRef7Jump;
                            else
                              typ = o_MemRef7;
                             linePtr++;
                            break;
                
                default:    //linePtr = oldLine;
                            break;
                
            }
        //} else
        //  linePtr = oldLine;
    }

    switch(typ)
    {
        
        case o_None:
            InstrB(parm);
            break;
        case o_w1:
            mdWordLength = 1;
            break;
        case o_w2:
            mdWordLength = 2;
            break;
        case o_w3:
            mdWordLength = 3;
            break;
        case o_w4:
            mdWordLength = 4;
            break;
        case o_oldSyn:
            val = Eval();
            mdSupportOldSyntax = (val > 0);
            break;
            
        case o_Imm8:
            val = EvalByte();
            InstrBB(parm,val);
            break;
            
        case o_Imm16:
            val = Eval();
            CheckWord(val);
            InstrBW(parm,val);
            break;
            
        case o_BraRel:
            val = EvalBranch(2);
            InstrBB(parm,val);
            break;
            
        case o_Bra16:
            val = Eval();
            CheckWord(val);
            InstrBW(parm,val);
            break;
            
        case o_MemRef:
        case o_MemRefV:
            shortOpt = checkOptionalKeyword2(SHORT,PAGE0);  // switch to relative or page 0

            if (checkOptionalKeyword("[")) {  // direct, indirect or indexed, m=0,1,4 or 7
				oldLine = linePtr;
				GetWord(s);
				if (strcmp(s,"*") == 0) { // indirect
					val = Eval();
					if (((val <= 0xFF) || (shortOpt == O_PAGE0)) && (shortOpt != O_SHORT)) {  // page 0
                        if (val >= 0xFF) ERROR_PAGE0;
						InstrBB(parm | 0x02,val);
						Expect("]");
						break;
					}
					// m=3: in range of +127 to -128 based on the address after this opcode
					val2 = val - locPtr - 2;
					if (val2 < -128 || val2 > 127)
						ERRSHORT;
					InstrBB(parm | 0x03 ,val2);
					Expect("]");
					break;
					
			    } else
				if (strcmp(s,"X") == 0) { // Indexed, m=4 - LDA [X]
					if (checkOptionalKeyword("+")) { // Indexed with bias, m=5 or m=6, LDA [X+expression]
						val = Eval();
						if (val >= 0 && val <= 255 && (shortOpt == O_SHORT)) 
                            InstrBB(parm | 0x05, val);
                        else {
                            if ((val <= 0) || (val >= 0x7fff))
                                ERRPAR_7FFF;
                            if (shortOpt > 0) ERRTOLARGE_SHORT;
                            InstrBW(parm | 0x06, val | 0x8000);
                        }
						Expect("]");
						break;
					}
                    if (shortOpt > 0) ERRNOEFFECT_SHORT;
					InstrB(parm | 0x04);
					Expect("]");
					break;
				}
                
				linePtr = oldLine;
				val = Eval(); 				// LDA [expression]
                Expect("]");
                if (shortOpt > 0) {
                    if ((shortOpt == O_PAGE0) || ((shortOpt == 0) && (val <= 255))) {  // Mode 0: Direct page 0
                        if (val > 255) ERROR_SHORTRANGE;
                        InstrBB(parm,val);
                        break;
                    }
                    // m=1: in range of +127 to -128 based on the address after this opcode
                    val2 = val - locPtr - 2;
                    if (!(val2 < -128 || val2 > 127)) {
                        InstrBB(parm | 0x01 ,val2);
                        break;
                    }
                }
				// m=6, 16 bit address
				InstrBW(parm | 0x06, val);
                if (shortOpt > 0) ERRTOLARGE_SHORT;
				break;
				
			}
	
            if (typ == o_MemRef) {
                // could only be m=7 (literal 16 bit)
                val = Eval();
                InstrBW(parm | 0x07,val);
                if (shortOpt > 0) ERRTOLARGE_SHORT;
                break;
            }
            if (shortOpt > 0) ERRNOEFFECT_SHORT;
            wordLength = mdWordLength;
            // for the variable length opcodes use the default
            // wordLength if the word length is not prefixed
            val = Eval();
            if (checkOptionalKeyword(",")) { // length was specified
                if ((val < 0) || (val > 4))
                    ERRPAR_WSIZE;
                else
                    wordLength = val;
                val = Eval();
            }
            InstrVar(parm | 0x07, val, wordLength);
            break;
            
        case o_MemRefJump:
            shortOpt = checkOptionalKeyword2(SHORT,PAGE0);  // switch to relative or page 0
			oldLine = linePtr;
			GetWord(s);

			if (strcmp(s,"X") == 0) {
				oldLine = linePtr;
				if (GetWord(s) == 0) {		// at end of line ?
					// can only be m=4 JMP X
					InstrB(parm | 0x04);
					break;
				}
				linePtr = oldLine;
				Expect("+");
				oldLine = linePtr;
				GetWord(s);
				if (strcmp(s,"[") == 0) {  // m=7 )(x=1), indirect extended - JMP X+[1234h]
					val = Eval();
					Expect("]");
					if ((val < 0) || (val > 0x7fff)) 
						ERRPAR_7FFF;
                    if (shortOpt) ERRTOLARGE_SHORT;
					InstrBW(parm | 0x07, val | 0x8000);
					break;
				}
				linePtr = oldLine;
				// m=5 or m=6, extended address with x=1
				val = Eval();
				if ((val >= 0) && (val <= 0xff) && (shortOpt == O_SHORT)) { // m=5, indexed with bias
					InstrBB(parm | 0x05, val);
					break;
				}
				if ((val < 0) || (val > 0x7fff)) 
					ERRPAR_7FFF;
                if (shortOpt) ERRTOLARGE_SHORT;
				InstrBW(parm | 0x06, val | 0x8000);
				break;
			}
			if (strcmp(s,"[") == 0) {	// m=2,3 or 7 (x=0)
				val = Eval();
				Expect("]");
				if (((val >= 0) && (val <= 255)) || (shortOpt == O_PAGE0)) {  // m=2, indirect page 0
                    if (val > 255) ERROR_SHORTRANGE;
					InstrBB(parm | 0x02, val);
					break;
				}
				
				// m=3: indirect relative
				val2 = val - locPtr - 2;
				if (!(val2 < -128 || val2 > 127)) {
					InstrBB(parm | 0x03 ,val2);
					break;
				}
				
				// m=7 indirect extended, x=0
				if ((val < 0) || (val > 0x7fff)) 
					ERRPAR_7FFF;
				InstrBW(parm | 0x07, val & 0x7FFF);
				break;
			}
			// m=0,1 or 6 (x=0)
			linePtr = oldLine;
			val = Eval();
			
			//printf("shortOpt: %d, val: 0x%04x, instrLen:%d\n",shortOpt,val,instrLen);
			
            if (shortOpt > 0) {
                if (shortOpt == O_PAGE0) {  // m=0, direct page 0
                    if ((val < 0) || (val > 255)) ERROR_SHORTRANGE;
                    InstrBB(parm, val);
                    break;
                }
				
                // m=1: direct relative
                val2 = val - locPtr - 2;
                //printf("val2: 0x%04x\n",val2);
                if ((val2 < -128) || (val2 > 127)) ERROR_SHORTRANGE;
                InstrBB(parm | 0x01 ,val2);
                break;
                
            }
				
			// m=6 extended address, x=0
			if ((val < 0) || (val > 0x7fff)) 
				ERRPAR_7FFF;
            if (shortOpt > 0) ERRTOLARGE_SHORT;
			InstrBW(parm | 0x06, val & 0x7FFF);
			break;
            
        case o_MemRef01:  // old style m=0 or 1
            val = Eval();
            if ((val >= 0) && (val <= 255)) {   // page 0
                InstrBB(parm,val);
                break;
            }
            val2 = val - locPtr - 2;
			if (!(val2 < -128 || val2 > 127)) {
				InstrBB(parm | 0x01 ,val2);
				break;
			}
            ERRSHORT;
            break;
            
        case o_MemRef23:  // old style m=2 or 3
            val = Eval();
            if ((val >= 0) && (val <= 255)) {   // page 0
                InstrBB(parm | 0x02,val);
                break;
            }
            val2 = val - locPtr - 2;
			if (!(val2 < -128 || val2 > 127)) {
				InstrBB(parm | 0x03 ,val2);
				break;
			}
            ERRSHORT;
            break;
            
        case o_MemRef4:
            InstrB(parm | 0x04);
            break;
            
            
        case o_MemRef5:
            val = Eval();
            if ((val < 0) || (val > 255)) {
                ERRPAR_FF;
                val = 0;
            }
            InstrBB(parm | 0x05, val);
            break;
        
        case o_MemRef7Jump:
        case o_MemRef6:
            val = Eval();
            if (checkOptionalKeyword(",")) { // ,X is optional
				Expect("X");
                InstrBW(parm | (0x06 + (typ == o_MemRef7Jump)) , val | 0x8000);
                break;
            }    
            if ((val <= 0) || (val >= 0x7fff))
                ERRPAR_7FFF;
            InstrBW(parm | (0x06 + (typ == o_MemRef7Jump)), val);
            break;
            
        case o_MemRef7: // literal, 1..4 bytes
            wordLength = mdWordLength;
            val = Eval();
            if (checkOptionalKeyword(",")) { // ,1 2 3 or 4 is optional
                wordLength = Eval();
                if ((wordLength < 1) || (wordLength > 4)) {
                    ERRPAR_WSIZE;
                    wordLength = 1;
                }
            }
            InstrVar(parm | 0x07, val, wordLength);
            break;
            
        // OBA 0
        // OBA Device(0..4),OrderCode(5..7)
        case o_InOut:
            val = Eval();
            if (checkOptionalKeyword(",")) { // , is optional
				val2 = Eval();
                val = val & 0x1f;  // device only
                val2 = (val2 & 0x07) << 5;
                val = val | val2;
            }
            InstrBB(parm, val);
            break;
           
        // OBM 4,4,Buf
        // OBM 4,4,Buf,X
        case o_IBM_OBM:
            val = Eval();
            Expect(",");
            val2 = Eval();
            val = val & 0x1f;  // device only
            val2 = (val2 & 0x07) << 5;
            val = val | val2;
            Expect(",");
            val2 = Eval();  // address
            if (checkOptionalKeyword(",")) { // ,X is optional
                Expect("X");
                InstrBBW(parm, val, val2 | 0x8000);
                break;
            }
            InstrBBW(parm, val, val2);
            break;
            

        default:
            return 0;
            break;
    }

    return 1;
}


void AsmMD1600Init(void)
{
    char *p;

    p = AddAsm(versionName, &MD1600_DoCPUOpcode, NULL, NULL);
    AddCPU(p, "MD1600", 0, BIG_END, ADDR_16, LIST_24, 8, 0, MD1600_opcdTab);
}
