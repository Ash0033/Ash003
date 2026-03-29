#pragma once

#include "num_node.h"
#include <iostream>

inline NumberNode::NumberNode(double value) : value_ {value} {}

inline double NumberNode::get_value() const {
        return value_;
}

inline double NumberNode::evaluate(const std::map<std::string, double>&) const {
        return value_;
}