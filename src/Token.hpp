#pragma once

#include "Defines.hpp"
#include "String.hpp"

#define TOKEN_KINDS                                       \
    TOKEN_KIND(EndOfFile, "EndOfFile")                    \
    TOKEN_KIND_DATA(Error, "Error", String, ErrorMessage) \
    TOKEN_KIND_DATA(Identifier, "Name", String, Name)     \
    TOKEN_KIND_DATA(Integer, "Integer", u64, IntValue)    \
    TOKEN_KIND_DATA(Float, "Float", f64, FloatValue)      \
                                                          \
    TOKEN_KIND(LParen, "(")                               \
    TOKEN_KIND(RParen, ")")                               \
    TOKEN_KIND(LBrace, "{")                               \
    TOKEN_KIND(RBrace, "}")                               \
    TOKEN_KIND(LBracket, "[")                             \
    TOKEN_KIND(RBracket, "]")                             \
    TOKEN_KIND(Colon, ":")                                \
    TOKEN_KIND(Semicolon, ";")                            \
    TOKEN_KIND(Comma, ",")                                \
                                                          \
    TOKEN_KIND(Plus, "+")                                 \
    TOKEN_KIND(Minus, "-")                                \
    TOKEN_KIND(Asterisk, "*")                             \
    TOKEN_KIND(Slash, "/")                                \
    TOKEN_KIND(Percent, "%")                              \
    TOKEN_KIND(Equals, "=")                               \
    TOKEN_KIND(ExclamationMark, "!")                      \
    TOKEN_KIND(LessThan, "<")                             \
    TOKEN_KIND(GreaterThan, ">")                          \
                                                          \
    TOKEN_KIND(PlusEquals, "+=")                          \
    TOKEN_KIND(MinusEquals, "-=")                         \
    TOKEN_KIND(AsteriskEquals, "*=")                      \
    TOKEN_KIND(SlashEquals, "/=")                         \
    TOKEN_KIND(PercentEquals, "%=")                       \
    TOKEN_KIND(EqualsEquals, "==")                        \
    TOKEN_KIND(ExclamationMarkEquals, "!=")               \
    TOKEN_KIND(LessThanEquals, "<=")                      \
    TOKEN_KIND(GreaterThanEquals, ">=")

enum struct TokenKind : u8 {
#define TOKEN_KIND(name, str)                          name,
#define TOKEN_KIND_DATA(name, str, dataType, dataName) name,
    TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_DATA
};

inline static String GetTokenKindName(TokenKind kind) {
    switch (kind) {
#define TOKEN_KIND(name, str) \
    case TokenKind::name:     \
        return str;
#define TOKEN_KIND_DATA(name, str, dataType, dataName) TOKEN_KIND(name, str)
        TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_DATA
    }

    Error("Unknown token kind!");
}

struct Token {
    TokenKind Kind;
    u64 Position;
    u64 Line;
    u64 Column;
    u64 Length;

    union {
#define TOKEN_KIND(name, str)
#define TOKEN_KIND_DATA(name, str, dataType, dataName) dataType dataName;
        TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_DATA
    } Data;
};

#define TOKEN_KIND(name, str)                               \
    inline static bool Token_Is##name(const Token& token) { \
        return token.Kind == TokenKind::name;               \
    }
#define TOKEN_KIND_DATA(name, str, dataType, dataName) TOKEN_KIND(name, str)
TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_DATA

#define TOKEN_KIND(name, str)                                                                \
    inline static Token Token_Create##name(u64 position, u64 line, u64 column, u64 length) { \
        Token token    = {};                                                                 \
        token.Kind     = TokenKind::name;                                                    \
        token.Position = position;                                                           \
        token.Line     = line;                                                               \
        token.Column   = column;                                                             \
        token.Length   = length;                                                             \
        return token;                                                                        \
    }
#define TOKEN_KIND_DATA(name, str, dataType, dataName)                                                          \
    inline static Token Token_Create##name(u64 position, u64 line, u64 column, u64 length, dataType dataName) { \
        Token token         = {};                                                                               \
        token.Kind          = TokenKind::name;                                                                  \
        token.Position      = position;                                                                         \
        token.Line          = line;                                                                             \
        token.Column        = column;                                                                           \
        token.Length        = length;                                                                           \
        token.Data.dataName = dataName;                                                                         \
        return token;                                                                                           \
    }
TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_DATA

#if !defined(KEEP_TOKEN_KINDS)
    #undef TOKEN_KINDS
#endif
