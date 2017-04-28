#pragma once

#include <string>

class SymbolTable {
    public:
    void* symbolTable;
    
    SymbolTable();
    ~SymbolTable();

    void AddVariable(std::string name, double& ref);
};

class Expression {
    public:
    void* expression;

    Expression();
    ~Expression();

    void RegisterSymbolTable(SymbolTable& symbolTable);
    double Value();
};

class Parser {
    public:
    void* parser;

    Parser();
    ~Parser();

    bool Compile(const std::string& exprStr, Expression& expr);
};