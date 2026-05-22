#pragma once
#include "debug_state.h"
#include "../Parser/ast.h"
#include <functional>
#include <stdexcept>
#include <string>
 
// Thrown to implement return-statement unwinding
struct ReturnSignal {
    AshValue value;
};
 
// Called by the interpreter whenever it's about to execute a statement.
// The UI can pause, print, etc.
using PauseCallback = std::function<void(DebugState&, int line, const std::string& desc)>;
 
class Interpreter {
public:
    Interpreter(const Program& prog, DebugState& state, PauseCallback on_pause)
        : prog_(prog), state_(state), on_pause_(std::move(on_pause)) {}
 
    // Run function `name` with given arguments
    AshValue run(const std::string& fn_name,
                 std::vector<AshValue> args = {},
                 int call_line = 0);
 
private:
    const Program& prog_;
    DebugState&    state_;
    PauseCallback  on_pause_;
 
    const FnDecl& find_fn(const std::string& name, int line) const;
 
    void      exec_stmt (const Stmt& s);
    AshValue  eval_expr (const Expr& e);
 
    // Notify the debugger about a statement about to execute
    void notify(int line, const std::string& desc);
 
    // Arithmetic/logic helpers
    static AshValue apply_binop(const std::string& op,
                                const AshValue& l,
                                const AshValue& r,
                                int line);
};
 