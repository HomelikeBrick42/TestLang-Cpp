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
    AstStatement* ParseStatement();
    AstDeclaration* ParseDeclaration(AstName* name);

    AstExpression* ParseExpression();
    AstExpression* ParsePrimaryExpression();
    AstExpression* ParseBinaryExpression(u64 parentPrecedence);

    AstType* ParseType();
private:
    Token NextToken();
    Token ExpectToken(TokenKind kind);
public:
    Lexer Lexer;
    Array<String> Errors;
private:
    Token Current;
    AstFile* ParentFile;
    AstScope* ParentScope;
};
