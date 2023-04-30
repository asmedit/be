objects := editor.o buffer.o terminal.o hex/hex.o dasm/dasm.o \
	x86/insnsd.o x86/insnsa.o x86/insnsb.o x86/insnsn.o x86/disasm.o \
	x86/regdis.o x86/regs.o x86/regflags.o x86/regvals.o \
	x86/iflag.o x86/sync.o x86/disp8.o x86/nctype.o x86/readnum.o  \
	x86/common.o x86/alloc.o x86/string.o \
	x86/warnings.o x86/error.o x86/ver.o \
	arm/BranchExcSys.c arm/DataProcessingImmediate.c arm/LoadsAndStores.c \
	arm/bits.c arm/strext.c \
	arm/DataProcessingFloatingPoint.c arm/DataProcessingRegister.c \
	arm/armadillo.c arm/instruction.c arm/utils.c \
	riscv/riscv-disas.c

all: be
be: $(objects) be.o
clean:
	$(RM) $(objects) be.o $(objects:.o=.d) be
