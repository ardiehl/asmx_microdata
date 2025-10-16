// simple disassembler for microdata Micro 1600/21, Micro 821, Basic Four 1200,1300 and 1320
// Armin Diehl <ad@ardiehl.de> 13 August 2015

/* 13.08.2015 ad: initial version
 * 08.10.2015 ad: allow comments as well 0x0a as line terminator in bf boot files
 * 13.10.2025 ad: added missing opcodes for Basic Four 1200 (looks like there are some more opcodes as in the Microdata 1600 docs on bitsavers
 *                added support for Basic Four 1300 and 1320 CPU's
 * 15.10.2025 ad: added missing opcode descriptions
 *                in/out for basic four seem to has 4 bit function and device, md1600 has 3 bit function
 * 16.10.2025 ad: added second pass to generate labels for assembler out
 *                labels can be added via command line or file named inputFileName.labels
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>

#define VERMAJ 1
#define VERMIN 2
#define MYSELF "Microdata 1600/21,821, Basic Four 1200,1300 and 1320 disassembler %d.%02d, Armin Diehl <ad@ardiehl.de> October 15, 2025 (%s)\n"

// Microdata only (not Basic Four)
#define CPU1600 1
// 1200 is for basic four 1200
#define CPU1200 2
// the 13xx have some different opcodes
#define CPU1300 4
#define CPU1320 8
#define CPUALL (CPU1600+CPU1200+CPU1300+CPU1320)
#define CPU13xx (CPU1300+CPU1320)
#define CPU1213 (CPU1200+CPU1300)
#define CPU1216 (CPU1200+CPU1600)
#define CPUBF (CPU1200+CPU1300+CPU1320)
#define CPUNO20 (CPU1600+CPU1200+CPU1300)

int cpu = CPU1200;

#define maxOpcdLen  4          // max opcode length (for building opcode table)
typedef char OpcdStr[maxOpcdLen+1];
struct OpcdRec
{
    OpcdStr         name;       // opcode name
    short           typ;        // opcode type
    unsigned short  opcodeval;
    int				cpu;		// supported cpu's
    short           varlen;     // parameter is based on word length
    char * cmnt;
};
typedef struct OpcdRec *OpcdPtr;

enum
{
    o_None,         // No operands
    o_BraRel,       // short branch
    o_Bra16,        // long branch
    o_Imm8,         // one byte as parameter
    o_Imm16,        // two bytes as parameter
    o_m0,           // m=0: 8 bit addr in page 0
    o_m1,           // m=1: relative 8 bit
    o_m2,           // m=2: indirect page 0
    o_m3,           // m=3: indirect relative
    o_m4,           // m=4: indexed
    o_m5,           // m=5: indexed with bias
    o_m6,           // m=6: extended address
    o_m7,           // m=7: indirect extended address or variable length imm
    o_jm0,           // m=0: 8 bit addr in page 0
    o_jm1,           // m=1: relative 8 bit
    o_jm2,           // m=2: indirect page 0
    o_jm3,           // m=3: indirect relative
    o_jm4,           // m=4: indexed
    o_jm5,           // m=5: indexed with bias
    o_jm6,           // m=6: extended address
    o_jm7,           // m=7: indirect extended address or variable length imm
    o_inout,
    o_IBM_OBM,
    o_3F,			// 2 byte 3F instructions 13xx
    o_47			// 2 byte 47 instructions 13xx
    };

#define OP_JMP 0x60
#define OP_RETJMP 0x68

typedef struct dataSpec_t dataSpec_t;
struct dataSpec_t {
        uint16_t addr;
        uint16_t len;
        char * comment;
        dataSpec_t *next;
};

dataSpec_t *dataSpec;

int labelNameCounter;

typedef struct label_t label_t;
struct label_t {
        uint16_t addr;
        char * name;
        char * comment;
        int LabelInCode;	// 1 if label is within the disassembled code
        label_t *next;
};

label_t *labels;


struct OpcdRec* MD1600_opcdTab[256];

struct OpcdRec opcodeTable3F[] =
{
	{"SCP", o_None,     0x00, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x01, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x02, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x03, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x04, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x05, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x06, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x07, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x08, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x09, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x0A, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x0B, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x0C, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x0D, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x0E, CPUBF  , 0, "String compare"},
	{"SCP", o_None,     0x0F, CPUBF  , 0, "String compare"},
	{"CCP", o_None,     0x20, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x21, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x22, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x23, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x24, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x25, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x26, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x27, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x28, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x29, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x2A, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x2B, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x2C, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x2D, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x2E, CPUBF  , 0, "Character compare"},
	{"CCP", o_None,     0x2F, CPUBF  , 0, "Character compare"},
	{"DRT", o_None,     0x46, CPUBF  , 0, "Disable real time clock"},
	{"LEM", o_None,     0x57, CPU1200, 0, "Leave extended memory"},
	{"BCS", o_None,     0x5d, CPU1200, 0, "Blank character scan"},
	{"GNB", o_None,     0x5d, CPU13xx, 0, "Get next byte (no docs)"},
	{"ERT", o_None,     0x7e, CPU13xx, 0, "Enable real time clock"},
	{"KCP", o_None,     0x80, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x81, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x82, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x83, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x84, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x85, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x86, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x87, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x88, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x89, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x8A, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x8B, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x8C, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x8D, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x8E, CPUBF,   0, "Key compare (no docs)"},
	{"KCP", o_None,     0x8F, CPUBF,   0, "Key compare (no docs)"},
	{"VCP", o_None,     0xA0, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA1, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA2, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA3, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA4, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA5, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA6, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA7, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA8, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xA9, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xAA, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xAB, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xAC, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xAD, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xAE, CPU1320, 0, "Variable key compare (no docs)"},
	{"VCP", o_None,     0xAF, CPU1320, 0, "Variable key compare (no docs)"},
	{"DGT", o_None,     0xC2, CPUBF,   0, "Digit test (no docs)"},
	{"AVB", o_None,     0xCF, CPUBF,   0, "Divide unsigned A by B (no docs)"},
	{"LLT", o_None,     0xDB, CPUBF,   0, "Letter test (no docs)"},
	{"SIS", o_None,     0xEA, CPU13xx, 0, "Strobe internal status (no docs)"},
	{"EEM", o_None,     0xEB, CPUBF,   0, "Enter extended mem (no docs)"},
	{"TUM", o_None,     0xEB, CPU1320, 0, "Tie up memory (no docs)"},
	{"TAO", o_None,     0xEC, CPU13xx, 0, "Transfer A to O (no docs)"},
	{"INV", o_None,     0xED, CPU13xx, 0, "Interbank move (no docs)"},
	{"DBS", o_None,     0xEE, CPU13xx, 0, "DMA memory bank select (no docs)"},
	{"CBS", o_None,     0xEF, CPU13xx, 0, "CPU memory bank select (no docs)"},
	{""}
};

struct OpcdRec opcodeTable47[] =
{
	{"DMA", o_None,     0x00, CPU13xx, 0, "Decimal multiply and add (no docs)"},
	{"SRE", o_None,     0x09, CPU13xx, 0, "String range equal (no docs)"},
	{"SRN", o_None,     0x0D, CPU13xx, 0, "String range not equal (no docs)"},
	{"DMS", o_None,     0x40, CPU13xx, 0, "Decimal multiply & sub (no docs)"},
	{"DMI", o_None,     0x60, CPU13xx, 0, "Decimal multiply & invert (no docs)"},
	{"BSA", o_None,     0x80, CPU13xx, 0, "Binary string add (no docs)"},
	{"BSS", o_None,     0xC0, CPU13xx, 0, "Binary string substract (no docs)"},
	{"SMO", o_None,     0xC1, CPU13xx, 0, "String or (no docs)"},
	{"SMX", o_None,     0xD1, CPU13xx, 0, "String mask exclusive or (no docs)"},
	{"SMA", o_None,     0xE1, CPU13xx, 0, "String mask end (no docs)"}
};

struct OpcdRec opcodeTable[] =
{
    {"HLT", o_None,     0x00, CPUALL , 0, "Halt cpu"},
    {"TRP", o_None,     0x01, CPUALL , 0, "Trap 16 Bit JMP, save P to [*80], jump to [*82]"},
    {"ESW", o_None,     0x02, CPUBF  , 0, "Load sense switches to A(12-15)"},
    {"ESW", o_None,     0x03, CPU1600, 0, "Load sense switches to A(12-15)"},
    {"PMP", o_None,     0x03, CPU1600, 0, "Protect Memory Page"},
    {"TBA", o_None,     0x03, CPUBF  , 0, "Transfer B to A"},
    {"DIN", o_None,     0x04, CPUALL , 0, "Disable interrupts"},
    {"EIN", o_None,     0x05, CPUALL , 0, "Enable interrupts"},
    {"DRT", o_None,     0x06, CPU1600, 0, "Disable realtime clock"},
    {"TAB", o_None,     0x06, CPUBF  , 0, "Transfer A to B"},
    {"ERT", o_None,     0x07, CPU1600, 0, "Enable realtime clock"},
    {"RMV", o_None,     0x07, CPUBF  , 0, "Reverse move (no docs)"},
    {"RO1", o_None,     0x08, CPUNO20, 0, "Reset overflow and word length 1"},  // Microdata, 1200 and 1300
    {"SMT", o_None,     0x08, CPU1320, 0, "String multiply by 10 (no docs)"},	// 1320 only
    {"RO2", o_None,     0x09, CPU1216, 0, "Reset overflow and word length 2"},	// 1200 only
    {"RO3", o_None,     0x0A, CPU1216, 0, "Reset overflow and word length 3"},
    {"GNB", o_None,     0x0A, CPU13xx, 0, "Get next byte (no docs)"},
    {"RO4", o_None,     0x0B, CPUNO20, 0, "Reset overflow and word length 4"},
    {"PJI", o_BraRel,   0x0B, CPU1320, 0, "Push P and jump"},
    {"SO1", o_None,     0x0C, CPUNO20, 0, "Set overflow and word length 1"},
    {"JIT", o_BraRel,   0x0C, CPU1320, 0, "Jump indirect thru table (no docs)"},
    {"SO2", o_None,     0x0D, CPU1216, 0, "Set overflow and word length 2"},
    {"BBN", o_Imm16,    0x0D, CPU13xx, 0, "Branch if byte not equal (no docs)"},
    {"SO3", o_None,     0x0E, CPU1216, 0, "Set overflow and word length 3"},
    {"BBE", o_Imm16,    0x0E, CPU13xx, 0, "Branch if byte equal (no docs)"},
    {"SO4", o_None,     0x0F, CPU1216, 0, "Set overflow and word length 4"},
    {"SDT", o_None,     0x0F, CPU13xx, 0, "String divide by 10 (no docs)"},

    {"JOV", o_BraRel,   0x10, CPUALL , 0, "Jump if overlow set"},
    {"JAZ", o_BraRel,   0x11, CPUALL , 0, "Jump if A = 0"},
    {"JBZ", o_BraRel,   0x12, CPUALL , 0, "Jump if B = 0"},
    {"JXZ", o_BraRel,   0x13, CPUALL , 0, "Jump if X = 0"},
    {"JAN", o_BraRel,   0x14, CPUALL , 0, "Jump if A < 0"},
    {"JXN", o_BraRel,   0x15, CPUALL , 0, "Jump if X < 0"},
    {"JAB", o_BraRel,   0x16, CPUALL , 0, "Jump if A = B"},
    {"JAX", o_BraRel,   0x17, CPUALL , 0, "Jump if A = X"},
    {"NOV", o_BraRel,   0x18, CPUALL , 0, "Jump if no overflow"},
    {"NAZ", o_BraRel,   0x19, CPUALL , 0, "Jump if A <> 0"},
    {"NBZ", o_BraRel,   0x1A, CPUALL , 0, "Jump if B <> 0"},
    {"NXZ", o_BraRel,   0x1B, CPUALL , 0, "Jump if X <> 0"},
    {"NAN", o_BraRel,   0x1C, CPUALL , 0, "Jump if A >= 0"},
    {"NXN", o_BraRel,   0x1D, CPUALL , 0, "Jump if X >= 0"},
    {"NAB", o_BraRel,   0x1E, CPUALL , 0, "Jump if A <> B"},
    {"NAX", o_BraRel,   0x1F, CPUALL , 0, "Jump if A <> X"},
    /* shifts */
    {"LLA", o_Imm8,     0x20, CPUALL ,0, "Shift A left, bits shifted out of A15 are shifted into A0"},
    {"LLB", o_Imm8,     0x21, CPUALL ,0, "Shift B left, bits shifted out of A15 are shifted into A0"},
    {"LLL", o_Imm8,     0x22, CPUALL ,0, "Shift AB left, bits shifted out of B15 are shifted into A0"},
    {"RRT", o_None,     0x23, CPU1216,0, "recursive return"},
    {"PLP", o_None,     0x23, CPU13xx,0, "Pull P"},

    {"LRA", o_Imm8,     0x24, CPUALL ,0, "Shift A right, bits shifted out are lost"},
    {"LRB", o_Imm8,     0x25, CPUALL ,0, "Shift B right, bits shifted out are lost"},
    {"LRL", o_Imm8,     0x26, CPUALL ,0, "Shift AB right, bits shifted out are lost"},
    {"DML", o_None,     0x27, CPUBF  ,0, "Decimal digit multiply (no docs)"},

    {"ALA", o_Imm8,     0x28, CPUALL ,0, "Shift A left, bits shifted out are lost"},
    {"ALB", o_Imm8,     0x29, CPUALL ,0, "Shift B left, bits shifted out are lost"},
    {"ALL", o_Imm8,     0x2A, CPUALL ,0, "Shift AB left, bits shifted out are lost"},
    {"DDV", o_None,     0x2B, CPUBF , 0, "Decimal digit divide (Basic Four, no docs)"},
    {"ARA", o_Imm8,     0x2C, CPUALL ,0, "Shift A right, A15 remains unchanged"},
    {"ARB", o_Imm8,     0x2D, CPUALL ,0, "Shift B right, A15 remains unchanged"},
    {"ARL", o_Imm8,     0x2E, CPUALL ,0, "Shift AB right, A15 remains unchanged"},
    {"DCB", o_None,     0x2F, CPUALL, 0, "Decrement B"},
    {"ADB", o_Imm16,    0x30, CPUBF , 0, "Add to B"},


    /* input / output operations */
    {"IBA", o_inout,    0x31, CPUALL ,0, "Input byte to A"},
    {"IBB", o_inout,    0x32, CPUALL ,0, "Input byte to B"},
    {"IBM", o_IBM_OBM,  0x33, CPUALL ,0, "Input byte to memory"},
    {"NOP", o_None,     0x34, CPUALL, 0},

    {"CLC", o_None,     0x35, CPU1600 ,0, "compare string, while [A]==[X] && X<=B"},	// Microdata only
    {"RCL", o_None,     0x35, CPU1200 ,0, "Recursive call (no docs)"},					// Basic Four 1200 only
    {"PPJ", o_Imm16,    0x35, CPU13xx ,0, "Push P and jump (no Docs)"},

    {"DDJ", o_Imm16,    0x36, CPUBF   ,0, "Determine decimal carry (no docs)"},
    {"NBN", o_Imm16,    0x37, CPUBF   ,0, "Jump if B not negative (no docs)"},
    {"JBN", o_Imm16,    0x38, CPUBF   ,0, "Jump if B negative (no docs)"},

    {"OBA", o_inout,    0x39, CPUALL ,0, "output byte from A"},
    {"OBB", o_inout,    0x3A, CPUALL ,0, "output byte from B"},
    {"OBM", o_IBM_OBM,  0x3B, CPUALL ,0, "output byte from memory"},
    {"DAD", o_None,     0x3C, CPU1600,0, "Decimal add, (X)=(X)+(B), variable, len in A"},
    {"DDC", o_Imm8,     0x3C, CPUBF  ,0, "Decimal digit conversion (no docs)"},
    {"DSB", o_None,     0x3D, CPU1600,0, "Decimal sub, (X)=(X)-(B), variable, len in A"},
    {"JBX", o_Imm8,     0x3D, CPUBF  ,0, "Jump if B = X (no docs)"},

    {"MUL", o_Imm16,    0x3E, CPU1600,0, "AB = A * param[16]"},
    {"NBX", o_Imm8,     0x3E, CPUBF  ,0, "Jump if B <> X (no docs)"},

    {"DIV", o_Imm16,    0x3F, CPU1600,0, "B=AB/param[16],A=AB MOD param[16]"},
    {"?3F", o_3F   ,    0x3f, CPUBF  ,0, ""}, // TODO: add 2 byte opcodes starting with 3F

    /* register operate */
    {"ORA", o_None,     0x40, CPUALL ,0, "A=A or B"},
    {"XRA", o_None,     0x41, CPUALL ,0, "A=A xor B"},
    {"ORB", o_None,     0x42, CPUALL ,0, "B=A or B"},
    {"XRB", o_None,     0x43, CPUALL ,0, "B=A xor B"},
    {"INX", o_None,     0x44, CPUALL ,0, "increment X"},
    {"DCX", o_None,     0x45, CPUALL ,0, "decrement X"},

    {"AWX", o_None,     0x46, CPU1216,0, "add current word length to X"},
    {"PJN", o_None,     0x46, CPU13xx,0, "Push P and jump using next (no docs)"},

    {"SWX", o_None,     0x47, CPU1216,0, "substract current word length from X"},
    {"?47", o_47   ,    0x47, CPUBF  ,0, ""}, // TODO: add 2 byte opcodes starting with 47
    {"INA", o_None,     0x48, CPUALL ,0, "Increment A"},
    {"INB", o_None,     0x49, CPUALL ,0, "Increment B"},
    {"OCA", o_None,     0x4A, CPUALL ,0, "Ones complement A, A = not A"},
    {"ORB", o_None,     0x4B, CPUALL ,0, "Ones complement B, B = not B"},
    {"TAX", o_None,     0x4C, CPUALL ,0, "Transfer A to X, X = A"},
    {"TBX", o_None,     0x4D, CPUALL ,0, "Transfer B to X, X = B"},
    {"TXA", o_None,     0x4E, CPUALL ,0, "Transfer X to A, A = X"},
    {"TXB", o_None,     0x4F, CPUALL ,0, "Transfer X to B, B = X"},

    /* Stack */
    {"RTN", o_None,     0x50, CPUALL ,0, "Return: pop O/W[8],P,B,A,X (9 bytes) and jump to P"},
    {"CAL", o_Bra16,    0x51, CPUALL ,0, "Call: push X,A,B,P,O/W[8] (9 bytes) and jump"},
    {"PLX", o_None,     0x52, CPUALL ,0, "Pull X from stack"},
    {"PSX", o_None,     0x53, CPUALL ,0, "Push X to stack"},
    {"PLA", o_None,     0x54, CPUALL ,0, "Pull A from stack"},
    {"PSA", o_None,     0x55, CPUALL ,0, "Push A to stack"},
    {"PLB", o_None,     0x56, CPUALL ,0, "Pull B from stack"},
    {"PSB", o_None,     0x57, CPUALL ,0, "Push B to stack"},

    {"MST", o_Imm16,    0x58, CPU1216,0, "Mult Step"},
    {"MAB", o_None,     0x58, CPU13xx,0, "Multiply A by B (no docs)"},

    {"ADX", o_Imm16,    0x59, CPUALL ,0, "Add to X"},
    {"JEP", o_BraRel,   0x5A, CPUALL ,0, "Jump if even parity (A contains even number of 1 bits"},
    {"EBX", o_None,     0x5B, CPUALL ,0, "Exchange B and X"},

    /* character/string manipulation */

    {"MOV",   o_None,     0x5C, CPUALL ,0, "move [X] to [A], end address (of X) in B"},
    {"GCC",   o_None,     0x5D, CPUALL ,0, "generate 16 bit CRC in A, [X]=src,B=end addr"},
    {"SCH",   o_None,     0x5E, CPUALL ,0, "Search [X] (terminated with 0) at list [A] where list = (compValue,jumpAddr,..,0x00,0x0000) while X <= B"},
    {"GAP",   o_None,     0x5F, CPUALL ,0, "Gernerate ascii parity, src=[X] while X <= B"},

    /* Memory reference ops */
    {"JMP",   o_jm0,  0x60, CPUALL ,0, "Jump - direct page 0"},
    {"JMP",   o_jm1,  0x61, CPUALL ,0, "Jump - direct relative"},
    {"JMP",   o_jm2,  0x62, CPUALL ,0, "Jump - indirect page 0"},
    {"JMP",   o_jm3,  0x63, CPUALL ,0, "Jump - indirect raltive"},
    {"JMP",   o_jm4,  0x64, CPU1216,0, "Jump - indexed"},
    {"JMP",   o_jm4,  0x64, CPU13xx,0, "Jump - T-index + bias (no docs)"},
    {"JMP",   o_jm5,  0x65, CPUALL ,0, "Jump - indexed + bias"},
    {"JMP",   o_jm6,  0x66, CPUALL ,0, "Jump - extended address"},
    {"JMP",   o_jm7,  0x67, CPUALL ,0, "Jump - literal"},

    {"RTJ",  o_jm0,  0x68, CPUALL ,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - direct page 0"},
    {"RTJ",  o_jm1,  0x69, CPUALL ,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - direct relative"},
    {"RTJ",  o_jm2,  0x6A, CPUALL ,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - indirect page 0"},
    {"RTJ",  o_jm3,  0x6B, CPUALL ,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - indirect relative"},
    {"RTJ",  o_jm4,  0x6C, CPU1216,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - indexed"},
    {"RTJ",  o_jm4,  0x6C, CPU13xx,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - T-index + bias (no docs)"},
    {"RTJ",  o_jm5,  0x6D, CPUALL ,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - indexed + bias"},
    {"RTJ",  o_jm6,  0x6E, CPUALL ,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - extended address"},
    {"RTJ",  o_jm7,  0x6F, CPUALL ,0, "Return Jump, save retAaddr at effective addr and jump to the eff. addr+2 - literal"},

    {"IWM",  o_m0,  0x70, CPUALL ,0, "Increment word in memory - direct page 0"},
    {"IWM",  o_m1,  0x71, CPUALL ,0, "Increment word in memory - direct relative"},
    {"IWM",  o_m2,  0x72, CPUALL ,0, "Increment word in memory - indirect page 0"},
    {"IWM",  o_m3,  0x73, CPUALL ,0, "Increment word in memory - indirect relative"},
    {"IWM",  o_m4,  0x74, CPU1216,0, "Increment word in memory - indexed"},
    {"IWM",  o_m4,  0x74, CPU13xx,0, "Increment word in memory - T-index + bias (no docs)"},
    {"IWM",  o_m5,  0x75, CPUALL ,0, "Increment word in memory - indexed + bias"},
    {"IWM",  o_m6,  0x76, CPUALL ,0, "Increment word in memory - extended address"},
    {"IWM",  o_m7,  0x77, CPUALL ,0, "Increment word in memory - literal"},


    {"DWM",  o_m0,  0x78, CPUALL ,0, "Decrement word in memory - direct page 0"},
    {"DWM",  o_m1,  0x79, CPUALL ,0, "Decrement word in memory - direct relative"},
    {"DWM",  o_m2,  0x7A, CPUALL ,0, "Decrement word in memory - indirect page 0"},
    {"DWM",  o_m3,  0x7B, CPUALL ,0, "Decrement word in memory - indirect relative"},
    {"DWM",  o_m4,  0x7C, CPU1216,0, "Decrement word in memory - indexed"},
    {"DWM",  o_m4,  0x7C, CPU13xx,0, "Decrement word in memory - T-index + bias (no docs)"},
    {"DWM",  o_m5,  0x7D, CPUALL ,0, "Decrement word in memory - indexed + bias"},
    {"DWM",  o_m6,  0x7E, CPUALL ,0, "Decrement word in memory - extended address"},
    {"DWM",  o_m7,  0x7F, CPUALL ,0, "Decrement word in memory - literal"},

    {"LDX",  o_m0,  0x80, CPUALL ,0, "Load 16 bit word to X - direct page 0"},
    {"LDX",  o_m1,  0x81, CPUALL ,0, "Load 16 bit word to X - direct relative"},
    {"LDX",  o_m2,  0x82, CPUALL ,0, "Load 16 bit word to X - indirect page 0"},
    {"LDX",  o_m3,  0x83, CPUALL ,0, "Load 16 bit word to X - indirect relative"},
    {"LDX",  o_m4,  0x84, CPU1216,0, "Load 16 bit word to X - indexed"},
    {"LDX",  o_m4,  0x84, CPU13xx,0, "Load 16 bit word to X - T-index + bias (no docs)"},
    {"LDX",  o_m5,  0x85, CPUALL ,0, "Load 16 bit word to X - indexed + bias"},
    {"LDX",  o_m6,  0x86, CPUALL ,0, "Load 16 bit word to X - extended address"},
    {"LDX",  o_m7,  0x87, CPUALL ,0, "Load 16 bit word to X - literal"},

    {"STX",  o_m0,  0x88, CPUALL ,0, "Store X to memory - direct page 0"},
    {"STX",  o_m1,  0x89, CPUALL ,0, "Store X to memory - direct relative"},
    {"STX",  o_m2,  0x8A, CPUALL ,0, "Store X to memory - indirect page 0"},
    {"STX",  o_m3,  0x8B, CPUALL ,0, "Store X to memory - indirect relative"},
    {"STX",  o_m4,  0x8C, CPU1216,0, "Store X to memory - indexed"},
    {"STX",  o_m4,  0x8C, CPU13xx,0, "Store X to memory - T-index + bias (no docs)"},
    {"STX",  o_m5,  0x8D, CPUALL ,0, "Store X to memory - indexed + bias"},
    {"STX",  o_m6,  0x8E, CPUALL ,0, "Store X to memory - extended address"},
    {"STX",  o_m7,  0x8F, CPUALL ,0, "Store X to memory - literal"},

    {"LDB",  o_m0,  0x90, CPUALL ,0, "Load 16 bit word to B - direct page 0"},
    {"LDB",  o_m1,  0x91, CPUALL ,0, "Load 16 bit word to B - direct relative"},
    {"LDB",  o_m2,  0x92, CPUALL ,0, "Load 16 bit word to B - indirect page 0"},
    {"LDB",  o_m3,  0x93, CPU1216,0, "Load 16 bit word to B - indirect relative"},
    {"LDB",  o_m4,  0x94, CPU13xx,0, "Load 16 bit word to B - indexed"},
    {"LDB",  o_m4,  0x94, CPUALL ,0, "Load 16 bit word to B - T-index + bias (no docs)"},
    {"LDB",  o_m5,  0x95, CPUALL ,0, "Load 16 bit word to B - indexed + bias"},
    {"LDB",  o_m6,  0x96, CPUALL ,0, "Load 16 bit word to B - extended address"},
    {"LDB",  o_m7,  0x97, CPUALL ,0, "Load 16 bit word to B - literal"},

    {"STB",  o_m0,  0x98, CPUALL ,0, "Store B (16 Bit) to memory - direct page 0"},
    {"STB",  o_m1,  0x99, CPUALL ,0, "Store B (16 Bit) to memory - direct relative"},
    {"STB",  o_m2,  0x9A, CPUALL ,0, "Store B (16 Bit) to memory - indirect page 0"},
    {"STB",  o_m3,  0x9B, CPUALL ,0, "Store B (16 Bit) to memory - indirect relative"},
    {"STB",  o_m4,  0x9C, CPU1216,0, "Store B (16 Bit) to memory - indexed"},
    {"STB",  o_m4,  0x9C, CPU13xx,0, "Store B (16 Bit) to memory - T-index + bias (no docs)"},
    {"STB",  o_m5,  0x9D, CPUALL ,0, "Store B (16 Bit) to memory - indexed + bias"},
    {"STB",  o_m6,  0x9E, CPUALL ,0, "Store B (16 Bit) to memory - extended address"},
    {"STB",  o_m7,  0x9F, CPUALL ,0, "Store B (16 Bit) to memory - literal"},

    {"ADA",  o_m0,  0xA0, CPUALL ,0, "Add to A - direct page 0"},
    {"ADA",  o_m1,  0xA1, CPUALL ,0, "Add to A - direct relative"},
    {"ADA",  o_m2,  0xA2, CPUALL ,0, "Add to A - indirect page 0"},
    {"ADA",  o_m3,  0xA3, CPUALL ,0, "Add to A - indirect relative"},
    {"ADA",  o_m4,  0xA4, CPU1216,0, "Add to A - indexed"},
    {"ADA",  o_m4,  0xA4, CPU13xx,0, "Add to A - T-index + bias (no docs)"},
    {"ADA",  o_m5,  0xA5, CPUALL ,0, "Add to A - indexed + bias"},
    {"ADA",  o_m6,  0xA6, CPUALL ,0, "Add to A - extended address"},
    {"ADA",  o_m7,  0xA7, CPUALL ,0, "Add to A - literal"},

    {"ADV",  o_m0,  0xA8, CPUALL , 1, "Add variable length to A or AB - direct page 0"},
    {"ADV",  o_m1,  0xA9, CPUALL , 1, "Add variable length to A or AB - direct relative"},
    {"ADV",  o_m2,  0xAA, CPUALL , 1, "Add variable length to A or AB - indirect page 0"},
    {"ADV",  o_m3,  0xAB, CPUALL , 1, "Add variable length to A or AB - indirect relative"},
    {"ADV",  o_m4,  0xAC, CPU1216, 1, "Add variable length to A or AB - indexed"},
    {"ADV",  o_m4,  0xAC, CPU13xx, 1, "Add variable length to A or AB - T-index + bias (no docs)"},
    {"ADV",  o_m5,  0xAD, CPUALL , 1, "Add variable length to A or AB - indexed + bias"},
    {"ADV",  o_m6,  0xAE, CPUALL , 1, "Add variable length to A or AB - extended address"},
    {"ADV",  o_m7,  0xAF, CPUALL , 1, "Add variable length to A or AB - literal"},

    {"SBA",  o_m0,  0xB0, CPUALL ,0, "Sub from A - direct page 0"},
    {"SBA",  o_m1,  0xB1, CPUALL ,0, "Sub from A - direct relative"},
    {"SBA",  o_m2,  0xB2, CPUALL ,0, "Sub from A - indirect page 0"},
    {"SBA",  o_m3,  0xB3, CPUALL ,0, "Sub from A - indirect relative"},
    {"SBA",  o_m4,  0xB4, CPU1216,0, "Sub from A - indexed"},
    {"SBA",  o_m4,  0xB4, CPU13xx,0, "Sub from A - T-index + bias (no docs)"},
    {"SBA",  o_m5,  0xB5, CPUALL ,0, "Sub from A - indexed + bias"},
    {"SBA",  o_m6,  0xB6, CPUALL ,0, "Sub from A - extended address"},
    {"SBA",  o_m7,  0xB7, CPUALL ,0, "Sub from A - literal"},

    {"SBV",  o_m0,  0xB8, CPUALL , 1, "Sub variable length from A or AB - direct page 0"},
    {"SBV",  o_m1,  0xB9, CPUALL , 1, "Sub variable length from A or AB - direct relative"},
    {"SBV",  o_m2,  0xBA, CPUALL , 1, "Sub variable length from A or AB - indirect page 0"},
    {"SBV",  o_m3,  0xBB, CPUALL , 1, "Sub variable length from A or AB - indirect relative"},
    {"SBV",  o_m4,  0xBC, CPU1216, 1, "Sub variable length from A or AB - indexed"},
    {"SBV",  o_m4,  0xBC, CPU13xx, 1, "Sub variable length from A or AB - T-index + bias (no docs)"},
    {"SBV",  o_m5,  0xBD, CPUALL , 1, "Sub variable length from A or AB - indexed + bias"},
    {"SBV",  o_m6,  0xBE, CPUALL , 1, "Sub variable length from A or AB - extended address"},
    {"SBV",  o_m7,  0xBF, CPUALL , 1, "Sub variable length from A or AB - literal"},

    {"CPA",  o_m0,  0xC0, CPUALL ,0, "Compare A with 16 bit word - direct page 0"},
    {"CPA",  o_m1,  0xC1, CPUALL ,0, "Compare A with 16 bit word - direct relative"},
    {"CPA",  o_m2,  0xC2, CPUALL ,0, "Compare A with 16 bit word - indirect page 0"},
    {"CPA",  o_m3,  0xC3, CPUALL ,0, "Compare A with 16 bit word - indirect relative"},
    {"CPA",  o_m4,  0xC4, CPU1216,0, "Compare A with 16 bit word - indexed"},
    {"CPA",  o_m4,  0xC4, CPU13xx,0, "Compare A with 16 bit word - T-index + bias (no docs)"},
    {"CPA",  o_m5,  0xC5, CPUALL ,0, "Compare A with 16 bit word - indexed + bias"},
    {"CPA",  o_m6,  0xC6, CPUALL ,0, "Compare A with 16 bit word - extended address"},
    {"CPA",  o_m7,  0xC7, CPUALL ,0, "Compare A with 16 bit word - literal"},

    {"CPV",  o_m0,  0xC8, CPUALL , 1, "Compare A or AB with given data (variable length) - direct page 0"},
    {"CPV",  o_m1,  0xC9, CPUALL , 1, "Compare A or AB with given data (variable length) - direct relative"},
    {"CPV",  o_m2,  0xCA, CPUALL , 1, "Compare A or AB with given data (variable length) - indirect page 0"},
    {"CPV",  o_m3,  0xCB, CPUALL , 1, "Compare A or AB with given data (variable length) - indirect relative"},
    {"CPV",  o_m4,  0xCC, CPUALL , 1, "Compare A or AB with given data (variable length) - indexed"},
    {"CPV",  o_m4,  0xCC, CPU1216, 1, "Compare A or AB with given data (variable length) - T-index + bias (no docs)"},
    {"CPV",  o_m5,  0xCD, CPU13xx, 1, "Compare A or AB with given data (variable length) - indexed + bias"},
    {"CPV",  o_m6,  0xCE, CPUALL , 1, "Compare A or AB with given data (variable length) - extended address"},
    {"CPV",  o_m7,  0xCF, CPUALL , 1, "Compare A or AB with given data (variable length) - literal"},

    {"ANA",  o_m0,  0xD0, CPUALL ,0, "16 Bit AND - direct page 0"},
    {"ANA",  o_m1,  0xD1, CPUALL ,0, "16 Bit AND - direct relative"},
    {"ANA",  o_m2,  0xD2, CPUALL ,0, "16 Bit AND - indirect page 0"},
    {"ANA",  o_m3,  0xD3, CPUALL ,0, "16 Bit AND - indirect relative"},
    {"ANA",  o_m4,  0xD4, CPU1216,0, "16 Bit AND - indexed"},
    {"ANA",  o_m4,  0xD4, CPU13xx,0, "16 Bit AND - T-index + bias (no docs)"},
    {"ANA",  o_m5,  0xD5, CPUALL ,0, "16 Bit AND - indexed + bias"},
    {"ANA",  o_m6,  0xD6, CPUALL ,0, "16 Bit AND - extended address"},
    {"ANA",  o_m7,  0xD7, CPUALL ,0, "16 Bit AND - literal"},

    {"ANV",  o_m0,  0xD8, CPUALL , 1, "A or AB, AND variable length - direct page 0"},
    {"ANV",  o_m1,  0xD9, CPUALL , 1, "A or AB, AND variable length - direct relative"},
    {"ANV",  o_m2,  0xDA, CPUALL , 1, "A or AB, AND variable length - indirect page 0"},
    {"ANV",  o_m3,  0xDB, CPUALL , 1, "A or AB, AND variable length - indirect relative"},
    {"ANV",  o_m4,  0xDC, CPU1216, 1, "A or AB, AND variable length - indexed"},
    {"ANV",  o_m4,  0xDC, CPU13xx, 1, "A or AB, AND variable length - T-index + bias (no docs)"},
    {"ANV",  o_m5,  0xDD, CPUALL , 1, "A or AB, AND variable length - indexed + bias"},
    {"ANV",  o_m6,  0xDE, CPUALL , 1, "A or AB, AND variable length - extended address"},
    {"ANV",  o_m7,  0xDF, CPUALL , 1, "A or AB, AND variable length - literal"},

    {"LDA",  o_m0,  0xE0, CPUALL ,0, "Load 16 bit into A - direct page 0"},
    {"LDA",  o_m1,  0xE1, CPUALL ,0, "Load 16 bit into A - direct relative"},
    {"LDA",  o_m2,  0xE2, CPUALL ,0, "Load 16 bit into A - indirect page 0"},
    {"LDA",  o_m3,  0xE3, CPUALL ,0, "Load 16 bit into A - indirect relative"},
    {"LDA",  o_m4,  0xE4, CPU1216,0, "Load 16 bit into A - indexed"},
    {"LDA",  o_m4,  0xE4, CPU13xx,0, "Load 16 bit into A - T-Base indexed+bias"},
    {"LDA",  o_m5,  0xE5, CPUALL ,0, "Load 16 bit into A - indexed+bias"},
    {"LDA",  o_m6,  0xE6, CPUALL ,0, "Load 16 bit into A - extended address"},
    {"LDA",  o_m7,  0xE7, CPUALL ,0, "Load 16 bit into A - literal"},

    {"LDV",  o_m0,  0xE8, CPUALL , 1, "load variable length into A or AB - Direct page 0"},
    {"LDV",  o_m1,  0xE9, CPUALL , 1, "load variable length into A or AB - Direct relative"},
    {"LDV",  o_m2,  0xEA, CPUALL , 1, "load variable length into A or AB - Indirect page 0"},
    {"LDV",  o_m3,  0xEB, CPUALL , 1, "load variable length into A or AB - Indirect relative"},
    {"LDV",  o_m4,  0xEC, CPU1216, 1, "load variable length into A or AB - Indexed"},
    {"LDV",  o_m4,  0xEC, CPU13xx, 1, "load variable length into A or AB - T-Base indexed+bias"},
    {"LDV",  o_m5,  0xED, CPUALL , 1, "load variable length into A or AB - Indexed+bias"},
    {"LDV",  o_m6,  0xEE, CPUALL , 1, "load variable length into A or AB - Extended address"},
    {"LDV",  o_m7,  0xEF, CPUALL , 1, "load variable length into A or AB - Literal"},

    {"STA",  o_m0,  0xF0, CPUALL ,0, "Save A (16 bit) to given address - direct page 0"},
    {"STA",  o_m1,  0xF1, CPUALL ,0, "Save A (16 bit) to given address - direct relative"},
    {"STA",  o_m2,  0xF2, CPUALL ,0, "Save A (16 bit) to given address - indirect page 0"},
    {"STA",  o_m3,  0xF3, CPUALL ,0, "Save A (16 bit) to given address - indirect relative"},
    {"STA",  o_m4,  0xF4, CPU1216,0, "Save A (16 bit) to given address - indexed"},
    {"STA",  o_m4,  0xF4, CPU13xx,0, "Save A (16 bit) to given address - T-Base indexed+bias (no docs)"},
    {"STA",  o_m5,  0xF5, CPUALL ,0, "Save A (16 bit) to given address - indexed+bias"},
    {"STA",  o_m6,  0xF6, CPUALL ,0, "Save A (16 bit) to given address - extended address"},
    {"STA",  o_m7,  0xF7, CPUALL ,0, "Save A (16 bit) to given address - literal"},

    {"STV",  o_m0,  0xF8, CPUALL ,0, "Store variable A or AB to given address - direct page 0"},
    {"STV",  o_m1,  0xF9, CPUALL ,0, "Store variable A or AB to given address - direct relative"},
    {"STV",  o_m2,  0xFA, CPUALL ,0, "Store variable A or AB to given address - indirect page 0"},
    {"STV",  o_m3,  0xFB, CPUALL ,0, "Store variable A or AB to given address - indirect relative"},
    {"STV",  o_m4,  0xFC, CPU1216,0, "Store variable A or AB to given address - indexed"},
    {"STV",  o_m4,  0xFC, CPU13xx,0, "Store variable A or AB to given address - T-Base indexed+bias (no docs)"},
    {"STV",  o_m5,  0xFD, CPUALL ,0, "Store variable A or AB to given address - indexed+bias"},
    {"STV",  o_m6,  0xFE, CPUALL ,0, "Store variable A or AB to given address - extended address"},
    {"STV",  o_m7,  0xFF, CPUALL ,0, "Store variable A or AB to given address - literal"},
    {""}};

// -------------------------------------------------------------------

void dataSpecAdd (uint16_t addr, uint16_t len, char *comment) {
	dataSpec_t *m;

	if (!dataSpec) {
		dataSpec = calloc(1,sizeof(dataSpec_t));
		m = dataSpec;
	} else {
		m = dataSpec;
		while (m->next) m = m->next;
		m->next = calloc(1,sizeof(dataSpec_t));
		m = m->next;
	}
	m->addr = addr;
	m->len = len;
	m->comment = comment;
}

dataSpec_t* findDataSpec(uint16_t addr) {
	dataSpec_t *m = dataSpec;

	while (m) {
		if (addr == m->addr) return m;
		m = m->next;
	}
	return NULL;
}

void labelAdd (uint16_t addr, char *name, char *comment) {
	label_t *l;

	if (!labels) {
		labels = calloc(1,sizeof(label_t));
		l = labels;
	} else {
		l = labels;
		while (l->next) {
			if (l->addr == addr) return;
			l = l->next;
		}
		if (l->addr == addr) return;
		l->next = calloc(1,sizeof(label_t));
		l = l->next;
	}
	l->addr = addr;
	if (name) l->name = strdup(name);
	else {
		char lab[10];
		sprintf(lab,"L%04x",addr);
		l->name = strdup(lab);
	}
	if (comment) l->comment = strdup(comment);
}

label_t* findLabel(uint16_t addr) {
	label_t *l = labels;

	while (l) {
		if (addr == l->addr) return l;
		l = l->next;
	}
	return NULL;
}


#define fmtBF  0
#define fmtBin 1

int inputFmt = fmtBF;
int org;
int codeAddrMax;
uint16_t currAddr;
FILE *inFile;
int atEof;
int errors;
unsigned char currByte;
int nextIsOrg;
char prefix[255],command[255],params[255],comment[255];
int currWordLength = 1;
int comments = 1;
int linesOut;
int oldFmt;
int assemblerOut;


void strupper (char * s) {
    while (*s) {
        *s = toupper(*s);
        s++;
    }
}

void usage (char *prgName) {
    printf( \
      "usage: %s [-b] [-o OrgAddr] [-1] [-2] [-3] [-4] FileName\n" \
      " -b\n" \
      " --binary        binary file as input, default is mai basic four console boot file\n" \
      " -o addr\n" \
      " --address=addr  start address (hex) for binary input\n" \
      " -n\n" \
      " --nocomments    omit comments in output\n" \
      " -1              set word length to 1 (for ADV,SBV,CPV,LDV.. with m=7)\n" \
      " -2, -3, -4      set word length to 2, 3 or 4\n" \
      " -l\n"\
      " --old           old opcode format\n" \
      " -a\n" \
      " --asmout        omit address and hexdump of command\n" \
      " -d --data       addr(hex),numBytes(hex)[,comment] - specify data\n" \
      " -L --label      addr(hex),label[,comment] - add label\n" \
      " -M --cpumd      Microdata 800/1600 CPU\n" \
      " -B --cpu1200    Basic Four 1200 CPU (default)\n" \
      " -C --cpu1300    Basic Four 1300 CPU\n" \
      " -D --cpu1320    Basic Four 1320 CPU\n" \
      "\n" \
      "Basic four console boot file is an ascii file with upper case hex chars. Lines are\n" \
      "terminated with 0x1f. First four bytes of a line is the 16 bit target address. A line\n" \
      "with a target address only starts execution at target address.\n" \
      "Disassembly for variable opcodes (e.g. LDV) will fail if the code used diffetent\n" \
      "word lengths since the opcode is the same and the word length can be set in code.\n" \
      "Labels will be read from file if a file named inputFileName.labels is present.\n"

      ,prgName);
    _exit (1);
}

const char * cpuTxt() {
	switch (cpu) {
		case CPU1600: return "Microdata 800/1600";
		case CPU1200: return "Basic Four 1200";
		case CPU1300: return "Basic Four 1300";
		case CPU1320: return "Basic Four 1320";
		default: return "Unknown";
	}
}

const char * cpuTxtA() {
	switch (cpu) {
		case CPU1600: return "MD1600";
		case CPU1200: return "BF1200";
		case CPU1300: return "BF1300";
		case CPU1320: return "BF1320";
		default: return "Unknown";
	}
}

int hex2int (char * c) {
    int res;

    res = (int)strtol(c, NULL, 16);
    return res;
}

unsigned char nextChar() {
    unsigned char res = 0;
    int bytesRead;
    if (!atEof) {
        bytesRead = fread(&res,1,1,inFile);
        if (bytesRead != 1) { atEof = 1; res = 0; }
    }
    if ((res == 0x1f) || (res == 0x0a)) nextIsOrg = 1;
    return res;
}

unsigned char nextByte() {
    char hexStr[3];
    char res;
    if (inputFmt == fmtBin) {
        return nextChar();
    } else {
        if (nextIsOrg) return 0;
        hexStr[0] = nextChar();
        if (nextIsOrg) return 0;
        hexStr[1] = nextChar(); hexStr[2] = 0;
        if (nextIsOrg) return 0;
        res = hex2int(hexStr);
       // printf(" .. %02x ",res);
        return res;
    }
}


int8_t geti8() {
    char s[10]; int i;

    currAddr++;
    i = (int8_t)nextByte();
    sprintf(s,"%02x",(uint8_t)i); strcat(prefix,s);
    return i;
}

uint8_t get8() {
    int i;
    char s[10];

    currAddr++;
    i = (uint8_t)nextByte();
    sprintf(s,"%02x",i); strcat(prefix,s);
    return i;
}

uint16_t get16() {
    int i;
    i = get8() << 8;
    i = i + get8();
    return i;
}

void invalidOpcode(char * addrInfo) {
	char command[3];
	char comment[80];
	char params[20];

	strcpy(command,"DB");
	strcpy(comment,"; invalid opcode for selected cpu, following disassembly may be incorrect");
	sprintf(params,"%02xH",currByte);
	if (assemblerOut)
		printf("         %-6s %-20s %s%s\n",command,params,comment,addrInfo);
	else
		printf("%-17s %-6s %-20s %s%s\n",prefix,command,params,comment,addrInfo);
}


void processData(dataSpec_t* d) {
	int line = 0;
	int byteCount = 0;
	int bytesRemaing = d->len;
	char prefix[255];
	char sufix[255];
	char tmp[3];

	prefix[0] = 0;

	while (bytesRemaing) {

		if(byteCount) {
			sprintf(tmp,"%02x",currByte);
			strcat(prefix,tmp);
			strcat(sufix,",");
			strcat(sufix,tmp);
			strcat(sufix,"H");
		} else {
			strupper(prefix);
			sprintf(prefix,"%04x %02x",currAddr,currByte);
			strupper(sufix);
			sprintf(sufix,"DB %02xH",currByte);
		}


		byteCount++;
		if (byteCount > 5) {
			byteCount=0;
			line++;
			if (assemblerOut)
				printf("         %s\n",sufix);
			else
				printf("%-18s%s\n",prefix,sufix);
			linesOut++;
		}
		bytesRemaing--;
		if (bytesRemaing) {
			currByte = geti8();
		}
		currAddr++;
	}
	if (byteCount) {
		strupper(prefix); strupper(sufix);
		if (assemblerOut)
			printf("         %s\n",sufix);
		else
			printf("%-18s%s\n",prefix,sufix);
	}
}

#define P1 if(pass)
#define P0 if(!pass)

void processOpcode(int pass) {
    int typ;
    int wordLength;
    int param,param2,param1,absAddr;
    char cmd;
    int8_t relOfs;
    char stmp[20];
    char addrInfo[40];
    int nextOpcode;
    struct OpcdRec currOpc;
    int i;
    char *labelName = NULL;
    char *labelComment = NULL;
    label_t *label = findLabel(currAddr);	// may be we have a label (for assembler out)

    if (label && assemblerOut) {
		labelName = label->name;
		labelComment = label->comment;
    }
    label = NULL;

    addrInfo[0] = 0;
    params[0] = 0;
    comment[0] = 0;

    dataSpec_t *dataSpec = findDataSpec(currAddr);
    if (dataSpec) {
		processData(dataSpec);
		return;
    }

    sprintf(prefix,"%04x %02x",currAddr,currByte);
    currAddr++;

    if (!MD1600_opcdTab[(short)currByte]) {		// undefined opcode for selected cpu
		invalidOpcode(addrInfo);
		linesOut++;
		return;
    }
	currOpc = *MD1600_opcdTab[(short)currByte];
    typ = currOpc.typ;

    cmd = currByte;

    switch (typ) {
		case o_3F:
			i=0;
			nextOpcode = geti8();
			while (opcodeTable3F[i].name[0] != '\0') {
				if (opcodeTable3F[i].cpu & cpu && opcodeTable3F[i].opcodeval == nextOpcode) {
					currOpc = opcodeTable3F[(short)i];
					break;
				}
				i++;
			}
			invalidOpcode(addrInfo);
			linesOut++;
			return;
		case o_47:
			i=0;
			nextOpcode = geti8();
			while (opcodeTable47[i].name[0] != '\0') {
				if (opcodeTable47[i].cpu & cpu && opcodeTable3F[i].opcodeval == nextOpcode) {
					currOpc = opcodeTable3F[(short)i];
					break;
				}
				i++;
			}
			invalidOpcode(addrInfo);
			linesOut++;
			return;
    }

    strcpy(command,currOpc.name);
    typ = currOpc.typ;

    if (comments) {
      param = currByte;
      if (param >= 0x60) param &= 0xf8;
      if (currOpc.cmnt) {
        sprintf(comment,"; %s",currOpc.cmnt);
        if (currByte >= 0x60) {
            sprintf(stmp," (m=%d)",currByte & 0x07);
            strcat(comment,stmp);
        }
      }
    }

    switch (typ) {
		case o_None:         // No operands
          break;
        case o_BraRel:       // short branch
			relOfs = geti8();
			P0 labelAdd(currAddr+relOfs,NULL,NULL);
			P1 {
				if (assemblerOut) label = findLabel(currAddr+relOfs);
				if (assemblerOut && label)
					sprintf(params,label->name);
				else {
					sprintf(params,"%04xH",currAddr+relOfs);
					strupper(params);
				}
				sprintf(addrInfo," (ofs: %d=0x%02x, abs: 0x%04x)",relOfs,relOfs,currAddr+relOfs);
			}
			break;
        case o_Bra16:        // long branch
			param = get16();
			sprintf(params,"%04xH",param);
			strupper(params);
			break;
        case o_Imm8:         // one byte as parameter
			param = get8();
			if (param < 0x0a)
				sprintf(params,"%d",param);
			else
				sprintf(params,"%02xH",param);
			strupper(params);
			break;
        case o_Imm16:        // two bytes as parameter
			param = get16();
			sprintf(params,"%04xH",param);
			strupper(params);
			break;
        case o_m0:           // m=0: 8 bit addr in page 0
			param = get8();
			if (oldFmt)
				sprintf(params,"%02xH",param);
			else
				sprintf(params,"[%02xH]",param);
			strupper(params);
			break;
        case o_jm0:           // m=0: 8 bit addr in page 0
			param = get8();
			if (assemblerOut) label = findLabel(param);
			if (assemblerOut && label) {
				sprintf(params,"%s",label->name);
			} else {
				if (oldFmt)
					sprintf(params,"X'%02x'",param);
				else
					sprintf(params,"SHORT %02xH",param);
				strupper(params);
			}
			break;
        case o_m1:           // m=1: relative 8 bit
			relOfs = geti8();
			absAddr = currAddr+relOfs;
			P0 labelAdd(absAddr,NULL,NULL);
			P1 {
				if (assemblerOut) label = findLabel(absAddr);
				if (assemblerOut && label) {
					sprintf(params,"SHORT [%s]",label->name);
				} else {
					if (oldFmt) {
						sprintf(params,"X'%04x'",absAddr);
					} else {
						sprintf(params,"SHORT [%04xh]",absAddr);
					}
					strupper(params);
				}
				sprintf(addrInfo," (ofs: %d=0x%02x, abs: 0x%04x)",relOfs,relOfs,absAddr);
			}
			break;
        case o_jm1:           // m=1: relative 8 bit
			relOfs = geti8();
			absAddr = currAddr+relOfs;
			P0 labelAdd(absAddr,NULL,NULL);
			P1 {
				if (assemblerOut) label = findLabel(absAddr);
				if (assemblerOut && label) {
					sprintf(params,"SHORT %s",label->name);
				} else {
					if (oldFmt) {
						sprintf(params,"X'%04x'",absAddr);
					} else
						sprintf(params,"SHORT %04xH",absAddr);
					strupper(params);
				}
				sprintf(addrInfo," (ofs: %d=0x%02x, abs: 0x%04x)",relOfs,relOfs,absAddr);
			}
			break;
        case o_m2:           // m=2: indirect page 0
			param = get8();
			P0 labelAdd(param,NULL,NULL);
			P1 {
				if (assemblerOut) label = findLabel(param);
				if (assemblerOut && label) {
					if (oldFmt) {
						strcat(command,"'");
						sprintf(params,"%s",label->name);
					} else
						sprintf(params,"[*%s]",label->name);
				} else {
					if (oldFmt) {
						strcat(command,"'");
						sprintf(params,"%02x",param);
					} else
						sprintf(params,"[*%02xH]",param);
					strupper(params);
				}
			}
			break;
        case o_jm2:           // m=2: indirect page 0
			param = get8();
			P0 labelAdd(param,NULL,NULL);
			P1 {
				if (assemblerOut) label = findLabel(param);
				if (assemblerOut && label)
					sprintf(params,"[%s]",label->name);
				else {
					sprintf(params,"[%02xH]",param);
					strupper(params);
				}
			}
			break;
        case o_m3:           // m=3: indirect relative
			relOfs = geti8();
			absAddr = currAddr+relOfs;
			P0 labelAdd(absAddr,NULL,NULL);
			P1 {
				if (assemblerOut) label = findLabel(absAddr);
				if (assemblerOut && label) {
					if (oldFmt) {
						strcat(command,"*");
						sprintf(params,"%s",label->name);
					} else
						sprintf(params,"[*%s]",label->name);
				} else {
					if (oldFmt) {
						strcat(command,"*");
						sprintf(params,"X'%04x'",absAddr);
					} else
						sprintf(params,"[*%04xH]",absAddr);
					strupper(params);
				}
				sprintf(addrInfo," (ofs: %d=0x%02x, abs: 0x%04x)",relOfs,relOfs,absAddr);
			}
			break;
        case o_jm3:           // m=3: indirect relative
			relOfs = geti8();
			absAddr = currAddr+relOfs;
			P0 labelAdd(absAddr,NULL,NULL);
			P1 {
				if (assemblerOut) label = findLabel(absAddr);
				if (assemblerOut && label) {
					if (oldFmt) {
						strcat(command,"*");
						sprintf(params,"%s",label->name);
					} else
						sprintf(params,"[%s]",label->name);
				} else {
					if (oldFmt) {
						strcat(command,"*");
						sprintf(params,"X'%04x'",absAddr);
					} else
						sprintf(params,"[%04xH]",absAddr);
					strupper(params);
				}
				sprintf(addrInfo," (ofs: %d=0x%02x, abs: 0x%04x)",relOfs,relOfs,absAddr);
			}
			break;
        case o_m4:           // m=4: indexed
			if (oldFmt)
				strcat(command,"-");
			else
				strcpy(params,"[X]");
			break;
        case o_jm4:           // m=4: indexed
			if (oldFmt)
				strcat(command,"-");
			else
				strcpy(params,"X");
			break;
        case o_m5:           // m=5: indexed with bias
			param = get8();
			if (oldFmt) {
				strcat(command,"+");
				sprintf(params,"X'%02x'",param);
			} else
				sprintf(params,"[X+%02xH]",param);
            strupper(params);
			break;
        case o_jm5:           // m=5: indexed with bias
			param = get8();
			if (oldFmt) {
				strcat(command,"+");
				sprintf(params,"X'%02x'",param);
			} else
				sprintf(params,"X+%02xH",param);
			strupper(params);
			break;
        case o_m6:           // m=6: extended address
			param = get16();
			if (oldFmt) {
				strcat(command,"/");
				sprintf(params,"X'%04x'",param);
			} else {
				sprintf(params,"[%04xH]",param);
			}
			strupper(params);
			break;
		case o_jm6:           // m=6: extended address
			param = get16();
			if (oldFmt) {
				strcat(command,"/");
				sprintf(params,"X'%04x'",param);
			} else {
			if (param & 0x8000)
				sprintf(params,"X+%04xH",param & 0x7fff);
			else
				sprintf(params,"%04xH",param);
			}
			strupper(params);
			break;
        case o_m7:           // m=7: indirect extended address
        case o_jm7:
			if (oldFmt)
				strcat(command,"=");
			param = cmd & 0xf8;  // mask out m0..7
			if ((param == OP_JMP) | (param == OP_RETJMP)) {
				param = get16();
				if (oldFmt) {
					strcat(command,"/");
					sprintf(params,"X'%04x'",param);
				} else
				if (param & 0x8000) {  // bit7 = X+
					sprintf(params,"[X+%04xH]",param & 0x7fff);
				} else {
					sprintf(params,"[%04xH]",param);
				}
			} else {
				wordLength = 2;  // default to 2 byte for opcodes without "varlen" flag
				if (currOpc.varlen)
					// this may be 1,2,3 or 4 byte and depends on the word lenth set
					wordLength = currWordLength;
				switch (wordLength) {
					case 1:
						param = get8();
						if (oldFmt) sprintf(params,"X'%02x'",param);
						else sprintf(params,"%02xH",param);
						break;
					case 2:
						param = get16();
						if (oldFmt) sprintf(params,"X'%04x'",param);
						else sprintf(params,"%04xH",param);
						break;
					case 3:
						param = get16(); param2 = get8();
						if (oldFmt) sprintf(params,"X'%04x%02x'",param,param2);
						else sprintf(params,"%04x%02xH",param,param2);
						break;
					case 4:
						param = get16(); param2 = get16();
						if (oldFmt) sprintf(params,"X'%08x'",param);
						else sprintf(params,"%04x%04xH",param,param2);
						break;
				}
			}
			strupper(params);
			break;
        case o_IBM_OBM:
			param = get8();
			param2 = get16();
			if (param & 0x8000) {  // bit7 = X+
				sprintf(params,"%0x02,[X+%04xH]",param,param2 && 0x7fff);
			} else {
				sprintf(params,"%0x02,[%04xH]",param,param2);
			}
			strupper(params);
			break;
        case o_inout:
			param = get8();
			if (cpu & CPU13xx) { // AD 16.10.2025: looks like basic four 13xx is using 4 bit function code and 4 bit device address
				param2 = (param & 0xf0) >> 4;
				param1 = param & 0x0f;
			} else {
				param2 = (param & 0xe0) >> 5;
				param1 = param & 0x1f;
			}
			if (oldFmt) {
				sprintf(params,"X'%02x',X'%02x'",param2,param1);
			} else {
				sprintf(params,"%02xH,%02xH",param2,param1);
			}
			sprintf(addrInfo," (0x%02x)",param);
			strupper(params);
			break;

    }
    P1 {
		strupper(prefix); //strupper(params);
		if (assemblerOut) {
			int statementPrinted = 0;
			if (labelName) {
				char* newLabelName = malloc(strlen(labelName)+2);
				strcpy(newLabelName,labelName);
				strcat(newLabelName,":");
				if(strlen(labelName) > 7 || labelComment) {
					if (labelComment)
						printf("%s                               ; %s\n",newLabelName,labelComment);
					else
						printf("%s\n",newLabelName);
				} else {
					printf("%-9s%-6s %-20s %s%s\n",newLabelName,command,params,comment,addrInfo);
					statementPrinted++;
				}
				free(newLabelName);
			}
			if (!statementPrinted) printf("         %-6s %-20s %s%s\n",command,params,comment,addrInfo);
		} else {
			printf("%-17s %-6s %-20s %s%s\n",prefix,command,params,comment,addrInfo);
		}
		linesOut++;
    }
    if (currAddr > codeAddrMax) codeAddrMax = currAddr;
}


char skipNoise(char c) {
	while (c < '0') {
		if (atEof) return 0;
		c = nextChar();
	}
	return c;
}

// skip empty and/or comment lines in bf/microdata boot format
// comments not allowed on original format. I assume that the loader
// ignored empty lines
char skipEmptyLines(void) {
	char c;
	c = nextChar();
	while ((c < '0') || (c == ';')){
	  c = skipNoise(c);
	  if (atEof) return 0;
	  if (c == ';') { // comment line
		  c = nextChar();
		  while ((c != 0x0a) && (c != 0x1f)) {
			  c = nextChar();
			  if (atEof) return 0;
		 }
		 c = nextChar();
	  }
	}
	return c;
}


void processFile(char *fileName, int pass) {
    char orgAddr[9];
    int firstAddr = 1;

    inFile = fopen(fileName,"rb");
    if (!inFile) {
        printf("unable to open \"%s\"\n",fileName);
        errors++;
        return;
    }
    atEof = 0;
    nextIsOrg = 0;
    if (inputFmt == fmtBF) nextIsOrg = 1;
    currAddr = org;

    while (!atEof) {
        if (nextIsOrg) {
            nextIsOrg = 0;
            // 4 bytes for start address
            strcpy(orgAddr,"0x0000");
            orgAddr[2] = skipEmptyLines();
            if (!atEof){
              orgAddr[3] = nextChar();
              orgAddr[4] = nextChar();
              orgAddr[5] = nextChar();
              nextIsOrg = 0; currByte = nextByte();
              if (nextIsOrg) atEof=1;
            }
            if (!atEof) {
              org = hex2int(orgAddr);
              currAddr = org;
              //printf("org %04x\n",org);
              if (firstAddr) {
				  if (assemblerOut) {
					  P1 printf(" .CPU %s\n",cpuTxtA());
					  P1 printf(" ORG 0x%04x\n",currAddr);
				  }
			  }
              firstAddr = 0;
            }
       } else
         currByte = nextByte();
       //if (!((currByte == 0x1F) & (inputFmt == fmtBF)))
       if (!nextIsOrg)
           processOpcode(pass);
    }
    fclose(inFile);
}


void parseData(char *dParam) {
	char * src = strdup(dParam);
	char *token;
	int numParam = 0;
	uint16_t addr = 0;
	uint16_t len = 0;
	char *comment = NULL;

	token = strtok(src,",");
	while (token) {
		switch (numParam) {
		case 0:
			addr = strtol(token,NULL,16);	// FIXME: detect invalid hex chars
			if(errno) {
				fprintf(stderr,"invalid hex address in '%s'\n",dParam);
				exit(1);
			}
			break;

		case 1:
			len = strtol(token,NULL,16);	// FIXME: detect invalid hex chars
			if(errno) {
				fprintf(stderr,"invalid hex address in '%s'\n",dParam);
				exit(1);
			}
			break;

		case 2:
			comment = strdup(token);
			break;
		}
		numParam++;
		token = strtok(NULL,",");
	}
	dataSpecAdd (addr, len, comment);
	free(src);
}

void parseLabel(char *dParam) {
	char * src = strdup(dParam);
	char *token;
	int numParam = 0;
	uint16_t addr = 0;
	char *name = NULL;
	char *comment = NULL;

	token = strtok(src,",");
	while (token) {
		switch (numParam) {
		case 0:
			addr = strtol(token,NULL,16);	// FIXME: detect invalid hex chars
			if(errno) {
				fprintf(stderr,"invalid hex address in '%s'\n",dParam);
				exit(1);
			}
			break;
		case 1:
			name = strdup(token);
			break;
		case 2:
			comment = strdup(token);
			break;
		}
		numParam++;
		token = strtok(NULL,",");
	}
	labelAdd(addr, name, comment);
	free(src);
}


#define MAX_LF_LINE_LEN 255
void readLablesFromFile(char *fileName) {
	char line[MAX_LF_LINE_LEN];
	FILE *fp = fopen(fileName,"r");

	if (!fp) return;

    while (fgets(line, MAX_LF_LINE_LEN, fp)) {
        // Remove trailing CR / LF
        line[strcspn(line, "\n")] = 0;
        line[strcspn(line, "\r")] = 0;	// for Windows files
        if(strlen(line) >1 ) {
        	if(line[0] != '#' && line[0] != ';')
				parseLabel(line);
        }
    }
    fclose(fp);
}


int main (int argc, char **argv) {

  int c;
  errors = 0;
  opterr = 0;
  char tmpStr[255];

  static struct option long_options[] =
  {
    {"help",      no_argument,       0, 'h'},
    {"old",       no_argument,       0, 'l'},
    {"binary",    no_argument,       0, 'b'},
    {"address",   required_argument, 0, 'o'},
    {"nocomments",no_argument,       0, 'n'},
    {"asmout",    no_argument,       0, 'a'},
    {"cpumd",     no_argument,       0, 'M'},
    {"cpu1200",   no_argument,       0, 'B'},
    {"cpu1300",   no_argument,       0, 'C'},
    {"cpu1320",   no_argument,       0, 'D'},
    {"data"   ,   required_argument, 0, 'd'},
    {"label"  ,   required_argument, 0, 'L'},
    {0, 0, 0, 0}
  };

  int option_index = 0;


  while ((c = getopt_long (argc, argv, "1234nbo:hl?aMBCDd:L:",long_options, &option_index)) != -1) {
    switch (c)
      {
	  case 1:
	    break;
      case '1':
        currWordLength = 1;
        break;
      case '2':
        currWordLength = 2;
        break;
      case '3':
        currWordLength = 3;
        break;
      case '4':
        currWordLength = 4;
        break;
      case 'b':
        inputFmt = fmtBin;
        break;
      case 'o':
        org = hex2int(optarg);
        break;
      case 'n':
        comments = 0;
        break;
      case 'l':
        oldFmt = 1;
        break;
      case 'a':
        assemblerOut = 1;
        break;
	  case 'M':
		cpu = CPU1600;
		break;
	  case 'B':
		cpu = CPU1200;
		break;
	  case 'C':
		cpu = CPU1300;
		break;
	  case 'D':
		cpu = CPU1320;
		break;
	  case 'd':
	  	parseData(optarg);
		break;
	  case 'L':
	  	parseLabel(optarg);
		break;
      default:
        usage(argv[0]);
      }
  }

  if (optind >= argc) {
      printf("no input files specified\n");
      _exit(1);
  }
  printf("\n");

  // build instruction table for the selected cpu
  int i = 0;
  while (opcodeTable[i].name[0] != '\0') {
		if (opcodeTable[i].cpu & cpu) MD1600_opcdTab[opcodeTable[i].opcodeval] = &opcodeTable[i];
		i++;
  }

  if (assemblerOut) putchar(';');
  printf(MYSELF,VERMAJ,VERMIN,cpuTxt());

  dataSpec_t* m = dataSpec;
  while (m) {
		printf("; data @ %04xH, length %04xH\n",m->addr,m->len);
		m = m->next;
  }

// read labels file if present
  char *labelsFileName = malloc(strlen(argv[optind])+8);
  strcpy(labelsFileName,argv[optind]);
  strcat(labelsFileName,".labels");
  readLablesFromFile(labelsFileName);
  free(labelsFileName);


  processFile (argv[optind],0);		// pass 0
  label_t* l;
#if 0
  l = labels;
  while(l) {
		printf("%04x %s '%s'\n",l->addr,l->name,l->comment);
		l = l->next;
  }
  printf("\n");
#endif
  // mark labels not in code
  l = labels;
  while(l) {
	if (l->addr < org || l->addr > codeAddrMax) {
		l->LabelInCode = 0;
		if (assemblerOut) {
			printf("%s:\tEQU\t",l->name);
			sprintf(tmpStr,"%04xH\n",l->addr);
			printf(tmpStr);
		}
	} else
		l->LabelInCode = 1;
	l = l->next;
  }

  processFile (argv[optind],1);
  if (linesOut > 0)
    if (comments)
      if (oldFmt)
        printf("\n; Postfixes: * indirect(m2,3), - indexed(m4), + indexed bias(m5)\n; / extended address(m6), = indirect extended address(m7)\n");
  return (errors = 0);
}

