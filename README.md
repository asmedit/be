Binary Editor
=============

The `be` hacker editor aims to be a contemporary scalable `hiew` replacement.
Basically it is the `hx` hex editor with disassembly mode.
The `nasm` is used as disassembly library with support of 16, 32, 64
segment sizes and all Intel extensions:
MMX, SSE, SSE2, SSE3, SSE4.1 SSE4.2, SSE5, KNI, AVX, AVX-2, AVX-512,
AES, AMX, TSX, MPX, FMA, VNNI, GFNI, SGX, CET, PKU.

In the future `be` should have `macho`, `elf`, and `pe` containers
support along with RISC-V and AArch64 disassemblers.
Unfortunately I couldn't find appropriate ARM disassembler codebase to derive
so maybe I should finish <a href="https://tonpa.guru/stream/2019/2019-06-09%20A64%20Assembler.htm">one of my own</a>.


<img width="1250" alt="Screenshot 2022-06-04 at 09 23 11" src="https://user-images.githubusercontent.com/144776/171987389-7cafd20d-cdd7-4f39-9c1e-20c40915cc13.png">

<img width="1250" alt="Screenshot 2022-06-04 at 09 23 00" src="https://user-images.githubusercontent.com/144776/171987401-a0b57cd3-2c00-4a62-8df6-741f871c5087.png">


Credits
-------

* Kevin Pors, <a href="https://github.com/krpors/hx">hx</a>
* NASM Authors, <a href="https://github.com/netwide-assembler/nasm">nasm</a>
