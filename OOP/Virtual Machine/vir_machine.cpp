#pragma once

#include "vir_machine.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <unordered_map>

inline double VirtualMachine::execute(const std::vector<Instruction>& instructions,
                               int32_t result_register,
                               const std::map<std::string, double>& variable_values)
{
    std::unordered_map<int32_t, double> registers;  // регистровый файл

    std::cout << "\n  #   INSTRUCTION\n";
    std::cout << "  " << std::string(50, '-') << '\n';

    for (std::size_t index = 0; index < instructions.size(); ++index) {
        const auto& instruction = instructions[index];

        std::string instruction_text;

        switch (instruction.operator_code) {
            case OperationCode::MOVE: {
                registers[instruction.destination_register] = instruction.immediate_value;

                instruction_text = "MOVE  R" + std::to_string(instruction.destination_register)
                                 + ", " + std::to_string(instruction.immediate_value);
                break;
            }

            case OperationCode::LOAD: {
                auto it = variable_values.find(instruction.operation_name);
                if (it == variable_values.end())
                    throw std::runtime_error("Variable '" + instruction.operation_name + "' not found");

                registers[instruction.destination_register] = it->second;

                instruction_text = "LOAD  R" + std::to_string(instruction.destination_register)
                                 + ", " + instruction.operation_name
                                 + "  ; = " + std::to_string(it->second);
                break;
            }

            case OperationCode::ADD:
            case OperationCode::SUB:
            case OperationCode::MUL:
            case OperationCode::DIV: {
                double left_value  = registers.at(instruction.source_register_1);
                double right_value = registers.at(instruction.source_register_2);

                double result_value = 0.0;

                if (instruction.operator_code == OperationCode::ADD) {
                    result_value = left_value + right_value;
                }
                if (instruction.operator_code == OperationCode::SUB) {
                    result_value = left_value - right_value;
                }
                if (instruction.operator_code == OperationCode::MUL) {
                    result_value = left_value * right_value;
                }
                if (instruction.operator_code == OperationCode::DIV) {
                    if (right_value == 0.0)
                        throw std::runtime_error("Division by zero");
                    result_value = left_value / right_value;
                }

                registers[instruction.destination_register] = result_value;

                instruction_text = operator_code_name(instruction.operator_code)
                                 + "   R" + std::to_string(instruction.destination_register)
                                 + ", R" + std::to_string(instruction.source_register_1)
                                 + ", R" + std::to_string(instruction.source_register_2)
                                 + "  ; = " + std::to_string(result_value);
                break;
            }
        }

        std::cout << "  " << std::setw(2) << index << "  " << instruction_text << '\n';
    }

    return registers.at(result_register);
}