Binary Editor
=============

The `be` hacker editor aims to be a contemporary scalable `hiew` replacement.
Basically this is the `hx` hex editor with disassembly mode.
The `nasm` is used as disassembly library with support of 16, 32, 64 segment sizes and all Intel extensions:
MMX, SSE, SSE2, SSE3, SSE4.1 SSE4.2, SSE5, KNI, AVX, AVX-2, AVX-512, AES, AMX, TSX, MPX, FMA, VNNI, GFNI, SGX, CET, PKU.
Their x86 disassembler one of most sane in industry, however it should be reduced to 32K.

In the future there will `be` containing parsers for `macho`, `elf`, and `pe` formats
along with complex (speaking of full feature set as in `nasm`) RISC-V 128 and AArch64 disassemblers.
Unfortunately I couldn't find appropriate ARM disassembler codebase to derive so maybe I should finish
<a href="https://tonpa.guru/stream/2019/2019-06-09%20A64%20Assembler.htm">one of my own</a>.
Furtunately MIPS and PowerPC disassemblers are easy to implement.

There could also `be` pivot for 8-bit CPU 8080/8085/Z80, 6502, HC08
or for bytecode of virtual machines: JVM, CLR, BEAM, EBC (EFI Byte Code),
which are also relatively easy to implement. Despite a lot of plugins it should remain small and concise,
the only feature it should have for release is an annotation tool for coloring and commenting the dump.
This should be a hacker's notepad, patch tool and reverse engineering viewer.

The `be` default used for ASM view is 64-bit mode EM64T 16-byte
instruction pipeline and for HEX view is 192-bit width byte stream.
For the sake of smooth experience in 64-bit mode it requires 120 column terminal.

<img width="796" alt="ASM" src="https://user-images.githubusercontent.com/144776/172079654-9380b592-ff6d-4f51-b0b4-9837ddc376ab.png">
<img width="796" alt="HEX" src="https://user-images.githubusercontent.com/144776/172079707-34d042b1-c2a8-49ed-88b0-3e21d7569106.png">

Credits
-------

* Kevin Pors, <a href="https://github.com/krpors/hx">hx</a>
* Justin Sherman, <a href="https://github.com/jsherman212/armadillo">armadillo</a>
* NASM Authors, <a href="https://github.com/netwide-assembler/nasm">nasm</a>
