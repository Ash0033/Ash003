#pragma once
#include "value.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <optional>
 
// One stack frame
struct Frame {
    std::string                             fn_name;
    std::unordered_map<std::string, AshValue> locals;
    int                                     call_line = 0; // line where this fn was called
};
 
// A single entry in the execution log
struct ExecEvent {
    int         line;
    std::string description; // e.g. "let x = 42", "return 7"
};
 
struct DebugState {
    // --- breakpoints: set of source lines ---
    std::unordered_set<int> breakpoints;
 
    // --- watchpoints: variable names to watch for changes ---
    std::unordered_set<std::string> watchpoints;
 
    // --- call stack ---
    std::vector<Frame> call_stack;
 
    // --- step mode ---
    enum class StepMode { Run, Step, StepOver, StepOut };
    StepMode mode = StepMode::Run;
 
    // --- execution log (last N events) ---
    std::vector<ExecEvent> log;
    static constexpr std::size_t LOG_MAX = 200;
 
    // --- last-hit breakpoint line ---
    std::optional<int> last_break_line;
 
    // --- paused? ---
    bool paused = false;
 
    // ── Helpers ──────────────────────────────────────────────────────────────
 
    void push_frame(const std::string& name, int call_line) {
        call_stack.push_back({name, {}, call_line});
    }
 
    void pop_frame() {
        if (!call_stack.empty()) call_stack.pop_back();
    }
 
    Frame& top() { return call_stack.back(); }
 
    void set_local(const std::string& name, AshValue val) {
        bool changed = true;
        auto it = top().locals.find(name);
        if (it != top().locals.end() && it->second.to_string() == val.to_string())
            changed = false;
 
        top().locals[name] = val;
 
        if (changed && watchpoints.count(name))
            log_event(-1, "WATCH: " + name + " = " + val.to_string()
                      + " (" + val.type_name() + ")");
    }
 
    std::optional<AshValue> get_local(const std::string& name) const {
        for (int i = (int)call_stack.size()-1; i >= 0; --i) {
            auto it = call_stack[i].locals.find(name);
            if (it != call_stack[i].locals.end()) return it->second;
        }
        return std::nullopt;
    }
 
    void log_event(int line, const std::string& desc) {
        if (log.size() >= LOG_MAX) log.erase(log.begin());
        log.push_back({line, desc});
    }
 
    bool should_pause(int line) {
        if (mode == StepMode::Step) return true;
        if (breakpoints.count(line)) { last_break_line = line; return true; }
        return false;
    }
};
 