#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <sstream>

#include "Type/token.h"
#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "Code Generator/c_gen.h"
#include "Virtual Machine/vir_machine.h"
#include "helper.h"

struct VariableRange {
    double start_;
    double end_;
    double step_;

    VariableRange(double value)
        : start_(value), end_(value), step_(1.0) {}

    VariableRange(double start, double end, double step = 1.0)
        : start_(start), end_(end), step_(step) {}
};

std::map<std::string, VariableRange> parse_variable_ranges(const std::string& variables_line)
{
    std::map<std::string, VariableRange> variable_ranges;
    std::istringstream input_stream(variables_line);
    std::string variable_assignment;

    while (input_stream >> variable_assignment) {
        std::size_t equal_position = variable_assignment.find('=');
        if (equal_position == std::string::npos) {
            continue;
        }

        std::string variable_name = variable_assignment.substr(0, equal_position);
        std::string value_text = variable_assignment.substr(equal_position + 1);

        if (variable_name.empty() || value_text.empty()) {
            continue;
        }

        std::size_t first_colon_position = value_text.find(':');
        if (first_colon_position == std::string::npos) {
            variable_ranges.emplace(variable_name, VariableRange(std::stod(value_text)));
        } else {
            double start = std::stod(value_text.substr(0, first_colon_position));
            std::string rest = value_text.substr(first_colon_position + 1);

            std::size_t second_colon_pos = rest.find(':');
            if (second_colon_pos == std::string::npos) {
                variable_ranges.emplace(variable_name, VariableRange(start, std::stod(rest)));
            } else {
                double end = std::stod(rest.substr(0, second_colon_pos));
                double step = std::stod(rest.substr(second_colon_pos + 1));
                variable_ranges.emplace(variable_name, VariableRange(start, end, step));
            }
        }
    }

    return variable_ranges;
}

double evaluate_expression(const std::string& expression_text,
                           const std::map<std::string, double>& variable_values)
{
    print_separator();
    std::cout << "Expression: " << expression_text << '\n';

    std::cout << "Variables:  ";
    for (const auto& [variable_name, value] : variable_values) {
        std::cout << variable_name << "=" << value << "  ";
    }
    std::cout << '\n';

    print_separator();

    std::cout << "\nLexer (Finite State Machine)\n";
    Lexer lexer(expression_text);
    std::vector<Token> tokens = lexer.tokenize();
    print_tokens(tokens);

    std::cout << "\nParser -> AST\n";
    Parser parser(tokens);
    std::unique_ptr<BaseNode> ast = parser.parse();

    std::cout << "\nCode Generation\n";
    CodeGenerator code_generator{};
    int32_t result_register = 0;
    std::vector<Instruction> instructions = code_generator.generate(std::move(ast), result_register);

    std::cout << "  " << instructions.size() << " instructions generated, "
              << "result in R" << result_register << "\n";

    std::cout << "\nVM Execution";
    VirtualMachine virtual_machine;
    double result = virtual_machine.execute(instructions, result_register, variable_values);

    std::cout << "\nResult: " << result << '\n';
    print_separator();

    return result;
}

void evaluate_expression_range(const std::string& expression_text,
                               const std::map<std::string, VariableRange>& variable_ranges)
{
    Lexer lexer(expression_text);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    std::unique_ptr<BaseNode> ast = parser.parse();

    CodeGenerator code_generator;
    int32_t result_register = 0;
    std::vector<Instruction> instructions = code_generator.generate(std::move(ast), result_register);

    std::vector<std::pair<std::string, VariableRange>> variables_list(
        variable_ranges.begin(), variable_ranges.end()
    );

    std::map<std::string, double> current_variable_values;

    std::cout << "\nExpression: " << expression_text << '\n';
    std::cout << std::string(50, '-') << '\n';

    std::function<void(int32_t)> enumerate_values = [&](int32_t index) {
        if (index == static_cast<int32_t>(variables_list.size())) {
            VirtualMachine virtual_machine;
            double result = virtual_machine.execute(instructions, result_register, current_variable_values);

            for (const auto& [variable_name, value] : current_variable_values) {
                std::cout << variable_name << "=" << std::setw(6) << value << "  ";
            }
            std::cout << "->  " << result << '\n';
            return;
        }

        const auto& [variable_name, range] = variables_list[index];
        for (double value = range.start_; value <= range.end_ + 1e-9; value += range.step_) {
            current_variable_values[variable_name] = value;
            enumerate_values(index + 1);
        }
    };

    enumerate_values(0);
    std::cout << std::string(50, '-') << '\n';
}

int main()
{
    std::cout << "\n\n=== INTERACTIVE MODE ===\n";
    std::cout << "Type 'exit' to quit\n\n";

    while (true) {
        std::string expression_text;
        std::cout << "Expression: ";
        std::getline(std::cin, expression_text);

        if (expression_text == "exit" || expression_text == "quit" || expression_text == "выход") {
            break;
        }

        std::string variables_line;
        std::cout << "Variables (e.g. a=3 b=0:10 c=0:5:0.5, or Enter): ";
        std::getline(std::cin, variables_line);

        try {
            std::map<std::string, VariableRange> variable_ranges = parse_variable_ranges(variables_line);
            evaluate_expression_range(expression_text, variable_ranges);
        } catch (const std::exception& exception) {
            std::cerr << "  ERROR: " << exception.what() << "\n";
        }
    }

    return 0;
}