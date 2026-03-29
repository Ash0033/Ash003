#pragma once

#include <map>
#include <string>

struct BaseNode {
    virtual ~BaseNode() = default;
    virtual double evaluate(const std::map<std::string, double>& variables) const = 0;
};