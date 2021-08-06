#pragma once

#include "Defines.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Lexer.hpp"
#include "Ast.hpp"

class Parser {
public:
    Parser(const String& source);
    ~Parser();
public:
    AstExpression* ParseExpression();
private:
    AstExpression* ParsePrimaryExpression();
    AstExpression* ParseBinaryExpression(u64 parentPrecedence);
private:
    Token NextToken();
public:
    Lexer Lexer;
    Array<String> Errors;
private:
    Token Current;
    AstFile* ParentFile;
    AstScope* ParentScope;
};
