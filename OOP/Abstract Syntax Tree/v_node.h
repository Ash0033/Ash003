#pragma once

#include <string>
#include "ast_node.h"

class VariableNode : public BaseNode {
private:
    std::string variable_name_;

public:
    explicit VariableNode(const std::string& variable_name);

    std::string get_variable_name() const;

    double evaluate(const std::map<std::string, double>& variables) const override;
};

#include "v_node.cpp"