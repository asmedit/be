#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

enum OPTIONS {
     NO_OPTIONS = 0x00,
     USE_GP_NAMES = 0x01,
     USE_LONG_FORM = 0x02,
     USE_ARMIPS_CP0_NAMES = 0x04
};

enum GP {
     r0, // Constant 0
     at, // Used for psuedo-instructions 1
     v0, v1, // Function returns 2 3
     a0, a1, a2, a3, // Function arguments 4 5 6 7
     t0, t1, t2, t3, t4, t5, t6, t7, // Temporary 8 9 10 11 12 13 14 15
     s0, s1, s2, s3, s4, s5, s6, s7, // Saved 16 17 18 19 20 21 22 23
     t8, t9, // More temporary 24 25
     k0, k1, // Reserved for kernal 26 27
     gp, // Global area pointer 28
     sp, // Stack pointer 29
     s8, // One more saved pointer 30
     ra // Return address 31
};

static char * gpr[] = 
{    "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
     "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra" };

static char * cp0[] = 
{    "sp_mem_addr", "sp_dram_addr", "sp_rd_len", "sp_wr_len",
     "sp_status", "sp_dma_full", "sp_dma_busy", "sp_semaphore",
     "dpc_start", "dpc_end", "dpc_current", "dpc_status",
     "dpc_clock", "dpc_bufbusy", "dpc_pipebusy", "dpc_tmem" };

enum CP0 {
     DMA_CACHE,
     DMA_DRAM,
     DMA_READ_LENGTH,
     DMA_WRITE_LENGTH,
     SP_STATUS,
     DMA_FULL,
     DMA_BUSY,
     SP_RESERVED,
     CMD_START,
     CMD_END,
     CMD_CURRENT,
     CMD_STATUS,
     CMD_CLOCK,
     CMD_BUSY,
     CMD_PIPE_BUSY,
     CMD_TMEM_BUSY
};

enum RSP_OPCODE {
     SPECIAL = 0x00, // 000000
     REGIMM = 0x01,  // 000001
     J = 0x02,       // 000010
     JAL = 0x03,     // 000011
     BEQ = 0x04,     // 000100
     BNE = 0x05,     // 000101
     BLEZ = 0x06,    // 000110
     BGTZ = 0x07,    // 000111
     ADDI = 0x08,    // 001000
     ADDIU = 0x09,   // 001001
     SLTI = 0x0A,    // 001010
     SLTIU = 0x0B,   // 001011
     ANDI = 0x0C,    // 001100
     ORI = 0x0D,     // 001101
     XORI = 0x0E,    // 001110
     LUI = 0x0F,     // 001111
     COP0 = 0x10,    // 010000
     COP2 = 0x12,    // 010010
     LB = 0x20,      // 100000
     LH = 0x21,      // 100001
     LW = 0x23,      // 100011
     LBU = 0x24,     // 100100
     LHU = 0x25,     // 100101
     LWU = 0x27,     // 100111
     SB = 0x28,      // 101000
     SH = 0x29,      // 101001
     SW = 0x2B,      // 101011
     LWC2 = 0x32,    // 110010
     SWC2 = 0x3A,    // 111010
};

enum RSP_VECTOR_OPCODE {
     VMULF = 0x00, // Vector (Frac) Multiply
     VMULU = 0x01, // Vector (Unsigned Frac) Multiply
     VRNDP = 0x02, // Vector DCT Round (+)
     VMULQ = 0x03, // Vector (Integer) Multiply
     VMUDL = 0x04, // Vector low multiply
     VMUDM = 0x05, // Vector mid-m multiply
     VMUDN = 0x06, // Vector mid-n multiply
     VMUDH = 0x07, // Vector high multiply
     VMACF = 0x08, // Vector (Frac) Multiply Accumulate
     VMACU = 0x09, // Vector (Unsigned Frac) Multiply Accumulate
     VRNDN = 0x0A, // Vector DCT Round (-)
     VMACQ = 0x0B, // Vector (Integer) Multiply Accumulate
     VMADL = 0x0C, // Vector low multiply accumulate
     VMADM = 0x0D, // Vector mid-m multiply accumulate
     VMADN = 0x0E, // Vector mid-n multiply accumulate
     VMADH = 0x0F, // Vector high multiply accumulate
     VADD = 0x10,  // Vector add
     VSUB = 0x11,  // Vector subtract
     VABS = 0x13,  // Vector absolute value
     VADDC = 0x14, // Vector add with carry
     VSUBC = 0x15, // Vector subtract with carry
     VSAR = 0x1D,  // Vector accumulator read (and write)
     VLT = 0x20,   // Vector select (Less than)
     VEQ = 0x21,   // Vector select (Equal)
     VNE = 0x22,   // Vector select (Not equal)
     VGE = 0x23,   // Vector select (Greater than or equal)
     VCL = 0x24,   // Vector select clip (Test low)
     VCH = 0x25,   // Vector select clip (Test high)
     VCR = 0x26,   // Vector select crimp (Test low)
     VMRG = 0x27,  // Vector select merge
     VAND = 0x28,  // Vector AND
     VNAND = 0x29, // Vector NAND
     VOR = 0x2A,   // Vector OR
     VNOR = 0x2B,  // Vector NOR
     VXOR = 0x2C,  // Vector XOR
     VNXOR = 0x2D, // Vector NXOR
     VRCP = 0x30,  // Vector element scalar reciprocal (Single precision)
     VRCPL = 0x31, // Vector element scalar reciprocal (Double precision, Low)
     VRCPH = 0x32, // Vector element scalar reciprocal (Double precision, High)
     VMOV = 0x33,  // Vector element scalar move
     VRSQ = 0x34,  // Vector element scalar SQRT reciprocal
     VRSQL = 0x35, // Vector element scalar SQRT reciprocal (Double precision, Low)
     VRSQH = 0x36, // Vector element scalar SQRT reciprocal (Double precision, High)
     VNOP = 0x37,  // Vector null instruction
};

enum RSP_LOAD_STORE_COMMAND {
     b = 0x00, // (BYTE)        00000
     s = 0x01, // (HALFWORD)    00001
     l = 0x02, // (WORD)        00010
     d = 0x03, // (DOUBLEWORD)  00011
     q = 0x04, // (QUADWORD)    00100
     r = 0x05, // (REST)        00101
     p = 0x06, // (PACKED)      00110
     u = 0x07, // (UNPACKED)    00111
     h = 0x08, // (HALF)        01000
     f = 0x09, // (FOURTH)      01001
     w = 0x0A, // (WRAP)        01010
     t = 0x0B, // (TRANSPOSE)   01011
};

static bool usingLongForm = true;
static bool usingArmipsCP0Names = true;

static char str_opcode[100];
static char str_bo[100];
static char str_beqo[100];
static char str_lost[100];

static char * rsp[] = {
    "special", "regimm", "j", "jal", "beq", "bne", "blez", "bgtz", "addi", "addiu", "slti", "sltiu",
    "andi", "ori", "xori", "lui", "cop0", "cop2", "lb", "lh", "lw", "lbu", "lwu", "sb", "sh", "sw", "lwc2", "swc2" };

static char * rsp_vec[] =   {
    "vmulf", "vmulu", "vrndp", "vmulq", "vmudl", "vmudm", "vmudn", "vmudh", "vmacf", "vmacu", "vrndn",
    "vmacq", "vmadl", "vmadm", "vmadn", "vmadh", "vadd", "vsub", "vabs", "vaddc", "vsubc", "vsar", "vlt",
    "veq", "vne", "vge", "vch", "vcr", "vmrg", "vand", "vnand", "vor", "vnor", "vxor", "vnxor", "vcrp",
    "vrcpl", "vrcph", "vmov", "vrsq", "vrsql", "vrsqh", "vnop" };

static char * rsp_lost[] = {
    "b", "s", "l", "d", "q", "r", "p", "u", "h", "f", "w", "t" };

char * getRPRegName(enum GP reg) { return gpr[reg]; }
char * getCP0RegName(enum CP0 reg) { return cp0[reg]; }

char * decodeLoadStore(uint16_t operation, char *LorS)
{
    enum RSP_LOAD_STORE_COMMAND ls_subop = (enum RSP_LOAD_STORE_COMMAND)((operation >> 11) & 0x1F);
    sprintf(str_lost,"%s%sv", LorS, rsp_lost[ls_subop]);
    uint8_t base_ = (uint8_t)((operation >> 21) & 0x1F);
    uint8_t dest = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t del = (uint8_t)((operation >> 7) & 0xF);
    uint16_t offset = (uint16_t)((operation & 0x3F) << 2);
    sprintf(str_lost,"%s%sv $v%i[%d], 0x%4x(%s)", LorS, rsp_lost[ls_subop], dest, del, offset, getRPRegName(base_));
    return str_lost;
}

char * getLongFormForQuaterElement(int eq)
{
    switch (eq) {
        case 0: return "[00224466]";
        case 1: return "[11335577]";
        default: return "";
    }
}

char * getLongFormForHalfElement(int eh)
{
    switch (eh) {
        case 0: return "[00004444]";
        case 1: return "[11115555]";
        case 2: return "[22226666]";
        case 3: return "[33337777]";
        default: return "";
    }
}

char * getLongForm(int e)
{
    switch (e) {
        case 0: return "[00000000]";
        case 1: return "[11111111]";
        case 2: return "[22222222]";
        case 3: return "[33333333]";
        case 4: return "[44444444]";
        case 5: return "[55555555]";
        case 6: return "[66666666]";
        case 7: return "[77777777]";
        return "";
    }
}


char * decodeBranch(char *opcode, uint16_t operation, unsigned long int address)
{
     enum GP src = (enum GP)((operation >> 21) & 0x1F);
     uint16_t imm = (uint16_t)((operation & 0xFFFF) << 2);
     uint32_t current_offset = (uint32_t)((address + 4) + imm);
     sprintf(str_bo, "%s %s, 0x%8x", opcode, getRPRegName(src), current_offset);
     return str_bo;
}

char * decodeBranchEquals(char * opcode, uint16_t operation, unsigned long int address)
{
     enum GP src1 = (enum GP)((operation >> 21) & 0x1F);
     enum GP src2 = (enum GP)((operation >> 16) & 0x1F);
     uint16_t imm = (uint16_t)((operation & 0xFFFF) << 2);
     uint32_t current_offset = (uint32_t)((address + 4) + imm);
     sprintf(str_beqo, "%s %s, %s, 0x%8x", opcode, getRPRegName(src1), getRPRegName(src2), current_offset);
     return str_beqo;
}

char * decodeMIPS(uint32_t operation, unsigned long int address, char *outbuf)
{
    if (operation == 0x00000000) {
        sprintf(outbuf, "%s", "nop");
        return outbuf;
    }
    enum RSP_OPCODE opcode = (enum RSP_OPCODE)((operation >> 26) & 0x3F);
    switch (opcode)
    {
        case J:
        case JAL:
             sprintf(outbuf, "%s 0x0%7x", rsp[opcode], ((operation & 0x03FFFFFF) << 2));
             return outbuf;
        case BEQ:
        case BNE:
             sprintf(outbuf, "%s", decodeBranchEquals(rsp[opcode], operation, address));
             return outbuf;
        case BLEZ:
        case BGTZ:
             sprintf(outbuf, "%s", decodeBranch(rsp[opcode], operation, address));
             return outbuf;
    }
    sprintf(outbuf,"Unknown (opcode: 0x%x)", operation);
    return outbuf;
}

