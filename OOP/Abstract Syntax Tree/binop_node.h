#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include "ast_node.h"

class BinaryOperatorNode : public BaseNode {
private:
    char operator_symbol_;
    std::unique_ptr<BaseNode> left_operand_;
    std::unique_ptr<BaseNode> right_operand_;
public:
    explicit BinaryOperatorNode(char operator_symbol,
                                std::unique_ptr<BaseNode> left_operand,
                                std::unique_ptr<BaseNode> right_operand);

    char get_operator_symbol() const;
    std::unique_ptr<BaseNode> take_left_operand();
    std::unique_ptr<BaseNode> take_right_operand();

    double evaluate(const std::map<std::string, double>& variable_values) const override;
};

#include "binop_node.cpp"