// Copyright (c) Namdak Tonpa
// MIPS-IV DASM 200 LOC

#include <stdint.h>
#include <stdio.h>

// regs

char * rsp_lost[] = { "b", "s", "l", "d", "q", "r", "p", "u", "h", "f", "w", "t" };
char * gpr[] =  {  "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                   "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra" };
char * cp0[] =  {  "sp_mem_addr", "sp_dram_addr", "sp_rd_len",    "sp_wr_len", "sp_status",   "sp_dma_full",  "sp_dma_busy",  "sp_semaphore",
                   "dpc_start",   "dpc_end",      "dpc_current",  "dpc_status", "dpc_clock",   "dpc_bufbusy",  "dpc_pipebusy", "dpc_tmem" };

// opcodes

char * rsp[] = {   "special", "regimm", "j", "jal", "beq", "bne", "blez", "bgtz", "addi", "addiu", "slti", "sltiu", "andi", "ori", "xori", "lui",
                   "cop0", "cop1", "cop2", "cop3", "beql", "bnel", "blezl", "bgtz", "daddi", "daddiu", "ldl", "ldr", 0, 0, 0, 0,
                   "lb", "lh", "lwl", "lw", "lbu", "lhu", "lwr", "lwu", "sb", "sh", "swl", "sw", "sdl", "sdr", "swr", 0,
                   "ll", "lwc1", "lwc2", "pref", "lld", "ldc1", "ldc2", "ld", "sc", "swc1", "swc2", 0, "scd", "sdc1", "sdc2", "sd" };
char *specials[] = { "sll", "movci", "slr", "sra", "sllv", 0, "srlv", "srav", "jr", "jalr", "movz", "movn", "syscall", "break", 0, "sync",
                     "mfhi", "mthi", "mflo", "mtlo", "dsllv", 0, "dsrlv", "dsrav", "mult", "multu", "div", "divu", "dmult", "dmultu", "ddiv", "ddivu",
                     "add", "addu", "sub", "subu", "and", "or", "xor", "nor", 0, 0, "slt", "sltu", "dadd", "daddu", "dsub", "dsubu",
                     "tge", "tgeu", "tlt", "tltu", "teq", 0, "tne", 0, "dsll", 0, "dsrl", "dsra", "dsll32", 0, "dsrl32", "dsra32", 0 };
char *regimm[] = { "bltz", "bgez", "bltzl", "bgezl", 0, 0, 0, 0, "tgei", "tgeiu", "tlti", "tltiu", "teqi", 0, "tnei", 0,
                   "bltzal", "bgezal", "bltzall", "bgezall", 0 };
char * rsp_vec[] = { "vmulf", "vmulu", "vrndp", "vmulq", "vmudl", "vmudm", "vmudn", "vmudh",
                     "vmacf", "vmacu", "vrndn", "vmacq", "vmadl", "vmadm", "vmadn", "vmadh",
                     "vadd", "vsub", 0, "vabs", "vaddc", "vsubc", 0, 0, 0, 0, 0, 0, 0, "vsar", 0, 0,
                     "vlt", "veq", "vne", "vge", "vcl", "vch", "vcr", "vmrg",
                     "vand", "vnand", "vor", "vnor", "vxor", "vnxor", 0, 0,
                     "vcrp", "vrcpl", "vrcph", "vmov", "vrsq", "vrsql", "vrsqh", "vnop" };

char str_opcode[256], str_bo[256], str_beqo[256], str_lost[256], str_vec_elem[256], str_vec[256], str_vecop[256], str_cop0[256];
char str_cop2[256], str_mc[256], str_reg[256], unknown[256];

char *decodeVectorElement(uint8_t v, uint8_t e)
{
    if ((e & 0x8) == 8) sprintf(str_vec_elem,"%i[%i]", v, (e & 0x7));
    else if ((e & 0xC) == 4) sprintf(str_vec_elem,"%i[%ih]", v, (e & 0x3));
    else if ((e & 0xE) == 2) sprintf(str_vec_elem,"%i[%iq]", v, (e & 0x1));
    else sprintf(str_vec_elem,"%i", v);
    return str_vec_elem;
}

char *decodeVectorElementScalar(uint8_t opcode, uint32_t operation)
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
    uint8_t opcode = (uint8_t)(operation & 0x3F);
    if (opcode == 0x37) { sprintf(str_vecop, "%s", "nop"); return str_vecop; }
    else if (opcode < 0x37) { sprintf(str_vecop, "%s", decodeVectorElementScalar(opcode, operation)); return str_vecop; }

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
    uint8_t rt = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t rd = (uint8_t)((operation >> 11) & 0x1F);
    sprintf(str_mc, "%s %s, $v%i", opcode, gpr[rt], rd);
    return str_mc;
}

char * decodeMoveToFromCoprocessor(char *opcode, uint32_t operation)
{
    uint8_t rt = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t rd = (uint8_t)((operation >> 11) & 0x1F);
    uint8_t e  = (uint8_t)((operation >> 7) & 0xF);
    sprintf(str_mc, "%s %s, $v%i[%i]", opcode, gpr[rt], rd, e);
    return str_mc;
}

char *decodeCOP0(uint32_t operation) {
     uint8_t mt = (uint8_t)((operation >> 21) & 0x1F);
     uint8_t rt = (uint8_t)((operation >> 16) & 0x1F);
     uint8_t rd = (uint8_t)((operation >> 11) & 0x1F);
     switch (mt) {
         case 0x00: sprintf(str_cop0, "mfc0 %s, %s", gpr[rt], cp0[rd]); break;
         case 0x04: sprintf(str_cop0, "mtc0 %s, %s", gpr[rt], cp0[rd]); break;
         default:   sprintf(str_cop0, "Unknown COP0 opcode: 0x%x", mt); break;
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
        default:   sprintf(str_cop2, "Unknown COP2 opcode: 0x%x", subop);
                   return str_cop2;
    }
}

char * decodeLoadStore(uint16_t operation, char *LorS)
{
    uint8_t ls_subop = (uint8_t)((operation >> 11) & 0x1F);
    sprintf(str_lost,"%s%sv", LorS, rsp_lost[ls_subop]);
    uint8_t src = (uint8_t)((operation >> 21) & 0x1F);
    uint8_t dst = (uint8_t)((operation >> 16) & 0x1F);
    uint8_t del = (uint8_t)((operation >> 7) & 0xF);
    uint16_t offset = (uint16_t)((operation & 0x3F) << 2);
    sprintf(str_lost,"%s%sv $v%i[%d], 0x%4x(%s)", LorS, rsp_lost[ls_subop], dst, del, offset, gpr[src]);
    return str_lost;
}

char *decodeNormalLoadStore(char *opcode, uint32_t operation)
{
     uint8_t dst = (uint8_t)((operation >> 16) & 0x1F);
     uint8_t src = (uint8_t)((operation >> 21) & 0x1F);
     uint16_t imm = (uint16_t)(operation & 0xFFFF);
     if (imm < 0) sprintf(str_lost,"%s %s, -0x%x(%s)", opcode, gpr[dst], imm, gpr[src]);
             else sprintf(str_lost,"%s %s, 0x%x(%s)", opcode, gpr[dst], imm, gpr[src]);
     return str_lost;
}

char *decodeTwoRegistersWithImmediate(char *opcode, uint32_t operation)
{
     uint8_t dst = (uint8_t)((operation >> 16) & 0x1F);
     uint8_t src = (uint8_t)((operation >> 21) & 0x1F);
     uint16_t imm = (uint16_t)(operation & 0xFFFF);
     if (imm < 0) sprintf(str_lost,"%s %s, %s, 0x%x", opcode, gpr[dst], gpr[src], imm);
             else sprintf(str_lost,"%s %s, %s, 0x%x", opcode, gpr[dst], gpr[src], imm);
     return str_lost;
}

char *decodeOneRegisterWithImmediate(char *opcode, uint32_t operation)
{
     uint8_t dst = (uint8_t)((operation >> 16) & 0x1F);
     sprintf(str_reg, "%s %s, 0x%x", opcode, gpr[dst], operation & 0xFFFF);
     return str_reg;
}

char *decodeThreeRegister(char* opcode, uint32_t operation, int swapRT_RS)
{
     uint8_t dest = (uint8_t)((operation >> 11) & 0x1F);
     uint8_t src1 = (uint8_t)((operation >> 21) & 0x1F);
     uint8_t src2 = (uint8_t)((operation >> 16) & 0x1F);
     if(!swapRT_RS) sprintf(str_reg,"%s %s, %s, %s", opcode, gpr[dest], gpr[src1], gpr[src2]);
     else sprintf(str_reg,"%s %s, %s, %s", opcode, gpr[dest], gpr[src2], gpr[src1]);
     return str_reg;
}
char * decodeBranch(char *opcode, uint32_t operation, unsigned long int address)
{
     uint8_t src = (uint8_t)((operation >> 21) & 0x1F);
     uint16_t imm = (uint16_t)((operation & 0xFFFF) << 2);
     uint32_t current_offset = (uint32_t)((address + 4) + imm);
     sprintf(str_bo, "%s %s, 0x%8x", opcode, gpr[src], current_offset);
     return str_bo;
}

char * decodeBranchEquals(char * opcode, uint32_t operation, unsigned long int address)
{
     uint8_t src1 = (uint8_t)((operation >> 21) & 0x1F);
     uint8_t src2 = (uint8_t)((operation >> 16) & 0x1F);
     uint16_t imm = (uint16_t)((operation & 0xFFFF) << 2);
     uint32_t current_offset = (uint32_t)((address + 4) + imm);
     sprintf(str_beqo, "%s %s, %s, 0x%08x", opcode, gpr[src1], gpr[src2], current_offset);
     return str_beqo;
}

char *decodeSpecialShift(char *opcode, uint32_t operation)
{
     uint8_t dest = (uint8_t)((operation >> 11) & 0x1F);
     uint8_t src  = (uint8_t)((operation >> 16) & 0x1F);
     int imm = (int)((operation >> 6) & 0x1F);
     sprintf(str_lost, "%s %s, %s, %i", opcode, gpr[dest], gpr[src], imm);
     return str_lost;
}

uint32_t le_to_be(uint32_t num) {
    uint8_t b[4] = {0}; *(uint32_t*)b = num; uint8_t tmp = 0; tmp = b[0];
    b[0] = b[3]; b[3] = tmp; tmp = b[1]; b[1] = b[2]; b[2] = tmp; return *(uint32_t*)b;
}

char * decodeMIPS(unsigned long int address, char *outbuf, int*lendis, unsigned long int offset)
{
    uint32_t operation = le_to_be((uint32_t)*((unsigned long int *)address));
    if (operation == 0x00000000) { sprintf(outbuf, "%s", "nop"); return outbuf; }
    uint8_t reg = (uint8_t)((operation >> 21) & 0x1F);
    uint8_t opcode = (uint8_t)((operation >> 26) & 0x3F);
    if (opcode == 0x00) {
        uint8_t function = (uint8_t)(operation & 0x3F);
        if (function < 0x04) sprintf(outbuf, "%s", decodeSpecialShift(specials[function], operation));
        else if (function < 0x08) sprintf(outbuf, "%s", decodeThreeRegister(specials[function], operation, 1));
        else if (function == 0x08) sprintf(outbuf, "%s %s", specials[function], gpr[reg]);
        else if (function == 0x09) {
             uint8_t return_reg = (uint8_t)((operation >> 11) & 0x1F);
             if (return_reg == 0xF) sprintf(outbuf, "%s %s", specials[function], gpr[reg]);
             else sprintf(outbuf, "%s %s, %s", specials[function], gpr[return_reg], gpr[reg]);
        } else if (function < 0x20) sprintf(outbuf, "%s %i", specials[function], ((operation >> 6) & 0xFFFFF));
        else if (function < 0x40) sprintf(outbuf, "%s", decodeThreeRegister(specials[function], operation, 0));
        else sprintf(outbuf,"Unknown SPECIAL opcode: 0x%x function: 0x%x", opcode, function);
    } else if (opcode == 0x01) {
        uint8_t rt = (uint8_t)((operation >> 16) & 0x1F);
        if (rt < 0x14) sprintf(outbuf, "%s", decodeBranch(regimm[rt], operation, offset));
        else sprintf(outbuf,"Unknown REGIMM opcode: 0x%x rt: 0x%x", opcode, rt);
    } else if (opcode  < 0x04) sprintf(outbuf, "%s 0x0%x", rsp[opcode], ((operation & 0x03FFFFFF) << 2));
    else if (opcode  < 0x06) sprintf(outbuf, "%s", decodeBranchEquals(rsp[opcode], operation, offset));
    else if (opcode  < 0x08) sprintf(outbuf, "%s", decodeBranch(rsp[opcode], operation, offset));
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
