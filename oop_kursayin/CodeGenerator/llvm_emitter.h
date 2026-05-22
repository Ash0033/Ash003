#pragma once
#include "../Parser/ast.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
 
// Emits LLVM IR (.ll) as text.
// Feed the result to:  llvm-as out.ll -o out.bc
//                      llc -march=riscv32 out.bc -o out.s
class LLVMEmitter {
public:
    std::string emit(const Program& prog);
 
private:
    std::ostringstream out_;
    int                tmp_ = 0;   // SSA register counter
    int                lbl_ = 0;   // label counter
 
    // name → SSA register string ("%name.N")
    std::unordered_map<std::string, std::string> locals_;
 
    std::string new_tmp();
    std::string new_label(const std::string& hint);
 
    static std::string llvm_type(const AshType& t);
    static std::string llvm_bool_to_i1(const std::string& reg, std::ostringstream& o);
 
    void emit_fn(const FnDecl& fn);
    void emit_stmt(const Stmt& s);
    // Returns the SSA register holding the result
    std::string emit_expr(const Expr& e);
};
 