#pragma once

#include <vector>
#include <string>
#include "../Parser/parser.h"

struct Instruction {
    OperationCode operator_code;
    int32_t destination_register = 0;
    int32_t source_register_1 = 0;
    int32_t source_register_2 = 0;
    double immediate_value = 0.0;
    std::string operation_name;
};

class CodeGenerator {
private:
    std::vector<Instruction> instructions_;
    int32_t next_register_index_ = 0;

    int32_t allocate_register();
    int32_t visit(std::unique_ptr<struct BaseNode> ast_node);
public:
    std::vector<Instruction> generate(std::unique_ptr<BaseNode> ast_node, int32_t& output_register);
};

#include "c_gen.cpp"