#pragma once

#include "Defines.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Token.hpp"

#define AST_KINDS                                                         \
    AST_KIND(File, "File", {                                              \
        AstScope* Scope;                                                  \
        Array<Ast*> ExtraVariablesInScope; /* e.g. Function parameters */ \
    })                                                                    \
                                                                          \
    AST_KIND_BEGIN(Statement)                                             \
                                                                          \
    AST_KIND(Scope, "Scope", { Array<AstStatement*> Statements; })        \
                                                                          \
    AST_KIND(Declaration, "Declaration", {                                \
        AstName* Name; /* TODO: Type */                                   \
        AstExpression* Value;                                             \
    })                                                                    \
                                                                          \
    AST_KIND_BEGIN(Expression)                                            \
                                                                          \
    AST_KIND(IntegerLiteral, "Integer Literal", { Token IntToken; })      \
    AST_KIND(FloatLiteral, "Float Literal", { Token FloatToken; })        \
    AST_KIND(Name, "Name", { Token Identifier; })                         \
                                                                          \
    AST_KIND(Unary, "Unary", {                                            \
        Token Operator;                                                   \
        AstExpression* Operand;                                           \
    })                                                                    \
                                                                          \
    AST_KIND(Binary, "Binary", {                                          \
        AstExpression* Left;                                              \
        Token Operator;                                                   \
        AstExpression* Right;                                             \
    })                                                                    \
                                                                          \
    AST_KIND_END(Expression)                                              \
                                                                          \
    AST_KIND_END(Statement)

enum struct AstKind {
#define AST_KIND(name, str, ...) name,
#define AST_KIND_BEGIN(name)     _##name##_Begin,
#define AST_KIND_END(name)       _##name##_End,
    AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END
};

inline static String GetAstKindName(AstKind kind) {
    switch (kind) {
#define AST_KIND(name, str, ...) \
    case AstKind::name:          \
        return str;
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
        AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END
        default:
            return "";
    }
}

struct Ast;

#define AST_KIND(name, str, ...) using Ast##name = Ast;
#define AST_KIND_BEGIN(name)     using Ast##name = Ast;
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

#define AST_KIND(name, str, ...) using Ast##name##Data = struct __VA_ARGS__;
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

struct Ast {
    AstKind Kind;
    AstFile* ParentFile;
    AstScope* ParentScope;

    union {
#define AST_KIND(name, str, ...) Ast##name##Data name;
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
        AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END
    };
};

#define AST_KIND(name, str, ...)                      \
    inline static bool Ast_Is##name(const Ast* ast) { \
        if (ast == nullptr) {                         \
            return false;                             \
        }                                             \
        return ast->Kind == AstKind::name;            \
    }
#define AST_KIND_BEGIN(name)                                                               \
    inline static bool Ast_Is##name(const Ast* ast) {                                      \
        if (ast == nullptr) {                                                              \
            return false;                                                                  \
        }                                                                                  \
        return ast->Kind > AstKind::_##name##_Begin && ast->Kind < AstKind::_##name##_End; \
    }
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

#define AST_KIND(name, str, ...)                                                                      \
    inline static Ast##name* Ast_Create##name(AstFile* file, AstScope* scope, Ast##name##Data data) { \
        ASSERT(file == nullptr || Ast_IsFile(file));                                                  \
        ASSERT(scope == nullptr || Ast_IsScope(scope));                                               \
        Ast* ast         = new Ast{};                                                                 \
        ast->Kind        = AstKind::name;                                                             \
        ast->ParentFile  = file;                                                                      \
        ast->ParentScope = scope;                                                                     \
        std::memcpy(&ast->name, &data, sizeof(Ast##name##Data));                                      \
        return ast;                                                                                   \
    }
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

#if !defined(KEEP_AST_KINDS)
    #undef AST_KINDS
#endif
