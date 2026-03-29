#pragma once

#include <vector>
#include <map>
#include <string>
#include "../Code Generator/c_gen.h"

class VirtualMachine {
public:
    double execute(const std::vector<Instruction>& instructions,
                   int32_t result_register,
                   const std::map<std::string, double>& variable_values);
};

#include "vir_machine.cpp"