#include "loader.h"
#include <stdexcept>
#include <cstdint>
#include <cstdio>
#include <cstring>
 
#ifdef __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
int32_t Loader::load_and_run(const std::string& path) {
    // ── 1. Read file ─────────────────────────────────────────────────────────
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0)
        throw std::runtime_error("Loader: cannot open '" + path + "'");
 
    struct stat st{};
    if (::fstat(fd, &st) < 0) { ::close(fd); throw std::runtime_error("Loader: fstat failed"); }
    std::size_t size = static_cast<std::size_t>(st.st_size);
    if (size == 0) { ::close(fd); throw std::runtime_error("Loader: empty binary"); }
 
    // ── 2. Allocate RWX page(s) ──────────────────────────────────────────────
    void* mem = ::mmap(nullptr, size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) { ::close(fd); throw std::runtime_error("Loader: mmap failed"); }
 
    // ── 3. Read binary into the mapping ──────────────────────────────────────
    {
        uint8_t* dst  = static_cast<uint8_t*>(mem);
        ssize_t  left = static_cast<ssize_t>(size);
        while (left > 0) {
            ssize_t n = ::read(fd, dst, static_cast<std::size_t>(left));
            if (n <= 0) {
                ::munmap(mem, size); ::close(fd);
                throw std::runtime_error("Loader: read failed");
            }
            dst  += n;
            left -= n;
        }
    }
    ::close(fd);
 
    // ── 4. Make executable (remove write, add exec) ───────────────────────────
    if (::mprotect(mem, size, PROT_READ | PROT_EXEC) != 0) {
        ::munmap(mem, size);
        throw std::runtime_error("Loader: mprotect failed");
    }
 
    // ── 5. Jump to offset 0 as  int32_t (*fn)() ──────────────────────────────
    //
    // NOTE: On a real RISC-V board this works natively.
    // On an x86 host you need qemu-riscv32 or a RISC-V VM.
    // The Loader interface is correct; execution is architecture-gated.
    using EntryFn = int32_t (*)();
    auto entry = reinterpret_cast<EntryFn>(mem);
    int32_t result = entry();
 
    ::munmap(mem, size);
    return result;
}
 
#else
// ── Stub for non-Linux hosts (Windows / macOS) ────────────────────────────────
int32_t Loader::load_and_run(const std::string&) {
    throw std::runtime_error(
        "Loader: native execution only supported on Linux. "
        "Run the binary with: qemu-riscv32 out.bin");
}
#endif
 