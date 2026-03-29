#pragma once

#include "v_node.h"
#include <iostream>

inline VariableNode::VariableNode(const std::string& variable_name) : variable_name_(variable_name) {}

inline std::string VariableNode::get_variable_name() const {
    return variable_name_;
}

inline double VariableNode::evaluate(const std::map<std::string, double>& variables) const {
    auto iterator = variables.find(variable_name_);
    if (iterator == variables.end()) {
        throw std::runtime_error("Variable '" + variable_name_ + "' not found");
    }

    return iterator->second;
}