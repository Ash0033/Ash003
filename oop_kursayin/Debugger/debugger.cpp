#include "debugger.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
 
// ── Ctor ─────────────────────────────────────────────────────────────────────
 
Debugger::Debugger(const Program& prog, const std::string& source)
    : prog_(prog), source_(source)
{
    std::istringstream ss(source);
    std::string line;
    while (std::getline(ss, line)) lines_.push_back(line);
}
 
// ── Helpers ───────────────────────────────────────────────────────────────────
 
std::vector<std::string> Debugger::split(const std::string& s) {
    std::vector<std::string> parts;
    std::istringstream ss(s);
    std::string w;
    while (ss >> w) parts.push_back(w);
    return parts;
}
 
void Debugger::print_frame_locals() const {
    if (state_.call_stack.empty()) {
        std::cout << "(no active frame)\n";
        return;
    }
    auto& frame = state_.call_stack.back();
    std::cout << "  [frame: " << frame.fn_name << "]\n";
    for (auto& [k, v] : frame.locals)
        std::cout << "    " << k << " : " << v.type_name()
                  << " = " << v.to_string() << "\n";
}
 
void Debugger::print_backtrace() const {
    if (state_.call_stack.empty()) {
        std::cout << "(stack is empty)\n";
        return;
    }
    for (int i = (int)state_.call_stack.size()-1; i >= 0; --i) {
        auto& f = state_.call_stack[i];
        std::cout << "  #" << (state_.call_stack.size()-1-i)
                  << "  " << f.fn_name
                  << "  (called from line " << f.call_line << ")\n";
    }
}
 
void Debugger::print_source(int highlight) const {
    for (std::size_t i = 0; i < lines_.size(); ++i) {
        int ln = (int)i + 1;
        bool bp   = state_.breakpoints.count(ln) > 0;
        bool here = (ln == highlight);
        std::cout << (here ? "→ " : "  ")
                  << (bp  ? "●" : " ")
                  << " " << ln << "\t" << lines_[i] << "\n";
    }
}
 
void Debugger::print_log(int n) const {
    int start = std::max(0, (int)state_.log.size() - n);
    for (int i = start; i < (int)state_.log.size(); ++i) {
        auto& ev = state_.log[i];
        std::cout << "  [" << (ev.line > 0 ? std::to_string(ev.line) : "--") << "] "
                  << ev.description << "\n";
    }
}
 
void Debugger::print_help() const {
    std::cout <<
        "  run [fn] [args]  start/restart (default: main)\n"
        "  b  <line>        set breakpoint\n"
        "  db <line>        delete breakpoint\n"
        "  bl               list breakpoints\n"
        "  w  <var>         watch variable\n"
        "  dw <var>         delete watch\n"
        "  s                step (one statement)\n"
        "  c                continue\n"
        "  bt               backtrace\n"
        "  l                locals in current frame\n"
        "  p  <var>         print variable\n"
        "  log [N]          show last N events (default 20)\n"
        "  src              show source with breakpoints\n"
        "  q / quit         quit\n"
        "  h / help         this message\n";
}
 
// ── Pause callback (called by interpreter inside its thread) ─────────────────
 
void Debugger::on_pause(DebugState& /*st*/, int line, const std::string& desc) {
    // Print context
    std::cout << "\n── PAUSED at line " << line << ": " << desc << " ──\n";
    if (line > 0 && line <= (int)lines_.size()) {
        // Show 3 lines context
        int from = std::max(1, line-1);
        int to   = std::min((int)lines_.size(), line+1);
        for (int i = from; i <= to; ++i) {
            bool bp   = state_.breakpoints.count(i) > 0;
            std::cout << (i==line ? "→ " : "  ")
                      << (bp ? "●" : " ")
                      << " " << i << "\t" << lines_[i-1] << "\n";
        }
    }
    print_frame_locals();
 
    // Mini REPL while paused
    bool step_req = false, cont_req = false;
    while (!step_req && !cont_req) {
        std::cout << "(dbg paused) ";
        std::string input;
        if (!std::getline(std::cin, input)) { cont_req = true; break; }
        if (input.empty()) { step_req = true; break; } // Enter = step
        handle_command(input, step_req, cont_req);
    }
 
    if (step_req) state_.mode = DebugState::StepMode::Step;
    else          state_.mode = DebugState::StepMode::Run;
}
 
// ── Command handler ───────────────────────────────────────────────────────────
 
bool Debugger::handle_command(const std::string& raw,
                              bool& step_req, bool& cont_req) {
    auto parts = split(raw);
    if (parts.empty()) return true;
    const std::string& cmd = parts[0];
 
    // quit
    if (cmd == "q" || cmd == "quit") {
        std::cout << "Goodbye.\n";
        std::exit(0);
    }
 
    // help
    if (cmd == "h" || cmd == "help") {
        print_help();
        return true;
    }
 
    // breakpoint set
    if (cmd == "b" && parts.size() >= 2) {
        int ln = std::stoi(parts[1]);
        state_.breakpoints.insert(ln);
        std::cout << "  Breakpoint set at line " << ln << "\n";
        return true;
    }
 
    // breakpoint delete
    if (cmd == "db" && parts.size() >= 2) {
        int ln = std::stoi(parts[1]);
        state_.breakpoints.erase(ln);
        std::cout << "  Breakpoint removed from line " << ln << "\n";
        return true;
    }
 
    // breakpoint list
    if (cmd == "bl") {
        if (state_.breakpoints.empty()) { std::cout << "  No breakpoints.\n"; return true; }
        std::cout << "  Breakpoints:\n";
        std::vector<int> bps(state_.breakpoints.begin(), state_.breakpoints.end());
        std::sort(bps.begin(), bps.end());
        for (int l : bps)
            std::cout << "    line " << l
                      << (l <= (int)lines_.size() ? "  " + lines_[l-1] : "") << "\n";
        return true;
    }
 
    // watch
    if (cmd == "w" && parts.size() >= 2) {
        state_.watchpoints.insert(parts[1]);
        std::cout << "  Watching: " << parts[1] << "\n";
        return true;
    }
    if (cmd == "dw" && parts.size() >= 2) {
        state_.watchpoints.erase(parts[1]);
        std::cout << "  Watch removed: " << parts[1] << "\n";
        return true;
    }
 
    // step
    if (cmd == "s") { step_req = true; return true; }
 
    // continue
    if (cmd == "c") { cont_req = true; return true; }
 
    // backtrace
    if (cmd == "bt") { print_backtrace(); return true; }
 
    // locals
    if (cmd == "l") { print_frame_locals(); return true; }
 
    // print variable
    if (cmd == "p" && parts.size() >= 2) {
        auto v = state_.get_local(parts[1]);
        if (v) std::cout << "  " << parts[1] << " = " << v->to_string()
                         << "  (" << v->type_name() << ")\n";
        else   std::cout << "  '" << parts[1] << "' not found in scope\n";
        return true;
    }
 
    // log
    if (cmd == "log") {
        int n = (parts.size() >= 2) ? std::stoi(parts[1]) : 20;
        print_log(n);
        return true;
    }
 
    // source
    if (cmd == "src") {
        print_source(state_.last_break_line.value_or(-1));
        return true;
    }
 
    // run — handled at outer REPL level
    if (cmd == "run") return false;
 
    std::cout << "  Unknown command '" << cmd << "'. Type 'h' for help.\n";
    return true;
}
 
// ── Main REPL ────────────────────────────────────────────────────────────────
 
void Debugger::run_repl() {
    std::cout << "╔══════════════════════════════════╗\n"
              << "║   Ash Debugger  (type h for help) ║\n"
              << "╚══════════════════════════════════╝\n";
    std::cout << "Source has " << lines_.size() << " lines, "
              << prog_.functions.size() << " functions:\n";
    for (auto& fn : prog_.functions)
        std::cout << "  fn " << fn.name << "  (line " << fn.line << ")\n";
    std::cout << "\n";
 
    while (true) {
        std::cout << "(dbg) ";
        std::string input;
        if (!std::getline(std::cin, input)) break;
        if (input.empty()) continue;
 
        auto parts = split(input);
        bool dummy_step = false, dummy_cont = false;
 
        if (parts[0] == "run") {
            // Parse: run [fn_name] [arg1 arg2 ...]
            std::string fn_name = "main";
            if (parts.size() >= 2) fn_name = parts[1];
 
            std::vector<AshValue> args;
            for (std::size_t i = 2; i < parts.size(); ++i) {
                // Try int, then float
                try {
                    std::size_t pos;
                    int64_t iv = std::stoll(parts[i], &pos);
                    if (pos == parts[i].size()) { args.push_back(AshValue::make_int(iv)); continue; }
                } catch (...) {}
                try {
                    double fv = std::stod(parts[i]);
                    args.push_back(AshValue::make_float(fv));
                } catch (...) {
                    std::cout << "  Cannot parse argument: " << parts[i] << "\n";
                }
            }
 
            state_.call_stack.clear();
            state_.log.clear();
            state_.last_break_line.reset();
 
            auto cb = [this](DebugState& st, int line, const std::string& desc) {
                on_pause(st, line, desc);
            };
 
            std::cout << "Running " << fn_name << "...\n";
            try {
                Interpreter interp(prog_, state_, cb);
                AshValue result = interp.run(fn_name, std::move(args));
                std::cout << "\nProgram finished. Return value: "
                          << result.to_string() << "\n";
            } catch (const std::exception& ex) {
                std::cout << "\nRuntime error: " << ex.what() << "\n";
            }
            continue;
        }
 
        handle_command(input, dummy_step, dummy_cont);
    }
}
 