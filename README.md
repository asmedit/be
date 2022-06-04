Binary Editor
=============

The `be` hacker editor aims to be a contemporary scalable `hiew` replacement.
Basically it is the `hx` hex editor with disassembly mode.
The `nasm` is used as disassembly library with support of 16, 32, 64
segment sizes and all Intel extensions:
MMX, SSE, SSE2, SSE3, SSE4.1 SSE4.2, SSE5, KNI, AVX, AVX-2, AVX-512,
AES, AMX, TSX, MPX, FMA, VNNI, GFNI, SGX, CET, PKU.
Their x86 disassembler one of the most sane and compact in industry, however it should be reduced to 32K.

In the future `be` should have `macho`, `elf`, and `pe` containers
support along with RISC-V and AArch64 disassemblers.
Unfortunately I couldn't find appropriate ARM disassembler codebase to derive
so maybe I should finish <a href="https://tonpa.guru/stream/2019/2019-06-09%20A64%20Assembler.htm">one of my own</a>. Futunately MIPS and PowerPC disassemblers are easy to implement. There could also `be` 8-bit pivot for 8080/8085/Z80, 6502, HC08, F8 or for bytecode of virtual machines: BEAM, EBC, etc.

The `be` default used for HEX view is 192-bit width byte stream and for ASM view is 64-bit mode EM64T 16-byte instruction pipeline. For comfortable usage this requires 120 column terminal.

<img width="1250" alt="Screenshot 2022-06-04 at 09 23 11" src="https://user-images.githubusercontent.com/144776/171987389-7cafd20d-cdd7-4f39-9c1e-20c40915cc13.png">

<img width="1250" alt="Screenshot 2022-06-04 at 09 23 00" src="https://user-images.githubusercontent.com/144776/171987401-a0b57cd3-2c00-4a62-8df6-741f871c5087.png">

Credits
-------

* Kevin Pors, <a href="https://github.com/krpors/hx">hx</a>
* NASM Authors, <a href="https://github.com/netwide-assembler/nasm">nasm</a>
