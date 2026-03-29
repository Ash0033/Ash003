#pragma once

#include <map>
#include <string>
#include "ast_node.h"

class NumberNode : public BaseNode {
private:
    double value_;
public:
    explicit NumberNode(double value);

    double get_value() const;

    double evaluate(const std::map<std::string, double>&) const override;
};

#include "num_node.cpp"