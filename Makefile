objects := editor.o buffer.o terminal.o hex/hex.o dasm/dasm.o \
#	x86/insnsd.o x86/insnsa.o x86/insnsb.o x86/insnsn.o x86/disasm.o \
#	x86/regdis.o x86/regs.o x86/regflags.o x86/regvals.o \
#	x86/iflag.o x86/sync.o x86/disp8.o x86/nctype.o x86/readnum.o  \
#	x86/common.o x86/alloc.o x86/string.o \
#	x86/warnings.o x86/error.o x86/ver.o

all: xt
#x86/xt-dasm: $(objects) x86/xt-dasm.o
xt: $(objects) xt.o

clean:
	$(RM) $(objects) xt.o x86/xt-dasm.o $(objects:.o=.d) xt x86/xt-dasm
