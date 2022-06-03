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

<img width="816" alt="Screenshot 2022-06-02 at 12 31 59" src="https://user-images.githubusercontent.com/144776/171601023-92d28a16-cf44-43e1-b888-3812e981cf90.png">

<img width="817" alt="Screenshot 2022-06-04 at 02 27 15" src="https://user-images.githubusercontent.com/144776/171966478-a1876b9b-2574-4ff4-a184-b38e7f9206e2.png">

Credits
-------

* Kevin Pors, <a href="https://github.com/krpors/hx">hx</a>
* NASM Authors, <a href="https://github.com/netwide-assembler/nasm">nasm</a>
