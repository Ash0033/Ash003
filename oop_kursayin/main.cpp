#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
 
#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "Sema/sema.h"
#include "CodeGenerator/llvm_emitter.h"
#include "Loader/loader.h"
 
static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open file: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
 
static void write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Cannot write file: " + path);
    f << content;
}
 
static void usage(const char* argv0) {
    std::cerr
        << "Usage:\n"
        << "  " << argv0 << " <source.ash>          -- compile + run\n"
        << "  " << argv0 << " <source.ash> --emit-ll -- print LLVM IR and exit\n";
}
 
int main(int argc, char** argv) {
    if (argc < 2) { usage(argv[0]); return 1; }
 
    bool emit_ll = (argc >= 3 && std::string(argv[2]) == "--emit-ll");
 
    try {
        // ── 1. Read source ────────────────────────────────────────────────────
        std::string source = read_file(argv[1]);
 
        // ── 2. Lex ────────────────────────────────────────────────────────────
        Lexer lexer(source);
        auto  tokens = lexer.tokenize();
 
        // ── 3. Parse ──────────────────────────────────────────────────────────
        Parser  parser(std::move(tokens));
        Program prog = parser.parse();
 
        // ── 4. Semantic analysis ──────────────────────────────────────────────
        Sema sema;
        sema.check(prog);
 
        // ── 5. Emit LLVM IR ───────────────────────────────────────────────────
        LLVMEmitter emitter;
        std::string ir = emitter.emit(prog);
 
        if (emit_ll) {
            std::cout << ir;
            return 0;
        }
 
        // ── 6. Write IR to temp file ──────────────────────────────────────────
        const std::string ll_path  = "/tmp/ash_out.ll";
        const std::string bc_path  = "/tmp/ash_out.bc";
        const std::string obj_path = "/tmp/ash_out.o";
        const std::string bin_path = "/tmp/ash_out.bin";
 
        write_file(ll_path, ir);
 
        // ── 7. Assemble IR → bitcode ──────────────────────────────────────────
        if (std::system(("llvm-as " + ll_path + " -o " + bc_path).c_str()) != 0)
            throw std::runtime_error("llvm-as failed");
 
        // ── 8. Compile bitcode → RISC-V object ───────────────────────────────
        if (std::system(("llc -march=riscv32 -mattr=+m -filetype=obj "
                         + bc_path + " -o " + obj_path).c_str()) != 0)
            throw std::runtime_error("llc failed");
 
        // ── 9. Link → flat binary ─────────────────────────────────────────────
        if (std::system(("ld.lld -m elf32lriscv --oformat=binary -e main "
                         + obj_path + " -o " + bin_path).c_str()) != 0)
            throw std::runtime_error("ld.lld failed");
 
        // ── 10. Load and execute ──────────────────────────────────────────────
        Loader loader;
        int32_t result = loader.load_and_run(bin_path);
        std::cout << "Program exited with: " << result << "\n";
 
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}