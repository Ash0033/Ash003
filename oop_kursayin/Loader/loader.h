#pragma once
#include <string>
#include <cstdint>
 
// Loads a flat binary (raw RISC-V machine code) into executable memory
// and calls the entry symbol as  int32_t entry().
//
// Typical usage after the compiler produces an ELF:
//   1.  llvm-as out.ll -o out.bc
//   2.  llc -march=riscv32 -filetype=obj out.bc -o out.o
//   3.  ld.lld -m elf32lriscv --oformat=binary -e main out.o -o out.bin
//   4.  Loader loader; int result = loader.load_and_run("out.bin");
 
class Loader {
public:
    // Loads `path` (flat binary), mmaps it as RX, jumps to offset 0.
    // Returns the integer value returned by the entry function.
    int32_t load_and_run(const std::string& path);
};
 