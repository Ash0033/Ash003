#pragma once
#include <string>
#include <variant>
#include <stdexcept>
#include "../Parser/ast.h"
 
// Runtime value: int | float | bool | function-pointer (fn name)
struct AshValue {
    enum class Kind { Int, Float, Bool, Fn };
    Kind kind = Kind::Int;
 
    int64_t     as_int   = 0;
    double      as_float = 0.0;
    bool        as_bool  = false;
    std::string as_fn;   // function name for first-class fn values
 
    static AshValue make_int  (int64_t v)         { AshValue r; r.kind=Kind::Int;   r.as_int=v;    return r; }
    static AshValue make_float(double  v)         { AshValue r; r.kind=Kind::Float; r.as_float=v;  return r; }
    static AshValue make_bool (bool    v)         { AshValue r; r.kind=Kind::Bool;  r.as_bool=v;   return r; }
    static AshValue make_fn   (std::string name)  { AshValue r; r.kind=Kind::Fn;    r.as_fn=name;  return r; }
 
    std::string to_string() const {
        switch (kind) {
            case Kind::Int:   return std::to_string(as_int);
            case Kind::Float: return std::to_string(as_float);
            case Kind::Bool:  return as_bool ? "true" : "false";
            case Kind::Fn:    return "<fn:" + as_fn + ">";
        }
        return "?";
    }
 
    std::string type_name() const {
        switch (kind) {
            case Kind::Int:   return "int";
            case Kind::Float: return "float";
            case Kind::Bool:  return "bool";
            case Kind::Fn:    return "fn";
        }
        return "?";
    }
};