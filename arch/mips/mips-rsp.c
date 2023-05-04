#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

enum GP {
     r0, at, v0, v1, a0, a1, a2, a3,
     t0, t1, t2, t3, t4, t5, t6, t7,
     s0, s1, s2, s3, s4, s5, s6, s7,
     t8, t9, k0, k1, gp, sp, s8, ra    };

static char * gpr[] =  {
     "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
     "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra" };

enum CP0 {
     DMA_CACHE,     DMA_DRAM,     DMA_READ_LENGTH,     DMA_WRITE_LENGTH,
     SP_STATUS,     DMA_FULL,     DMA_BUSY,            SP_RESERVED,
     CMD_START,     CMD_END,      CMD_CURRENT,         CMD_STATUS,
     CMD_CLOCK,     CMD_BUSY,     CMD_PIPE_BUSY,       CMD_TMEM_BUSY     };

static char * cp0[] =  {
     "sp_mem_addr", "sp_dram_addr", "sp_rd_len",    "sp_wr_len",
     "sp_status",   "sp_dma_full",  "sp_dma_busy",  "sp_semaphore",
     "dpc_start",   "dpc_end",      "dpc_current",  "dpc_status",
     "dpc_clock",   "dpc_bufbusy",  "dpc_pipebusy", "dpc_tmem" };

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
     LL = 0x30,       LWC1 = 0x31,     LWC2 = 0x32,     LWC3 = 0x33,
     LLD = 0x34,      LDC = 0x35,      LDC2 = 0x36,     LDC3 = 0x37,
     SC = 0x38,       SWC1 = 0x39,     SWC2 = 0x3A,     SWC3 = 0x3B,
     SCD = 0x3C,      SDC1 = 0x3D,     SDC2 = 0x3E,     SDC3 = 0x3F };

static char * rsp[] = {
    "special", "regimm", "j", "jal", "beq", "bne", "blez", "bgtz",
    "addi", "addiu", "slti", "sltiu", "andi", "ori", "xori", "lui",
    "cop0", "cop1", "cop2", "cop3", "beql", "bnel", "blezl", "bgtz",
    "daddi", "daddiu", "ldl", "ldr", 0, 0, 0, 0,
    "lb", "lh", "lwl", "lw", "lbu", "lhu", "lwr", "lwu",
    "sb", "sh", "swl", "sw", "sdl", "sdr", "swr", 0,
    "ll", "lwc1", "lwc2", "lwc3", 0, "ldc1", "ldc2", "ldc3",
    "sc", "swc1", "swc2", "swc3", 0, "sdc1", "sdc2", "sdc3" };

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
    "vadd", "vsub", 0, "vabs", "vaddc", "vsubc", 0, 0,
    0, 0, 0, 0, 0, "vsar", 0, 0,
    "vlt", "veq", "vne", "vge", "vcl", "vch", "vcr", "vmrg",
    "vand", "vnand", "vor", "vnor", "vxor", "vnxor", 0, 0,
    "vcrp", "vrcpl", "vrcph", "vmov", "vrsq", "vrsql", "vrsqh", "vnop" };

enum LOAD_STORE { b, s, l, d, q, r, p, u, h, f, w, t };

static char * rsp_lost[] = {
    "b", "s", "l", "d", "q", "r", "p", "u", "h", "f", "w", "t" };

static bool usingLongForm = true;
static bool usingArmipsCP0Names = true;

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
    switch(opcode) {
        case VRCP:
        case VRCPL:
        case VRCPH:
        case VMOV:
        case VRSQ:
        case VRSQL:
        case VRSQH:
            sprintf(str_vecop, "%s", decodeVectorElementScalar(opcode, operation));
        case VNOP:
            sprintf(str_vecop, "%s", "nop");
    }

    uint8_t e  = (uint8_t)((operation >> 21) & 0xF);
    uint8_t vt = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t vs = (uint8_t)((operation >> 11) & 0x1F);
    uint8_t vd = (uint8_t)((operation >> 6) & 0x1F);
    sprintf(unknown, "vop.%x", opcode);
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
     if (imm < 0) sprintf(str_lost,"%s %s, %s, %x", opcode, getRPRegName(dest), getRPRegName(base_), imm);
             else sprintf(str_lost,"%s %s, %s, %x", opcode, getRPRegName(dest), getRPRegName(base_), imm);
     return str_lost;
}

char *decodeOneRegisterWithImmediate(char *opcode, uint32_t operation)
{
     enum GP dest = (enum GP)((operation >> 16) & 0x1F);
     sprintf(str_reg, "%s %s, 0x%i", opcode, getRPRegName(dest), operation & 0xFFFF);
     return str_reg;
}

char *decodeThreeRegister(char* opcode, uint32_t operation, bool swapRT_RS)
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

char * decodeMIPS(uint32_t operation, unsigned long int address, char *outbuf)
{
    uint8_t subop = 0;
    if (operation == 0x00000000) {
        sprintf(outbuf, "%s", "nop");
        return outbuf;
    }
    enum RSP opcode = (enum RSP)((operation >> 26) & 0x3F);
    switch (opcode)
    {
        case J:
        case JAL:  sprintf(outbuf, "%s 0x0%x", rsp[opcode], ((operation & 0x03FFFFFF) << 2)); break;
        case BEQ:
        case BNE:  sprintf(outbuf, "%s", decodeBranchEquals(rsp[opcode], operation, address)); break;
        case BLEZ:
        case BGTZ: sprintf(outbuf, "%s", decodeBranch(rsp[opcode], operation, address)); break;
        case COP0: sprintf(outbuf, "%s", decodeCOP0(operation)); break;
        case COP1: sprintf(outbuf, "%s", decodeCOP0(operation)); break;
        case COP2: sprintf(outbuf, "%s", decodeCOP2(operation)); break;
        case COP3: sprintf(outbuf, "%s", decodeCOP2(operation)); break;
        case ADDI:
        case ADDIU:
        case SLTI:
        case SLTIU:
        case ANDI:
        case ORI:
        case XORI: sprintf(outbuf, "%s", decodeTwoRegistersWithImmediate(rsp[opcode], operation)); break;
        case LUI:  sprintf(outbuf, "%s", decodeOneRegisterWithImmediate(rsp[opcode], operation)); break;
        case LB:
        case LH:
        case LL:
        case LLD:
        case SCD:
        case LW:
        case LWL:
        case LWR:
        case LBU:
        case LHU:
        case LWU:
        case SC:
        case SB:
        case SH:
        case SWL:
        case LDC:
        case LDC2:
        case LDC3:
        case BGTZ2:
        case BEQL:
        case BNEL:
        case BLEZL:
        case DADDI:
        case DADDIU:
        case LDL:
        case LDR:
        case SW:   sprintf(outbuf, "%s", decodeNormalLoadStore(rsp[opcode],operation)); break;
        case LWC1: 
        case LWC3: 
        case LWC2: sprintf(outbuf, "%s", decodeLoadStore(operation, "l")); break;
        case SWC1:
        case SWC3:
        case SWC2: sprintf(outbuf, "%s", decodeLoadStore(operation, "s")); break;

        case REGIMM:
             subop = (uint8_t)((operation >> 16) & 0x1F);
             switch (subop) {
                 case 0x00: sprintf(outbuf, "%s", decodeBranch("bltz", operation, address)); break;
                 case 0x01: sprintf(outbuf, "%s", decodeBranch("bgez", operation, address)); break;
                 case 0x02: sprintf(outbuf, "%s", decodeBranch("bltzl", operation, address)); break;
                 case 0x03: sprintf(outbuf, "%s", decodeBranch("bgezl", operation, address)); break
;
                 case 0x08: sprintf(outbuf, "%s", decodeBranch("tgei", operation, address)); break;
                 case 0x09: sprintf(outbuf, "%s", decodeBranch("tgeiu", operation, address)); break;
                 case 0x0A: sprintf(outbuf, "%s", decodeBranch("tlti", operation, address)); break;
                 case 0x0B: sprintf(outbuf, "%s", decodeBranch("tltiu", operation, address)); break;
                 case 0x0C: sprintf(outbuf, "%s", decodeBranch("teqi", operation, address)); break;
                 case 0x0E: sprintf(outbuf, "%s", decodeBranch("tnei", operation, address)); break;

                 case 0x10: sprintf(outbuf, "%s", decodeBranch("bltzal", operation, address)); break;
                 case 0x11: sprintf(outbuf, "%s", decodeBranch("bgezal", operation, address)); break;
                 case 0x12: sprintf(outbuf, "%s", decodeBranch("bltzall", operation, address)); break;
                 case 0x13: sprintf(outbuf, "%s", decodeBranch("bgezall", operation, address)); break;
                 default:
                      sprintf(outbuf,"Unknown REGIMM opcode: 0x%x subop: 0x%x", opcode, subop);
             }
             break;
        case SPECIAL:
             subop = (uint8_t)(operation & 0x3F);
             switch (subop) {
                 case 0x00: sprintf(outbuf, "%s", decodeSpecialShift("sll", operation)); break;
                 case 0x02: sprintf(outbuf, "%s", decodeSpecialShift("slr", operation)); break;
                 case 0x03: sprintf(outbuf, "%s", decodeSpecialShift("sra", operation)); break;
                 case 0x04: sprintf(outbuf, "%s", decodeThreeRegister("sllv", operation, true)); break;
                 case 0x06: sprintf(outbuf, "%s", decodeThreeRegister("srlv", operation, true)); break;
                 case 0x07: sprintf(outbuf, "%s", decodeThreeRegister("srav", operation, true)); break;
                 case 0x08: sprintf(outbuf, "jr %s", getRPRegName(((enum GP)((operation >> 21) & 0x1F)))); break;
                 case 0x09:
                      enum GP return_reg = (enum GP)((operation >> 11) & 0x1F);
                      if (return_reg == ra) sprintf(outbuf, "jalr %s", getRPRegName(((enum GP)((operation >> 21) & 0x1F))));
                      else sprintf(outbuf, "jalr %s, %s", getRPRegName(return_reg), getRPRegName(((enum GP)((operation >> 21) & 0x1F))));
                      break;
                 case 0x0C: sprintf(outbuf, "syscall %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x0D: sprintf(outbuf, "break %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x0F: sprintf(outbuf, "sync %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x10: sprintf(outbuf, "mfhi %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x11: sprintf(outbuf, "mthi %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x12: sprintf(outbuf, "div %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x13: sprintf(outbuf, "divu %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x14: sprintf(outbuf, "dmult %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x15: sprintf(outbuf, "ddiv %i", ((operation >> 6) & 0xFFFFF)); break;
                 case 0x16: sprintf(outbuf, "ddivu %i", ((operation >> 6) & 0xFFFFF)); break;

                 case 0x20: sprintf(outbuf, "%s", decodeThreeRegister("add", operation, false)); break;
                 case 0x21: sprintf(outbuf, "%s", decodeThreeRegister("addu", operation, false)); break;
                 case 0x22: sprintf(outbuf, "%s", decodeThreeRegister("sub", operation, false)); break;
                 case 0x23: sprintf(outbuf, "%s", decodeThreeRegister("subu", operation, false)); break;
                 case 0x24: sprintf(outbuf, "%s", decodeThreeRegister("and", operation, false)); break;
                 case 0x25: sprintf(outbuf, "%s", decodeThreeRegister("or", operation, false)); break;
                 case 0x26: sprintf(outbuf, "%s", decodeThreeRegister("xor", operation, false)); break;
                 case 0x27: sprintf(outbuf, "%s", decodeThreeRegister("nor", operation, false)); break;
                 case 0x2A: sprintf(outbuf, "%s", decodeThreeRegister("slt", operation, false)); break;
                 case 0x2B: sprintf(outbuf, "%s", decodeThreeRegister("sltu", operation, false)); break;
                 case 0x2C: sprintf(outbuf, "%s", decodeThreeRegister("dadd", operation, false)); break;
                 case 0x2D: sprintf(outbuf, "%s", decodeThreeRegister("dsub", operation, false)); break;
                 case 0x2E: sprintf(outbuf, "%s", decodeThreeRegister("dsubu", operation, false)); break;
                 case 0x2F: sprintf(outbuf, "%s", decodeThreeRegister("dsubu", operation, false)); break;

                 case 0x30: sprintf(outbuf, "%s", decodeThreeRegister("tge", operation, false)); break;
                 case 0x31: sprintf(outbuf, "%s", decodeThreeRegister("tgeu", operation, false)); break;
                 case 0x32: sprintf(outbuf, "%s", decodeThreeRegister("tlt", operation, false)); break;
                 case 0x33: sprintf(outbuf, "%s", decodeThreeRegister("tltu", operation, false)); break;
                 case 0x34: sprintf(outbuf, "%s", decodeThreeRegister("teq", operation, false)); break;
                 case 0x36: sprintf(outbuf, "%s", decodeThreeRegister("tne", operation, false)); break;
                 case 0x38: sprintf(outbuf, "%s", decodeThreeRegister("dsll", operation, false)); break;
                 case 0x3A: sprintf(outbuf, "%s", decodeThreeRegister("dslr", operation, false)); break;
                 case 0x3B: sprintf(outbuf, "%s", decodeThreeRegister("dsra", operation, false)); break;
                 case 0x3C: sprintf(outbuf, "%s", decodeThreeRegister("dsll32", operation, false)); break;
                 case 0x3E: sprintf(outbuf, "%s", decodeThreeRegister("dslr32", operation, false)); break;
                 case 0x3F: sprintf(outbuf, "%s", decodeThreeRegister("dsra32", operation, false)); break;
                 default:
                      sprintf(outbuf,"Unknown SPECIAL opcode: 0x%x subop: 0x%x", opcode, subop);
                      break;
             }
             break;

        default:
             sprintf(outbuf,"Unknown RSP opcode: 0x%x", opcode);
             break;
    }
    return outbuf;
}

