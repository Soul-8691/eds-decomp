---
title: Toolchain
type: tool
status: solid
confidence: high
sources: [rom-analysis]
updated: 2026-10-24
---
# Toolchain

The build runs natively in WSL or Linux, laid out like pret's decomps (pokefirered): a `Makefile` driving agbcc and the system binutils. There is no container (Docker was dropped on 2026-10-24).

| Purpose | Tool | Notes |
|---|---|---|
| Build | `make -j$(nproc) compare` | Run in WSL. `tools/dr <cmd>` runs a command from the repo root (and sources the optional extras' `~/.eds-tools/env.sh`). |
| Compiler | [[agbcc]] (`tools/agbcc/bin/agbcc`, plus `old_agbcc` and `agbcc_arm`) | Built in a pret/agbcc checkout at `../agbcc`; the Makefile installs it into `tools/agbcc` (git-ignored) with its `install.sh`. Flags: [[compiler-flags]]. |
| Runtime libs | agbcc's `libgcc.a` / `libc.a` | Linked as-is; they match the ROM byte for byte. The member order is pinned in `tools/mkld.py`. |
| Assembler/linker | distro `binutils-arm-none-eabi` | `as -mcpu=arm7tdmi -mthumb-interwork` |
| Preprocessor | host `cpp` with `-nostdinc -undef` | |
| Optional extras | `tools/setup_native.sh` → `~/.eds-tools` | venv with capstone/pyelftools, [[decomp-permuter]], [[m2c]], [[objdiff]] CLI. Not needed for `make`. |
| Disassembler | `tools/disasm.py` (own, Python + capstone for ARM) | Recursive descent. Detects agbcc far jumps and jump tables, and fills gaps with prologue-first heuristics. Output is agbcc "divided" syntax. |
| Unit checker | `tools/check.py <unit> [--diff F]` | Compiles one unit, links it alone at its ROM address, and compares per function. Safe to run concurrently. |
| Skeletons | `tools/mkunit.py <unit>` | All-`INCLUDE_ASM` C file for a unit. |
| Linker script | `tools/mkld.py` (generated from `units.txt`) | Libraries sit at `@libs`; `@rodata` places a unit's rodata. |
| Auto symbols | `tools/autosyms.py` | Gives any undefined `name_XXXXXXXX` symbol the address in its name. |
| ROM diff | `tools/romdiff.py` | Runs automatically when `make compare` fails. |
| Card data | `tools/extract_cards.py` | stdlib only; `--verify` cross-checks known cards. |
| ROM map check | `tools/verify_rom_map.py` | Checks the [[rom-map]] boundaries. |

Decisions made on 2026-09-29: the compiler is agbcc, confirmed by the libgcc/libc byte match; everything runs in Docker (replaced on 2026-10-24 by the native WSL build); the project uses its own disassembler instead of gbadisasm or luvdis; and C units keep one asm file per function, included with `INCLUDE_ASM` (the splat-style workflow). See the [[log]].
