#pragma once

#include "binop_node.h"

inline BinaryOperatorNode::BinaryOperatorNode(char operator_symbol,
                                              std::unique_ptr<BaseNode> left_operand,
                                              std::unique_ptr<BaseNode> right_operand)
    : operator_symbol_(operator_symbol),
      left_operand_(std::move(left_operand)),
      right_operand_(std::move(right_operand)) {}

inline char BinaryOperatorNode::get_operator_symbol() const {
    return operator_symbol_;
}

inline std::unique_ptr<BaseNode> BinaryOperatorNode::take_left_operand() {
    return std::move(left_operand_);
}

inline std::unique_ptr<BaseNode> BinaryOperatorNode::take_right_operand() {
    return std::move(right_operand_);
}

inline double BinaryOperatorNode::evaluate(const std::map<std::string, double>& variable_values) const {
    const double left_value = left_operand_->evaluate(variable_values);
    const double right_value = right_operand_->evaluate(variable_values);

    switch (operator_symbol_) {
        case '+':
            return left_value + right_value;
        case '-':
            return left_value - right_value;
        case '*':
            return left_value * right_value;
        case '/':
            if (right_value == 0.0) {
                throw std::runtime_error("Division by zero");
            }
            return left_value / right_value;
        default:
            throw std::runtime_error(std::string("Unknown operator: ") + operator_symbol_);
    }
}