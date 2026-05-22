#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
 
#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "Sema/sema.h"
#include "Debugger/debugger.h"
 
static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path);
    std::ostringstream ss; ss << f.rdbuf();
    return ss.str();
}
 
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ash_debug <source.ash>\n";
        return 1;
    }
    try {
        std::string source = read_file(argv[1]);
 
        Lexer   lexer(source);
        auto    tokens = lexer.tokenize();
 
        Parser  parser(std::move(tokens));
        Program prog = parser.parse();
 
        Sema    sema;
        sema.check(prog);
 
        Debugger dbg(prog, source);
        dbg.run_repl();
 
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
 