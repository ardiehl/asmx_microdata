// asmm1600
// Microdata 821/1600/MAI Basic Four 13xx
// Armin Diehl 09/2015

#define version "1.02"
#define versionNameMD "Microdata 821 and 1600 assembler " version
#define versionNameBF "Basic Four 1200,1300 and 1320 assembler " version
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

// Microdata only (not Basic Four)
#define CPU1600 0x0100
// 1200 is for basic four 1200
#define CPU1200 0x0200
// the 13xx have some different opcodes
#define CPU1300 0x0400
#define CPU1320 0x0800
#define CPUALL (CPU1600+CPU1200+CPU1300+CPU1320)
#define CPU13xx (CPU1300+CPU1320)
#define CPU1213 (CPU1200+CPU1300)
#define CPU1216 (CPU1200+CPU1600)
#define CPUBF (CPU1200+CPU1300+CPU1320)
#define CPUNO20 (CPU1600+CPU1200+CPU1300)
#define CPUMASK 0x0f00
#define INST3F 0x1000
#define INST47 0x2000


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
    {".W1", o_w1,       0x00 | CPU1600},
    {".W2", o_w2,       0x00 | CPU1600},
    {".W3", o_w3,       0x00 | CPU1600},
    {".W4", o_w4,       0x00 | CPU1600},
    {"OLDSYN", o_oldSyn,0 | CPU1600},

    {"HLT", o_None,     0x00 | CPU1600},
    {"TRP", o_None,     0x01 | CPU1600},
    {"ESW", o_None,     0x03 | CPU1600},

    {"DIN", o_None,     0x04 | CPU1600},
    {"EIN", o_None,     0x05 | CPU1600},
    {"DRT", o_None,     0x06 | CPU1600},
    {"ERT", o_None,     0x07 | CPU1600},
    {"RO1", o_None,     0x08 | CPU1600},
    {"RO2", o_None,     0x09 | CPU1600},
    {"RO3", o_None,     0x0A | CPU1600},
    {"RO4", o_None,     0x0B | CPU1600},
    {"SO1", o_None,     0x0C | CPU1600},
    {"SO2", o_None,     0x0D | CPU1600},
    {"SO3", o_None,     0x0E | CPU1600},
    {"SO4", o_None,     0x0F | CPU1600},


    /* conditional jumps, all relative 8 bit signed */
    {"JOV", o_BraRel,   0x10 | CPU1600},
    {"JAZ", o_BraRel,   0x11 | CPU1600},
    {"JBZ", o_BraRel,   0x12 | CPU1600},
    {"JXZ", o_BraRel,   0x13 | CPU1600},
    {"JAN", o_BraRel,   0x14 | CPU1600},
    {"JXN", o_BraRel,   0x15 | CPU1600},
    {"JAB", o_BraRel,   0x16 | CPU1600},
    {"JAX", o_BraRel,   0x17 | CPU1600},
    {"NOV", o_BraRel,   0x18 | CPU1600},
    {"NAZ", o_BraRel,   0x19 | CPU1600},
    {"NBZ", o_BraRel,   0x1A | CPU1600},
    {"NXZ", o_BraRel,   0x1B | CPU1600},
    {"NAN", o_BraRel,   0x1C | CPU1600},
    {"NXN", o_BraRel,   0x1D | CPU1600},
    {"NAB", o_BraRel,   0x1E | CPU1600},
    {"NAX", o_BraRel,   0x1F | CPU1600},
    {"JEP", o_BraRel,   0x5A | CPU1600},

    /* shifts */
    {"LLA", o_Imm8,     0x20 | CPU1600},
    {"LLB", o_Imm8,     0x21 | CPU1600},
    {"LLL", o_Imm8,     0x22 | CPU1600},

    {"LRA", o_Imm8,     0x24 | CPU1600},
    {"LRB", o_Imm8,     0x25 | CPU1600},
    {"LRL", o_Imm8,     0x26 | CPU1600},
    {"ALA", o_Imm8,     0x28 | CPU1600},
    {"ALB", o_Imm8,     0x29 | CPU1600},
    {"ALL", o_Imm8,     0x2A | CPU1600},
    {"ARA", o_Imm8,     0x2C | CPU1600},
    {"ARB", o_Imm8,     0x2D | CPU1600},
    {"ARL", o_Imm8,     0x2E | CPU1600},

    {"DAD", o_None,     0x3C | CPU1600},
    {"DSB", o_None,     0x3D | CPU1600},

    {"MUL", o_Imm16,    0x3E | CPU1600},
    {"DIV", o_Imm16,    0x3F | CPU1600},

    /* input / output operations */
    {"IBA",   o_InOut,    0x31 | CPU1600},
    {"IBB",   o_InOut,    0x32 | CPU1600},
    {"IBM",   o_IBM_OBM,  0x33 | CPU1600},
    {"NOP",   o_None,     0x34 | CPU1600},
    {"OBA",   o_InOut,    0x39 | CPU1600},
    {"OBB",   o_InOut,    0x3A | CPU1600},
    {"OBM",   o_IBM_OBM,  0x3B | CPU1600},

    /* register operate */
    {"ORA", o_None,     0x40 | CPU1600},
    {"XRA", o_None,     0x41 | CPU1600},
    {"ORB", o_None,     0x42 | CPU1600},
    {"XRB", o_None,     0x43 | CPU1600},
    {"INX", o_None,     0x44 | CPU1600},
    {"DCX", o_None,     0x45 | CPU1600},
    {"AWX", o_None,     0x46 | CPU1600},
    {"SWX", o_None,     0x47 | CPU1600},
    {"INA", o_None,     0x48 | CPU1600},
    {"INB", o_None,     0x49 | CPU1600},
    {"OCA", o_None,     0x4A | CPU1600},
    {"ORB", o_None,     0x4B | CPU1600},
    {"TAX", o_None,     0x4C | CPU1600},
    {"TBX", o_None,     0x4D | CPU1600},
    {"TXA", o_None,     0x4E | CPU1600},
    {"TXB", o_None,     0x4F | CPU1600},

    {"MST", o_Imm16,    0x58 | CPU1600},
    {"ADX", o_Imm16,    0x59 | CPU1600},

    {"EBX", o_None,     0x5B | CPU1600},

    /* Stack */
    {"RTN",   o_None,     0x50 | CPU1600},
    {"RET",   o_None,     0x50 | CPU1600},
    {"CAL",   o_Bra16,    0x51 | CPU1600},
    {"CALL",  o_Bra16,    0x51 | CPU1600},
    {"PLX",   o_None,     0x52 | CPU1600},
    {"POPX",  o_None,     0x52 | CPU1600},
    {"PSX",   o_None,     0x53 | CPU1600},
    {"PUSHX", o_None,     0x53 | CPU1600},
    {"PLA",   o_None,     0x54 | CPU1600},
    {"POPA",  o_None,     0x54 | CPU1600},
    {"PSA",   o_None,     0x55 | CPU1600},
    {"PUSHA", o_None,     0x55 | CPU1600},
    {"PLB",   o_None,     0x56 | CPU1600},
    {"POPB",  o_None,     0x56 | CPU1600},
    {"PSB",   o_None,     0x57 | CPU1600},
    {"PUSHB", o_None,     0x57 | CPU1600},

    /* character/string manipulation */
    {"CLC",   o_None,     0x35 | CPU1600},
    {"MOV",   o_None,     0x5C | CPU1600},
    {"GCC",   o_None,     0x5D | CPU1600},
    {"SCH",   o_None,     0x5E | CPU1600},
    {"GAP",   o_None,     0x5F | CPU1600},

    /* Memory reference ops */
    {"JR",    o_BraRel   ,OP_JMP | 0x01 | CPU1600},
    {"JMP",   o_MemRefJump,OP_JMP | CPU1600},
    {"JMP_",  o_MemRef01  ,OP_JMP | CPU1600},

    {"RTJ",   o_MemRefJump,0x68 | CPU1600},
    {"RTJ_",  o_MemRef01  ,0x68 | CPU1600},

    {"IWM",     o_MemRef,   0x70 | CPU1600},
    {"IWM_",    o_MemRef01, 0x70 | CPU1600},

    {"DWM",     o_MemRef,   0x78 | CPU1600},
    {"DWM_",    o_MemRef01, 0x78 | CPU1600},

    {"LDX",     o_MemRef,   0x80 | CPU1600},
    {"LDX_",    o_MemRef01, 0x80 | CPU1600},

    {"STX",     o_MemRef,   0x88 | CPU1600},
    {"STX_",    o_MemRef01, 0x88 | CPU1600},

    {"LDB",     o_MemRef,   0x90 | CPU1600},
    {"LDB_",    o_MemRef01, 0x90 | CPU1600},

    {"STB",     o_MemRef,   0x98 | CPU1600},
    {"STB_",    o_MemRef01, 0x98 | CPU1600},

    {"ADA",     o_MemRef,   0xA0 | CPU1600},
    {"ADA_",    o_MemRef01, 0xA0 | CPU1600},

    {"ADV",     o_MemRefV,  0xA8 | CPU1600},
    {"ADV_",    o_MemRef01, 0xA8 | CPU1600},

    {"SBA",     o_MemRef,   0xB0 | CPU1600},
    {"SBA_",    o_MemRef01, 0xB0 | CPU1600},

    {"SBV",     o_MemRefV,  0xB8 | CPU1600},
    {"SBV_",    o_MemRef01, 0xB8 | CPU1600},

    {"CPA",     o_MemRef,   0xC0 | CPU1600},
    {"CPA_",    o_MemRef01, 0xC0 | CPU1600},

    {"CPV",     o_MemRefV,  0xC8 | CPU1600},
    {"CPV_",    o_MemRef01, 0xC8 | CPU1600},

    {"ANA",     o_MemRef,   0xD0 | CPU1600},
    {"ANA_",    o_MemRef01, 0xD0 | CPU1600},

    {"ANV",     o_MemRef,   0xD8 | CPU1600},
    {"ANV_",    o_MemRef01, 0xD8 | CPU1600},

    {"LDA",     o_MemRef,   0xE0 | CPU1600},
    {"LDA_",    o_MemRef01, 0xE0 | CPU1600},

    {"LDV",     o_MemRefV,  0xE8 | CPU1600},
    {"LDV_",    o_MemRef01, 0xE8 | CPU1600},

    {"STA",     o_MemRef,   0xF0 | CPU1600},
    {"STA_",    o_MemRef01, 0xF0 | CPU1600},

    {"STV",     o_MemRef,   0xF8 | CPU1600},
    {"STV_",    o_MemRef01, 0xF8 | CPU1600},

    {"",    o_Illegal,  0}
};

struct OpcdRec BF_opcdTab[] =
{
    {".W1", o_w1,       0x00 | CPUBF},
    {".W2", o_w2,       0x00 | CPUBF},
    {".W3", o_w3,       0x00 | CPUBF},
    {".W4", o_w4,       0x00 | CPUBF},
    {"OLDSYN", o_oldSyn,0 | CPUBF},

    {"HLT", o_None,     0x00 | CPUBF},
    {"TRP", o_None,     0x01 | CPUBF},
    {"ESW", o_None,     0x02 | CPUBF},
    {"TBA", o_None,     0x03 | CPUBF},

    {"DIN", o_None,     0x04 | CPUBF},
    {"EIN", o_None,     0x05 | CPUBF},
    {"TAB", o_None,     0x06 | CPUBF},
    {"RMV", o_None,     0x07 | CPUBF},
    {"RO1", o_None,     0x08 | CPUNO20},	// not for 1320
    {"SMT", o_None,     0x08 | CPU1320},	// 1320 only

    {"RO2", o_None,     0x09 | CPU1200},

    {"RO3", o_None,     0x0A | CPU1200},
    {"GNB", o_None,     0x0A | CPU13xx},
    {"RO4", o_None,     0x0B | CPUNO20},
    {"PJI", o_BraRel,   0x0B | CPU1320},
    {"SO1", o_None,     0x0C | CPUNO20},
    {"JIT", o_BraRel,   0x0C | CPU1320},  // ?? Jump indirect thru table
    {"SO2", o_None,     0x0D | CPU1200},
    {"BBN", o_Imm16,    0x0D | CPU1320},  // ?? Branch if byte not equal, params unknown
    {"SO3", o_None,     0x0E | CPU1200},
    {"BBE", o_Imm16,    0x0E | CPU1320},  // ?? Branch if byte equal, params unknown
    {"SO4", o_None,     0x0F | CPU1200},
    {"SDT", o_None,     0x0F | CPU1320},  // ?? String divide by 10

    /* conditional jumps, all relative 8 bit signed */
    {"JOV", o_BraRel,   0x10 | CPUBF},
    {"JAZ", o_BraRel,   0x11 | CPUBF},
    {"JBZ", o_BraRel,   0x12 | CPUBF},
    {"JXZ", o_BraRel,   0x13 | CPUBF},
    {"JAN", o_BraRel,   0x14 | CPUBF},
    {"JXN", o_BraRel,   0x15 | CPUBF},
    {"JAB", o_BraRel,   0x16 | CPUBF},
    {"JAX", o_BraRel,   0x17 | CPUBF},
    {"NOV", o_BraRel,   0x18 | CPUBF},
    {"NAZ", o_BraRel,   0x19 | CPUBF},
    {"NBZ", o_BraRel,   0x1A | CPUBF},
    {"NXZ", o_BraRel,   0x1B | CPUBF},
    {"NAN", o_BraRel,   0x1C | CPUBF},
    {"NXN", o_BraRel,   0x1D | CPUBF},
    {"NAB", o_BraRel,   0x1E | CPUBF},
    {"NAX", o_BraRel,   0x1F | CPUBF},

    {"NBN", o_BraRel,   0x37 | CPUBF},     // ?? Jump if B not negative
    {"JBN", o_BraRel,   0x38 | CPUBF},     // ?? Jump if B negative

    {"JEP", o_BraRel,   0x5A | CPUBF},

    {"RRT", o_BraRel,   0x23 | CPU1200},
    {"PLP", o_BraRel,   0x23 | CPU13xx},
    {"POPP",o_BraRel,   0x23 | CPU13xx},

    {"RCL", o_None,     0x35 | CPU1200},	// ?? recursive call
    {"PPJ", o_Imm16,    0x35 | CPU13xx},	// ?? Push P and jump

    {"DDY", o_Imm16,    0x36 | CPUBF},      // ?? Determine decimal carry

    /* shifts */
    {"LLA", o_Imm8,     0x20 | CPUBF},
    {"LLB", o_Imm8,     0x21 | CPUBF},
    {"LLL", o_Imm8,     0x22 | CPUBF},

    {"LRA", o_Imm8,     0x24 | CPUBF},
    {"LRB", o_Imm8,     0x25 | CPUBF},
    {"LRL", o_Imm8,     0x26 | CPUBF},
    {"DML", o_None,     0x27 | CPUBF},
    {"ALA", o_Imm8,     0x28 | CPUBF},
    {"ALB", o_Imm8,     0x29 | CPUBF},
    {"ALL", o_Imm8,     0x2A | CPUBF},
    {"DDV", o_None,     0x2B | CPUBF},
    {"ARA", o_Imm8,     0x2C | CPUBF},
    {"ARB", o_Imm8,     0x2D | CPUBF},
    {"ARL", o_Imm8,     0x2E | CPUBF},

    {"DDC", o_Imm8,     0x3C | CPUBF},
    {"JBX", o_BraRel,   0x3D | CPUBF},

    {"NBX", o_BraRel,   0x3E | CPUBF},   // ?? Jump if B <> X
    //{"MUL", o_Imm16,    0x3E | CPUBF},
    //{"DIV", o_Imm16,    0x3F | CPUBF},
    /* TODO: 3F instructions */

    /* input / output operations */
    {"IBA",   o_InOut,    0x31 | CPUBF},
    {"IBB",   o_InOut,    0x32 | CPUBF},
    {"IBM",   o_IBM_OBM,  0x33 | CPUBF},
    {"NOP",   o_None,     0x34 | CPUBF},
    {"OBA",   o_InOut,    0x39 | CPUBF},
    {"OBB",   o_InOut,    0x3A | CPUBF},
    {"OBM",   o_IBM_OBM,  0x3B | CPUBF},

    /* register operate */

    {"DCB", o_None,     0x2F | CPUBF},
    {"DECB",o_None,     0x2F | CPUBF},
    {"ADB", o_Imm16,    0x30 | CPUBF},	// ?? Add to B
    {"ORA", o_None,     0x40 | CPUBF},
    {"XRA", o_None,     0x41 | CPUBF},
    {"ORB", o_None,     0x42 | CPUBF},
    {"XRB", o_None,     0x43 | CPUBF},
    {"INX", o_None,     0x44 | CPUBF},
    {"INCX", o_None,    0x44 | CPUBF},
    {"DCX", o_None,     0x45 | CPUBF},
    {"DECX", o_None,     0x45 | CPUBF},
    {"AWX", o_None,     0x46 | CPU1200},
    {"PJN", o_None,     0x46 | CPU13xx},
    {"SWX", o_None,     0x47 | CPU1200},

    {"INA", o_None,     0x48 | CPUBF},
    {"INCA", o_None,     0x48 | CPUBF},
    {"INB", o_None,     0x49 | CPUBF},
    {"INCB", o_None,     0x49 | CPUBF},
    {"OCA", o_None,     0x4A | CPUBF},
    {"ORB", o_None,     0x4B | CPUBF},
    {"TAX", o_None,     0x4C | CPUBF},
    {"TBX", o_None,     0x4D | CPUBF},
    {"TXA", o_None,     0x4E | CPUBF},
    {"TXB", o_None,     0x4F | CPUBF},

    {"MST", o_None,     0x58 | CPU1200},
    {"MAB", o_Imm16,    0x58 | CPU13xx},
    {"ADX", o_Imm16,    0x59 | CPUBF},

    {"EBX", o_None,     0x5B | CPUBF},

    /* Stack */
    {"RTN",   o_None,     0x50 | CPUBF},
    {"RET",   o_None,     0x50 | CPUBF},
    {"CAL",   o_Bra16,    0x51 | CPUBF},
    {"CALL",  o_Bra16,    0x51 | CPUBF},
    {"PLX",   o_None,     0x52 | CPUBF},
    {"POPX",  o_None,     0x52 | CPUBF},
    {"PSX",   o_None,     0x53 | CPUBF},
    {"PUSHX", o_None,     0x53 | CPUBF},
    {"PLA",   o_None,     0x54 | CPUBF},
    {"POPA",  o_None,     0x54 | CPUBF},
    {"PSA",   o_None,     0x55 | CPUBF},
    {"PUSHA", o_None,     0x55 | CPUBF},
    {"PLB",   o_None,     0x56 | CPUBF},
    {"POPB",  o_None,     0x56 | CPUBF},
    {"PSB",   o_None,     0x57 | CPUBF},
    {"PUSHB", o_None,     0x57 | CPUBF},

    /* character/string manipulation */
    //{"CLC",   o_None,     0x35 | CPUBF},
    {"MOV",   o_None,     0x5C | CPUBF},
    {"GCC",   o_None,     0x5D | CPUBF},
    {"SCH",   o_None,     0x5E | CPUBF},
    {"GAP",   o_None,     0x5F | CPUBF},

    /* Memory reference ops */
    {"JR",    o_BraRel   ,OP_JMP | 0x01 | CPUBF},
    {"JMP",   o_MemRefJump,OP_JMP | CPUBF},
    {"JMP_",  o_MemRef01  ,OP_JMP | CPUBF},

    {"RTJ",   o_MemRefJump,0x68 | CPUBF},
    {"RTJ_",  o_MemRef01  ,0x68 | CPUBF},

    {"IWM",     o_MemRef,   0x70 | CPUBF},
    {"IWM_",    o_MemRef01, 0x70 | CPUBF},

    {"DWM",     o_MemRef,   0x78 | CPUBF},
    {"DWM_",    o_MemRef01, 0x78 | CPUBF},

    {"LDX",     o_MemRef,   0x80 | CPUBF},
    {"LDX_",    o_MemRef01, 0x80 | CPUBF},

    {"STX",     o_MemRef,   0x88 | CPUBF},
    {"STX_",    o_MemRef01, 0x88 | CPUBF},

    {"LDB",     o_MemRef,   0x90 | CPUBF},
    {"LDB_",    o_MemRef01, 0x90 | CPUBF},

    {"STB",     o_MemRef,   0x98 | CPUBF},
    {"STB_",    o_MemRef01, 0x98 | CPUBF},

    {"ADA",     o_MemRef,   0xA0 | CPUBF},
    {"ADA_",    o_MemRef01, 0xA0 | CPUBF},

    {"ADV",     o_MemRefV,  0xA8 | CPUBF},
    {"ADV_",    o_MemRef01, 0xA8 | CPUBF},

    {"SBA",     o_MemRef,   0xB0 | CPUBF},
    {"SBA_",    o_MemRef01, 0xB0 | CPUBF},

    {"SBV",     o_MemRefV,  0xB8 | CPUBF},
    {"SBV_",    o_MemRef01, 0xB8 | CPUBF},

    {"CPA",     o_MemRef,   0xC0 | CPUBF},
    {"CPA_",    o_MemRef01, 0xC0 | CPUBF},

    {"CPV",     o_MemRefV,  0xC8 | CPUBF},
    {"CPV_",    o_MemRef01, 0xC8 | CPUBF},

    {"ANA",     o_MemRef,   0xD0 | CPUBF},
    {"ANA_",    o_MemRef01, 0xD0 | CPUBF},

    {"ANV",     o_MemRef,   0xD8 | CPUBF},
    {"ANV_",    o_MemRef01, 0xD8 | CPUBF},

    {"LDA",     o_MemRef,   0xE0 | CPUBF},
    {"LDA_",    o_MemRef01, 0xE0 | CPUBF},

    {"LDV",     o_MemRefV,  0xE8 | CPUBF},
    {"LDV_",    o_MemRef01, 0xE8 | CPUBF},

    {"STA",     o_MemRef,   0xF0 | CPUBF},
    {"STA_",    o_MemRef01, 0xF0 | CPUBF},

    {"STV",     o_MemRef,   0xF8 | CPUBF},
    {"STV_",    o_MemRef01, 0xF8 | CPUBF},

    /* 3F 2 byte instructions */
    {"SCP0",   o_None,     0x00 | CPUBF | INST3F},
    {"SCP1",   o_None,     0x01 | CPUBF | INST3F},
    {"SCP2",   o_None,     0x02 | CPUBF | INST3F},
    {"SCP3",   o_None,     0x03 | CPUBF | INST3F},
    {"SCP4",   o_None,     0x04 | CPUBF | INST3F},
    {"SCP5",   o_None,     0x05 | CPUBF | INST3F},
    {"SCP6",   o_None,     0x06 | CPUBF | INST3F},
    {"SCP7",   o_None,     0x07 | CPUBF | INST3F},
    {"SCP8",   o_None,     0x08 | CPUBF | INST3F},
    {"SCP9",   o_None,     0x09 | CPUBF | INST3F},
    {"SCPA",   o_None,     0x0A | CPUBF | INST3F},
    {"SCPB",   o_None,     0x0B | CPUBF | INST3F},
    {"SCPC",   o_None,     0x0C | CPUBF | INST3F},
    {"SCPD",   o_None,     0x0D | CPUBF | INST3F},
    {"SCPE",   o_None,     0x0E | CPUBF | INST3F},
    {"SCPF",   o_None,     0x0F | CPUBF | INST3F},

    {"CCP0",   o_None,     0x20 | CPUBF | INST3F},
    {"CCP1",   o_None,     0x21 | CPUBF | INST3F},
    {"CCP2",   o_None,     0x22 | CPUBF | INST3F},
    {"CCP3",   o_None,     0x23 | CPUBF | INST3F},
    {"CCP4",   o_None,     0x24 | CPUBF | INST3F},
    {"CCP5",   o_None,     0x25 | CPUBF | INST3F},
    {"CCP6",   o_None,     0x26 | CPUBF | INST3F},
    {"CCP7",   o_None,     0x27 | CPUBF | INST3F},
    {"CCP8",   o_None,     0x28 | CPUBF | INST3F},
    {"CCP9",   o_None,     0x29 | CPUBF | INST3F},
    {"CCPA",   o_None,     0x2A | CPUBF | INST3F},
    {"CCPB",   o_None,     0x2B | CPUBF | INST3F},
    {"CCPC",   o_None,     0x2C | CPUBF | INST3F},
    {"CCPD",   o_None,     0x2D | CPUBF | INST3F},
    {"CCPE",   o_None,     0x2E | CPUBF | INST3F},
    {"CCPF",   o_None,     0x2F | CPUBF | INST3F},

    {"DRT",    o_None,     0x46 | CPUBF | INST3F},
    {"LEM",    o_None,     0x57 | CPU1200 | INST3F},

    {"BCS",    o_None,     0x5D | CPU1200 | INST3F},
    {"GNB",    o_None,     0x5D | CPU1300 | INST3F},
    {"HSH",    o_None,     0x5D | CPU1320 | INST3F},

    {"ERT",    o_None,     0x7E | CPUBF | INST3F},

    {"KCP0",   o_None,     0x80 | CPUBF | INST3F},
    {"KCP1",   o_None,     0x81 | CPUBF | INST3F},
    {"KCP2",   o_None,     0x82 | CPUBF | INST3F},
    {"KCP3",   o_None,     0x83 | CPUBF | INST3F},
    {"KCP4",   o_None,     0x84 | CPUBF | INST3F},
    {"KCP5",   o_None,     0x85 | CPUBF | INST3F},
    {"KCP6",   o_None,     0x86 | CPUBF | INST3F},
    {"KCP7",   o_None,     0x87 | CPUBF | INST3F},
    {"KCP8",   o_None,     0x88 | CPUBF | INST3F},
    {"KCP9",   o_None,     0x89 | CPUBF | INST3F},
    {"KCPA",   o_None,     0x8A | CPUBF | INST3F},
    {"KCPB",   o_None,     0x8B | CPUBF | INST3F},
    {"KCPC",   o_None,     0x8C | CPUBF | INST3F},
    {"KCPD",   o_None,     0x8D | CPUBF | INST3F},
    {"KCPE",   o_None,     0x8E | CPUBF | INST3F},
    {"KCPF",   o_None,     0x8F | CPUBF | INST3F},

    {"VCP0",   o_None,     0xA0 | CPU1320 | INST3F},
    {"VCP1",   o_None,     0xA1 | CPU1320 | INST3F},
    {"VCP2",   o_None,     0xA2 | CPU1320 | INST3F},
    {"VCP3",   o_None,     0xA3 | CPU1320 | INST3F},
    {"VCP4",   o_None,     0xA4 | CPU1320 | INST3F},
    {"VCP5",   o_None,     0xA5 | CPU1320 | INST3F},
    {"VCP6",   o_None,     0xA6 | CPU1320 | INST3F},
    {"VCP7",   o_None,     0xA7 | CPU1320 | INST3F},
    {"VCP8",   o_None,     0xA8 | CPU1320 | INST3F},
    {"VCP9",   o_None,     0xA9 | CPU1320 | INST3F},
    {"VCPA",   o_None,     0xAA | CPU1320 | INST3F},
    {"VCPB",   o_None,     0xAB | CPU1320 | INST3F},
    {"VCPC",   o_None,     0xAC | CPU1320 | INST3F},
    {"VCPD",   o_None,     0xAD | CPU1320 | INST3F},
    {"VCPE",   o_None,     0xAE | CPU1320 | INST3F},
    {"VCPF",   o_None,     0xAF | CPU1320 | INST3F},

    {"EEM",   o_None,     0xEB | CPU1200 | INST3F},
    {"TUM",   o_None,     0xEB | CPU1320 | INST3F},
    {"TAO",   o_None,     0xEC | CPU13xx | INST3F},
    {"IMV",   o_None,     0xED | CPU13xx | INST3F},
    {"DBS",   o_None,     0xEE | CPU13xx | INST3F},
    {"CBS",   o_None,     0xEF | CPU13xx | INST3F},

    {"DMA",   o_None,     0x00 | CPU13xx | INST47},
    {"SRE",   o_None,     0x09 | CPU13xx | INST47},
    {"SRN",   o_None,     0x0D | CPU13xx | INST47},
    {"DMS",   o_None,     0x40 | CPU13xx | INST47},
    {"DMI",   o_None,     0x60 | CPU13xx | INST47},
    {"BSA",   o_None,     0x80 | CPU13xx | INST47},
    {"BSS",   o_None,     0xC0 | CPU13xx | INST47},
    {"SMO",   o_None,     0xC1 | CPU13xx | INST47},
    {"SMX",   o_None,     0xD1 | CPU13xx | INST47},
    {"SMA",   o_None,     0xE1 | CPU13xx | INST47},

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

int MD1600_DoCPUOpcode(int typ, int parmIn)
{
    int     val,val2;
    Str255  s;
    char    *oldLine;
    int     wordLength;
    int     shortOpt;

    int parm = parmIn & 0x00ff;

    // check if opcode is supported by the selected CPU
	if (((parmIn & CPUMASK) & curCPU) == 0) return 0;

	if (parmIn & INST3F) {
		// 2 byte 3F instruction
		InstrB(0x3F);
	}
	if (parmIn & INST47) {
		// 2 byte 3F instruction
		InstrB(0x47);
	}


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
				if (curCPU == CPU1600) {
					val = val & 0x1f;  // device only
					val2 = (val2 & 0x07) << 5;
					val = val | val2;
				} else {	// AD 15.10.2025: looks like basic four is using 4 bit function code and 4 bit device address
					val = val & 0x0f;  // device only
					val2 = (val2 & 0x0f) << 4;
					val = val | val2;
				}
            }
            InstrBB(parm, val);
            break;

        // OBM 4,4,Buf
        // OBM 4,4,Buf,X
        case o_IBM_OBM:
            val = Eval();
            Expect(",");
            val2 = Eval();
            if (curCPU == CPU1600) {
				val = val & 0x1f;  // device only
				val2 = (val2 & 0x07) << 5;
				val = val | val2;
            } else {
            	val = val & 0x0f;  // device only
				val2 = (val2 & 0x0f) << 4;
				val = val | val2;
            }
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

    // Microdata 1600
    p = AddAsm(versionNameMD, &MD1600_DoCPUOpcode, NULL, NULL);
    AddCPU(p, "MD1600", CPU1600, BIG_END, ADDR_16, LIST_24, 8, 0, MD1600_opcdTab);

    // Basic Four 1200,1300 and 1320
    p = AddAsm(versionNameBF, &MD1600_DoCPUOpcode, NULL, NULL);
    AddCPU(p, "BF1200", CPU1200, BIG_END, ADDR_16, LIST_24, 8, 0, BF_opcdTab);
    AddCPU(p, "BF1300", CPU1300, BIG_END, ADDR_16, LIST_24, 8, 0, BF_opcdTab);
    AddCPU(p, "BF1320", CPU1320, BIG_END, ADDR_16, LIST_24, 8, 0, BF_opcdTab);

}
