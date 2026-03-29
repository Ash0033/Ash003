#pragma once

#include "c_gen.h"
#include <stdexcept>

inline int32_t CodeGenerator::allocate_register() {
    return next_register_index_++;
}

inline std::vector<Instruction> CodeGenerator::generate(std::unique_ptr<BaseNode> ast_node, int32_t& result_register) {
    instructions_.clear();
    next_register_index_ = 0;
    result_register = visit(std::move(ast_node));
    return instructions_;
}

inline int32_t CodeGenerator::visit(std::unique_ptr<BaseNode> ast_node) {
    if (const auto* number_node = dynamic_cast<const NumberNode*>(ast_node.get())) {
        int32_t destination_register = allocate_register();
        instructions_.push_back({
            OperationCode::MOVE,
            destination_register,
            0,
            0,
            number_node->get_value()
        });
        return destination_register;
    }

    if (const auto* variable_node = dynamic_cast<const VariableNode*>(ast_node.get())) {
        int32_t destination_register = allocate_register();
        instructions_.push_back({
            OperationCode::LOAD,
            destination_register,
            0,
            0,
            0.0,
            variable_node->get_variable_name()
        });
        return destination_register;
    }

    if (auto* binary_node = dynamic_cast<BinaryOperatorNode*>(ast_node.get())) {
        int32_t left_register = visit(binary_node->take_left_operand());
        int32_t right_register = visit(binary_node->take_right_operand());
        int32_t destination_register = allocate_register();

        OperationCode operation_code;
        switch (binary_node->get_operator_symbol()) {
            case '+': operation_code = OperationCode::ADD; break;
            case '-': operation_code = OperationCode::SUB; break;
            case '*': operation_code = OperationCode::MUL; break;
            case '/': operation_code = OperationCode::DIV; break;
            default:
                throw std::runtime_error(
                    std::string("Unknown operator: ") + binary_node->get_operator_symbol()
                );
        }

        instructions_.push_back({
            operation_code,
            destination_register,
            left_register,
            right_register
        });
        return destination_register;
    }

    throw std::runtime_error("Unknown AST node type");
}