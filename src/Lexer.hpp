#pragma once

#include "Defines.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Token.hpp"

class Lexer {
public:
    Lexer(const String& source);
    ~Lexer();

    Token NextToken();
private:
    String Source;
    u64 Position;
    u64 Line;
    u64 Column;
public:
    Array<String> Errors;
};
