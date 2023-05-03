#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

enum GP {
     r0, at, v0, v1, a0, a1, a2, a3,
     t0, t1, t2, t3, t4, t5, t6, t7,
     s0, s1, s2, s3, s4, s5, s6, s7,
     t8, t9, k0, k1, gp, sp, s8, ra    };

enum CP0 {
     DMA_CACHE,     DMA_DRAM,     DMA_READ_LENGTH,     DMA_WRITE_LENGTH,
     SP_STATUS,     DMA_FULL,     DMA_BUSY,            SP_RESERVED,
     CMD_START,     CMD_END,      CMD_CURRENT,         CMD_STATUS,
     CMD_CLOCK,     CMD_BUSY,     CMD_PIPE_BUSY,       CMD_TMEM_BUSY     };

static char * gpr[] =  {
     "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
     "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra" };

static char * cp0[] =  {
     "sp_mem_addr", "sp_dram_addr", "sp_rd_len",    "sp_wr_len",
     "sp_status",   "sp_dma_full",  "sp_dma_busy",  "sp_semaphore",
     "dpc_start",   "dpc_end",      "dpc_current",  "dpc_status",
     "dpc_clock",   "dpc_bufbusy",  "dpc_pipebusy", "dpc_tmem" };

enum RSP_OPCODE {
     SPECIAL = 0x00,  REGIMM = 0x01,   J = 0x02,        JAL = 0x03,
     BEQ = 0x04,      BNE = 0x05,      BLEZ = 0x06,     BGTZ = 0x07,
     ADDI = 0x08,     ADDIU = 0x09,    SLTI = 0x0A,     SLTIU = 0x0B,
     ANDI = 0x0C,     ORI = 0x0D,      XORI = 0x0E,     LUI = 0x0F,
     COP0 = 0x10,     COP2 = 0x12,     LB = 0x20,       LH = 0x21,
     LW = 0x23,       LBU = 0x24,      LHU = 0x25,      LWU = 0x27,
     SB = 0x28,       SH = 0x29,       SW = 0x2B,       LWC2 = 0x32,    SWC2 = 0x3A, };

enum RSP_VECTOR_OPCODE {
     VMULF = 0x00,    VMULU = 0x01,    VRNDP = 0x02,    VMULQ = 0x03,
     VMUDL = 0x04,    VMUDM = 0x05,    VMUDN = 0x06,    VMUDH = 0x07,
     VMACF = 0x08,    VMACU = 0x09,    VRNDN = 0x0A,    VMACQ = 0x0B,
     VMADL = 0x0C,    VMADM = 0x0D,    VMADN = 0x0E ,   VMADH = 0x0F,
     VADD = 0x10,     VSUB = 0x11,     VABS = 0x13,     VADDC = 0x14,
     VSUBC = 0x15,    VSAR = 0x1D,     VLT = 0x20,      VEQ = 0x21,
     VNE = 0x22,      VGE = 0x23,      VCL = 0x24,      VCH = 0x25,
     VCR = 0x26,      VMRG = 0x27,     VAND = 0x28,     VNAND = 0x29,
     VOR = 0x2A,      VNOR = 0x2B,     VXOR = 0x2C,     VNXOR = 0x2D,
     VRCP = 0x30,     VRCPL = 0x31,    VRCPH = 0x32,    VMOV = 0x33,
     VRSQ = 0x34,     VRSQL = 0x35,    VRSQH = 0x36,    VNOP = 0x37,   };

static char * rsp[] = {
    "special", "regimm", "j", "jal", "beq", "bne", "blez", "bgtz",
    "addi", "addiu", "slti", "sltiu", "andi", "ori", "xori", "lui",
    "cop0", 0, "cop2", 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    "lb", "lh", 0, "lw", "lbu", "lhu", 0, "lwu",
    "sb", "sh", 0, "sw", 0, 0, 0, 0,
    0, 0, "lwc2", 0, 0, 0, 0, 0,
    0, 0, "swc2", 0, 0, 0, 0, 0 };

static char * rsp_vec[] =   {
    "vmulf", "vmulu", "vrndp", "vmulq", "vmudl", "vmudm", "vmudn", "vmudh",
    "vmacf", "vmacu", "vrndn", "vmacq", "vmadl", "vmadm", "vmadn", "vmadh",
    "vadd", "vsub", 0, "vabs", "vaddc", "vsubc", 0, 0,
    0, 0, 0, 0, 0, "vsar", 0, 0,
    "vlt", "veq", "vne", "vge", "vcl", "vch", "vcr", "vmrg",
    "vand", "vnand", "vor", "vnor", "vxor", "vnxor", 0, 0,
    "vcrp", "vrcpl", "vrcph", "vmov", "vrsq", "vrsql", "vrsqh", "vnop" };

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

static char * rsp_lost[] = {
    "b", "s", "l", "d", "q", "r", "p", "u", "h", "f", "w", "t" };

static bool usingLongForm = true;
static bool usingArmipsCP0Names = true;

static char str_opcode[100];
static char str_bo[100];
static char str_beqo[100];
static char str_lost[100];

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

