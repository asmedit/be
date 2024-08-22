objects := be.o editor.o \
	hex/hex.o dasm/dasm.o term/buffer.o term/terminal.o \
	arch/x86/insnsd.o arch/x86/insnsa.o arch/x86/insnsb.o arch/x86/insnsn.o arch/x86/disasm.o \
	arch/x86/regdis.o arch/x86/regs.o arch/x86/regflags.o arch/x86/regvals.o \
	arch/x86/iflag.o arch/x86/sync.o arch/x86/disp8.o arch/x86/nctype.o arch/x86/readnum.o  \
	arch/x86/common.o arch/x86/alloc.o arch/x86/string.o \
	arch/x86/warnings.o arch/x86/error.o arch/x86/ver.o \
	arch/arm/BranchExcSys.o arch/arm/DataProcessingImmediate.o arch/arm/LoadsAndStores.o \
	arch/arm/bits.o arch/arm/strext.o \
	arch/arm/DataProcessingFloatingPoint.o arch/arm/DataProcessingRegister.o \
	arch/arm/armadillo.o arch/arm/instruction.o arch/arm/utils.o \
	arch/riscv/riscv-disas.o \
	arch/ppc/ppc_disasm.o \
	arch/m68k/dis68k.o \
	arch/sh4/sh4.o \
	arch/nv/nv.o \
	arch/mips/mips-rsp.o \
	arch/pdp11/pdp11.o
.PHONY: all
all: be
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
be: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)
.PHONY: install
install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m0755 be $(DESTDIR)$(PREFIX)/bin
.PHONY: clean
clean:
	$(RM) $(objects) $(objects:.o=.d) be
