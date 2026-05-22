#pragma once
#include "interpreter.h"
#include "../Parser/ast.h"
#include <string>
 
// Interactive debugger REPL.
//
// Commands:
//   run [fn] [args...]  — start/restart execution
//   b  <line>           — set breakpoint
//   db <line>           — delete breakpoint
//   bl                  — list breakpoints
//   w  <name>           — watch variable
//   dw <name>           — delete watch
//   s                   — step (into)
//   n                   — next (step over — same as step for interpreter)
//   c                   — continue
//   bt                  — backtrace
//   l                   — list locals in current frame
//   p  <name>           — print variable value
//   log [N]             — show last N execution events (default 20)
//   src                 — show source with breakpoint markers
//   q / quit            — quit
//   h / help            — help
 
class Debugger {
public:
    Debugger(const Program& prog, const std::string& source);
    void run_repl();
 
private:
    const Program&     prog_;
    std::string        source_;       // full source text, for src command
    std::vector<std::string> lines_;  // source split by line
 
    DebugState         state_;
 
    // Called by the interpreter when execution pauses
    void on_pause(DebugState& st, int line, const std::string& desc);
 
    // True while the pause callback is waiting for user input
    bool in_pause_ = false;
 
    // Command dispatch
    bool handle_command(const std::string& line,
                        bool& step_requested,
                        bool& continue_requested);
 
    // Helpers
    void print_frame_locals() const;
    void print_backtrace()    const;
    void print_source(int highlight_line = -1) const;
    void print_log(int n) const;
    void print_help() const;
 
    static std::vector<std::string> split(const std::string& s);
};
 