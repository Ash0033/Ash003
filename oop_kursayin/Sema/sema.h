#pragma once
#include "../Parser/ast.h"
#include <unordered_map>
#include <string>
#include <vector>
 
// One scope level: name → type
using Scope = std::unordered_map<std::string, AshType>;
 
class Sema {
public:
    // Entry point — annotates all types in place, throws on error
    void check(Program& prog);
 
private:
    // Stack of scopes (innermost last)
    std::vector<Scope> scopes_;
 
    // Return type of the function being checked
    AshType current_ret_type_;
 
    void push_scope();
    void pop_scope();
    void define(const std::string& name, AshType t, int line);
    const AshType& lookup(const std::string& name, int line) const;
 
    void   check_fn(FnDecl& fn);
    void   check_stmt(Stmt& s);
    AshType check_expr(Expr& e);
 
    static std::string type_str(const AshType& t);
};
 