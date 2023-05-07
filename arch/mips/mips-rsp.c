// Copyright (c) Namdak Tonpa
// MIPS-IV DASM 300 LOC

#include <stdint.h>
#include <stdio.h>

enum GP {
     r0, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7,
     s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, s8, ra    };

static char * gpr[] =  {
     "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
     "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra" };

enum CP0 {
     DMA_CACHE,     DMA_DRAM,     DMA_READ_LENGTH,     DMA_WRITE_LENGTH,
     SP_STATUS,     DMA_FULL,     DMA_BUSY,            SP_RESERVED,
     CMD_START,     CMD_END,      CMD_CURRENT,         CMD_STATUS,
     CMD_CLOCK,     CMD_BUSY,     CMD_PIPE_BUSY,       CMD_TMEM_BUSY     };

static char * cp0[] =  {
     "sp_mem_addr", "sp_dram_addr", "sp_rd_len",    "sp_wr_len", "sp_status",   "sp_dma_full",  "sp_dma_busy",  "sp_semaphore",
     "dpc_start",   "dpc_end",      "dpc_current",  "dpc_status", "dpc_clock",   "dpc_bufbusy",  "dpc_pipebusy", "dpc_tmem" };

enum RSP {
     SPECIAL = 0x00,  REGIMM = 0x01,   J = 0x02,        JAL = 0x03,
     BEQ = 0x04,      BNE = 0x05,      BLEZ = 0x06,     BGTZ = 0x07,
     ADDI = 0x08,     ADDIU = 0x09,    SLTI = 0x0A,     SLTIU = 0x0B,
     ANDI = 0x0C,     ORI = 0x0D,      XORI = 0x0E,     LUI = 0x0F,
     COP0 = 0x10,     COP1 = 0x11,     COP2 = 0x12,     COP3 = 0x13,
     BEQL = 0x14,     BNEL = 0x15,     BLEZL = 0x16,    BGTZ2 = 0x17,
     DADDI = 0x18,    DADDIU = 0x19,   LDL = 0x1A,      LDR = 0x1B,
     LB = 0x20,       LH = 0x21,       LWL  = 0x22,     LW = 0x23,
     LBU = 0x24,      LHU = 0x25,      LWR = 0x26,      LWU = 0x27,
     SB = 0x28,       SH = 0x29,       SWL = 0x2A,      SW = 0x2B,
     SDL = 0x2C,      SDR = 0x2D,      SWR = 0x2E,
     LL = 0x30,       LWC1 = 0x31,     LWC2 = 0x32,     PREF = 0x33,
     LLD = 0x34,      LDC1 = 0x35,     LDC2 = 0x36,     LD = 0x37,
     SC = 0x38,       SWC1 = 0x39,     SWC2 = 0x3A,
     SCD = 0x3C,      SDC1 = 0x3D,     SDC2 = 0x3E,     SD = 0x3F };

char * rsp[] = {
    "special", "regimm", "j", "jal", "beq", "bne", "blez", "bgtz", "addi", "addiu", "slti", "sltiu", "andi", "ori", "xori", "lui",
    "cop0", "cop1", "cop2", "cop3", "beql", "bnel", "blezl", "bgtz", "daddi", "daddiu", "ldl", "ldr", 0, 0, 0, 0,
    "lb", "lh", "lwl", "lw", "lbu", "lhu", "lwr", "lwu", "sb", "sh", "swl", "sw", "sdl", "sdr", "swr", 0,
    "ll", "lwc1", "lwc2", "pref", "lld", "ldc1", "ldc2", "ld", "sc", "swc1", "swc2", 0, "scd", "sdc1", "sdc2", "sd" };

enum SPECIAL {
     SLL = 0x00,      MOVCI = 0x01,    SLR = 0x02,      SRA = 0x03,
     SLLV = 0x04,     SRLV = 0x06,     SRAV = 0x07,     JR = 0x08,
     JALR = 0x09,     MOVZ = 0x0A,     MOVN = 0x0B,     SYSCALL = 0x0C,
     BREAK = 0x0D,    SYNC = 0x0F,     MFHI = 0x10,     MTHI = 0x11,
     MFLO = 0x12,     MTLO = 0x13,     DSLLV = 0x14,    DSRLV = 0x16,
     DSRAV = 0x17,    MULT = 0x18,     MULTU = 0x19,    DIV = 0x1A,
     DIVU = 0x1B,     DMULT = 0x1C,    DMULTU = 0x1D,   DDIV = 0x1E,
     DDIVU = 0x1F,    ADD = 0x20,      ADDU = 0x21,     SUB = 0x22,
     SUBU = 0x23,     AND = 0x24,      OR = 0x25,       XOR = 0x26,
     NOR = 0x27,      SLT = 0x2A,      SLTU = 0x2B,     DADD = 0x2C,
     DADDU = 0x2D,    DSUB = 0x2E,     DSUBU = 0x2F,    TGE = 0x30,
     TGEU = 0x31,     TLT = 0x32,      TLTU = 0x33,     TEQ = 0x34,
     TNE = 0x36,      DSLL = 0x38,     DSRL = 0x3A,     DSRA = 0x3B,
     DSLL32 = 0x3C,   DSRL32 = 0x3E,   DSRA32 = 0x3F };

char *specials[] = {
     "sll", "movci", "slr", "sra", "sllv", 0, "srlv", "srav", "jr", "jalr", "movz", "movn", "syscall", "break", 0, "sync",
     "mfhi", "mthi", "mflo", "mtlo", "dsllv", 0, "dsrlv", "dsrav", "mult", "multu", "div", "divu", "dmult", "dmultu", "ddiv", "ddivu",
     "add", "addu", "sub", "subu", "and", "or", "xor", "nor", 0, 0, "slt", "sltu", "dadd", "daddu", "dsub", "dsubu",
     "tge", "tgeu", "tlt", "tltu", "teq", 0, "tne", 0, "dsll", 0, "dsrl", "dsra", "dsll32", 0, "dsrl32", "dsra32", 0 };

enum REGIMM {
     BLTZ = 0x00,     BGEZ = 0x01,     BLTZL = 0x02,    BGEZL = 0x03,
     TGEI = 0x08,     TGEIU = 0x09,    TLTI = 0x0A,     TLTIU = 0x0B,
     TEQI = 0x0C,     TNEI = 0x0E,     BLTZAL = 0x10,   BGEZAL = 0x11,
     BLTZALL = 0x12 };

char *regimm[] = {
     "bltz", "bgez", "bltzl", "bgezl", 0, 0, 0, 0, "tgei", "tgeiu", "tlti", "tltiu", "teqi", 0, "tnei", 0,
     "bltzal", "bgezal", "bltzall", "bgezall", 0 };

enum VEC {
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

static char * rsp_vec[] =   {
    "vmulf", "vmulu", "vrndp", "vmulq", "vmudl", "vmudm", "vmudn", "vmudh",
    "vmacf", "vmacu", "vrndn", "vmacq", "vmadl", "vmadm", "vmadn", "vmadh",
    "vadd", "vsub", 0, "vabs", "vaddc", "vsubc", 0, 0, 0, 0, 0, 0, 0, "vsar", 0, 0,
    "vlt", "veq", "vne", "vge", "vcl", "vch", "vcr", "vmrg",
    "vand", "vnand", "vor", "vnor", "vxor", "vnxor", 0, 0,
    "vcrp", "vrcpl", "vrcph", "vmov", "vrsq", "vrsql", "vrsqh", "vnop" };

enum LOAD_STORE { b, s, l, d, q, r, p, u, h, f, w, t };

static char * rsp_lost[] = { "b", "s", "l", "d", "q", "r", "p", "u", "h", "f", "w", "t" };

static int usingLongForm = 1;
static int usingArmipsCP0Names = 1;

static char str_opcode[256];
static char str_bo[256];
static char str_beqo[256];
static char str_lost[256];
static char str_vec_elem[256];
static char str_vec[256];
static char str_vecop[256];
static char str_cop0[256];
static char str_cop2[256];
static char str_mc[256];
static char str_reg[256];
static char unknown[256];

char *getRPRegName(enum GP reg) { return gpr[reg]; }
char *getCP0RegName(enum CP0 reg) { return cp0[reg]; }

char *decodeVectorElement(uint8_t v, uint8_t e)
{
    if ((e & 0x8) == 8) sprintf(str_vec_elem,"%i[%i]", v, (e & 0x7));
    else if ((e & 0xC) == 4) sprintf(str_vec_elem,"%i[%ih]", v, (e & 0x3));
    else if ((e & 0xE) == 2) sprintf(str_vec_elem,"%i[%iq]", v, (e & 0x1));
    else sprintf(str_vec_elem,"%i", v);
    return str_vec_elem;
}

char *decodeVectorElementScalar(enum VEC opcode, uint32_t operation)
{
    uint8_t e  = (uint8_t)((operation >> 21) & 0xF);
    uint8_t vt = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t de = (uint8_t)((operation >> 11) & 0x1F);
    uint8_t vd = (uint8_t)((operation >> 6) & 0x1F);
    sprintf(str_vec,"%s $v%s, $v%s", rsp_vec[opcode], decodeVectorElement(vd, de), decodeVectorElement(vt, e));
    return str_vec;
}

char *decodeVector(uint32_t operation)
{
    enum VEC opcode = (enum VEC)(operation & 0x3F);
    sprintf(unknown, "vop.%x", opcode);
    switch (opcode) {
        case VRCP:
        case VRCPL:
        case VRCPH:
        case VMOV:
        case VRSQ:
        case VRSQL:
        case VRSQH: sprintf(str_vecop, "%s", decodeVectorElementScalar(opcode, operation)); break;
        case VNOP:  sprintf(str_vecop, "%s", "nop"); break;
        default: break;
    }

    uint8_t e  = (uint8_t)((operation >> 21) & 0xF);
    uint8_t vt = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t vs = (uint8_t)((operation >> 11) & 0x1F);
    uint8_t vd = (uint8_t)((operation >> 6) & 0x1F);

    sprintf(str_vecop, "%s $v%i, $v%i, $v%s", (rsp_vec[opcode] == 0 ? unknown : rsp_vec[opcode]),
       vd, vs, decodeVectorElement(vt, e));
    return str_vecop;
}

char *decodeMoveControlToFromCoprocessor(char *opcode, uint32_t operation)
{
    enum GP rt = (enum GP)((operation >> 16) & 0x1F);
    uint8_t rd = (uint8_t)((operation >> 11) & 0x1F);
    sprintf(str_mc, "%s %s, $v%i", opcode, getRPRegName(rt), rd);
    return str_mc;
}

char * decodeMoveToFromCoprocessor(char *opcode, uint32_t operation)
{
    enum GP rt = (enum GP)((operation >> 16) & 0x1F);
    uint8_t rd = (uint8_t)((operation >> 11) & 0x1F);
    uint8_t e  = (uint8_t)((operation >> 7) & 0xF);
    sprintf(str_mc, "%s %s, $v%i[%i]", opcode, getRPRegName(rt), rd, e);
    return str_mc;
}

char *decodeCOP0(uint32_t operation) {
     uint8_t mt = (uint8_t)((operation >> 21) & 0x1F);
     enum GP rt = (enum GP)((operation >> 16) & 0x1F);
     enum CP0 rd = (enum CP0)((operation >> 11) & 0x1F);
     switch (mt) {
         case 0x00: sprintf(str_cop0, "mfc0 %s, %s", getRPRegName(rt), getCP0RegName(rd)); break;
         case 0x04: sprintf(str_cop0, "mtc0 %s, %s", getRPRegName(rt), getCP0RegName(rd)); break;
         default:   sprintf(str_cop0, "Unimplemented COP0 opcode 0x%x", mt); break;
     }

     return str_cop0;
}

char *decodeCOP2(uint32_t operation) {
    if ((operation & 0x7FF) != 0) return decodeVector(operation);
    uint8_t subop = (uint8_t)((operation >> 21) & 0x1F);
    switch (subop) {
        case 0x00: return decodeMoveToFromCoprocessor("mfc2", operation);
        case 0x04: return decodeMoveToFromCoprocessor("mtc2", operation);
        case 0x02: return decodeMoveControlToFromCoprocessor("cfc2", operation);
        case 0x06: return decodeMoveControlToFromCoprocessor("ctc2", operation);
        default:   sprintf(str_cop2, "Unimplemented COP2 opcode: 0x%x", subop);
                   return str_cop2;
    }
}

char * decodeLoadStore(uint16_t operation, char *LorS)
{
    enum LOAD_STORE ls_subop = (enum LOAD_STORE)((operation >> 11) & 0x1F);
    sprintf(str_lost,"%s%sv", LorS, rsp_lost[ls_subop]);
    uint8_t base_ = (uint8_t)((operation >> 21) & 0x1F);
    uint8_t dest = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t del = (uint8_t)((operation >> 7) & 0xF);
    uint16_t offset = (uint16_t)((operation & 0x3F) << 2);
    sprintf(str_lost,"%s%sv $v%i[%d], 0x%4x(%s)", LorS, rsp_lost[ls_subop], dest, del, offset, getRPRegName(base_));
    return str_lost;
}

char *decodeNormalLoadStore(char *opcode, uint32_t operation)
{
     enum GP dest = (enum GP)((operation >> 16) & 0x1F);
     enum GP base_ = (enum GP)((operation >> 21) & 0x1F);
     uint16_t imm = (uint16_t)(operation & 0xFFFF);
     if (imm < 0) sprintf(str_lost,"%s %s, -0x%x(%s)", opcode, getRPRegName(dest), imm, getRPRegName(base_));
             else sprintf(str_lost,"%s %s, 0x%x(%s)", opcode, getRPRegName(dest), imm, getRPRegName(base_));
     return str_lost;
}

char *decodeTwoRegistersWithImmediate(char *opcode, uint32_t operation)
{
     enum GP dest = (enum GP)((operation >> 16) & 0x1F);
     enum GP base_  = (enum GP)((operation >> 21) & 0x1F);
     uint16_t imm = (uint16_t)(operation & 0xFFFF);
     if (imm < 0) sprintf(str_lost,"%s %s, %s, 0x%x", opcode, getRPRegName(dest), getRPRegName(base_), imm);
             else sprintf(str_lost,"%s %s, %s, 0x%x", opcode, getRPRegName(dest), getRPRegName(base_), imm);
     return str_lost;
}

char *decodeOneRegisterWithImmediate(char *opcode, uint32_t operation)
{
     enum GP dest = (enum GP)((operation >> 16) & 0x1F);
     sprintf(str_reg, "%s %s, 0x%x", opcode, getRPRegName(dest), operation & 0xFFFF);
     return str_reg;
}

char *decodeThreeRegister(char* opcode, uint32_t operation, int swapRT_RS)
{
     enum GP dest = (enum GP)((operation >> 11) & 0x1F);
     enum GP src1 = (enum GP)((operation >> 21) & 0x1F);
     enum GP src2 = (enum GP)((operation >> 16) & 0x1F);
     if(!swapRT_RS)
        sprintf(str_reg,"%s %s, %s, %s", opcode, getRPRegName(dest), getRPRegName(src1), getRPRegName(src2));
     else 
        sprintf(str_reg,"%s %s, %s, %s", opcode, getRPRegName(dest), getRPRegName(src2), getRPRegName(src1));
     return str_reg;
}
char * decodeBranch(char *opcode, uint32_t operation, unsigned long int address)
{
     enum GP src = (enum GP)((operation >> 21) & 0x1F);
     uint16_t imm = (uint16_t)((operation & 0xFFFF) << 2);
     uint32_t current_offset = (uint32_t)((address + 4) + imm);
     sprintf(str_bo, "%s %s, 0x%8x", opcode, getRPRegName(src), current_offset);
     return str_bo;
}

char * decodeBranchEquals(char * opcode, uint32_t operation, unsigned long int address)
{
     enum GP src1 = (enum GP)((operation >> 21) & 0x1F);
     enum GP src2 = (enum GP)((operation >> 16) & 0x1F);
     uint16_t imm = (uint16_t)((operation & 0xFFFF) << 2);
     uint32_t current_offset = (uint32_t)((address + 4) + imm);
     sprintf(str_beqo, "%s %s, %s, 0x%8x", opcode, getRPRegName(src1), getRPRegName(src2), current_offset);
     return str_beqo;
}

char *decodeSpecialShift(char *opcode, uint32_t operation)
{
     enum GP dest = (enum GP)((operation >> 11) & 0x1F);
     enum GP src  = (enum GP)((operation >> 16) & 0x1F);
     int imm = (int)((operation >> 6) & 0x1F);
     sprintf(str_lost, "%s %s, %s, %i", opcode, getRPRegName(dest), getRPRegName(src), imm);
     return str_lost;
}

uint32_t le_to_be(uint32_t num) {
    uint8_t b[4] = {0};
    *(uint32_t*)b = num;
    uint8_t tmp = 0;
    tmp = b[0];
    b[0] = b[3];
    b[3] = tmp;
    tmp = b[1];
    b[1] = b[2];
    b[2] = tmp;
    return *(uint32_t*)b;
}

char * decodeMIPS(unsigned long int address, char *outbuf, int*lendis)
{
    uint32_t operation = le_to_be( (uint32_t)*((unsigned long int *)address));
    if (operation == 0x00000000) { sprintf(outbuf, "%s", "nop"); return outbuf; }
    enum GP reg = (enum GP)((operation >> 21) & 0x1F);
    enum RSP opcode = (enum RSP)((operation >> 26) & 0x3F);
    if (opcode == 0x00) {
        uint8_t function = (uint8_t)(operation & 0x3F);
        if (function < 0x04) sprintf(outbuf, "%s", decodeSpecialShift(specials[function], operation));
        else if (function < 0x08) sprintf(outbuf, "%s", decodeThreeRegister(specials[function], operation, 1));
        else if (function == 0x08) sprintf(outbuf, "%s %s", specials[function], getRPRegName(reg));
        else if (function == 0x09) {
             enum GP return_reg = (enum GP)((operation >> 11) & 0x1F);
             if (return_reg == ra) sprintf(outbuf, "%s %s", specials[function], getRPRegName(reg));
             else sprintf(outbuf, "%s %s, %s", specials[function], getRPRegName(return_reg), getRPRegName(reg));
        } else if (function < 0x20) sprintf(outbuf, "%s %i", specials[function], ((operation >> 6) & 0xFFFFF));
        else if (function < 0x40) sprintf(outbuf, "%s", decodeThreeRegister(specials[function], operation, 0));
        else sprintf(outbuf,"Unknown SPECIAL opcode: 0x%x function: 0x%x", opcode, function);
    } else if (opcode == 0x01) {
        uint8_t rt = (uint8_t)((operation >> 16) & 0x1F);
        if (rt < 0x14) sprintf(outbuf, "%s", decodeBranch(regimm[rt], operation, address));
        else sprintf(outbuf,"Unknown REGIMM opcode: 0x%x rt: 0x%x", opcode, rt);
    } else if (opcode  < 0x04) sprintf(outbuf, "%s 0x0%x", rsp[opcode], ((operation & 0x03FFFFFF) << 2));
    else if (opcode  < 0x06) sprintf(outbuf, "%s", decodeBranchEquals(rsp[opcode], operation, address));
    else if (opcode  < 0x08) sprintf(outbuf, "%s", decodeBranch(rsp[opcode], operation, address));
    else if (opcode  < 0x0F) sprintf(outbuf, "%s", decodeTwoRegistersWithImmediate(rsp[opcode], operation));
    else if (opcode  < 0x10) sprintf(outbuf, "%s", decodeOneRegisterWithImmediate(rsp[opcode], operation));
    else if (opcode == 0x10) sprintf(outbuf, "%s", decodeCOP0(operation));
    else if (opcode == 0x12) sprintf(outbuf, "%s", decodeCOP2(operation));
    else if (opcode  < 0x2F) sprintf(outbuf, "%s", decodeNormalLoadStore(rsp[opcode],operation));
    else if (opcode  < 0x38) sprintf(outbuf, "%s", decodeLoadStore(operation, "l"));
    else if (opcode  < 0x3F) sprintf(outbuf, "%s", decodeLoadStore(operation, "s"));
    else sprintf(outbuf,"Unknown RSP opcode: 0x%x", opcode);
    *lendis = 4;
    return outbuf;
}
