#include "Parser.hpp"

Parser::Parser(const String& source)
    : Lexer(source)
    , Errors(Array_Create<String>())
    , Current(this->Lexer.NextToken())
    , ParentFile(nullptr)
    , ParentScope(nullptr) {}

Parser::~Parser() = default;

Token Parser::NextToken() {
    Token token   = this->Current;
    this->Current = this->Lexer.NextToken();
    return token;
}

AstExpression* Parser::ParseExpression() {
    return ParseBinaryExpression(0);
}

AstExpression* Parser::ParsePrimaryExpression() {
    switch (this->Current.Kind) {
        case TokenKind::Identifier:
            return Ast_CreateName(this->ParentFile, this->ParentScope, { this->NextToken() });

        case TokenKind::Integer:
            return Ast_CreateIntegerLiteral(this->ParentFile, this->ParentScope, { this->NextToken() });

        case TokenKind::Float:
            return Ast_CreateFloatLiteral(this->ParentFile, this->ParentScope, { this->NextToken() });

        case TokenKind::LParen: {
            this->NextToken();
            AstExpression* expression = this->ParseExpression();
            if (!Token_IsRParen(this->Current)) {
                const char* message = "Expected ')' got '%.*s'";
                String name         = GetTokenKindName(this->Current.Kind);
                u64 size            = std::snprintf(nullptr, 0, message, (u64)name.Length, name.Data);
                char* buffer        = new char[size];
                std::sprintf(buffer, message, (u32)name.Length, name.Data);
                Array_Push(this->Errors, String(message));
            } else {
                this->NextToken();
            }
            return expression;
        } break;

        default: {
            // TODO: Error
            ASSERT(false);
            return nullptr;
        } break;
    }
}

inline static u64 GetUnaryOperatorPrecedence(const Token& token) {
    switch (token.Kind) {
        case TokenKind::Plus:
        case TokenKind::Minus:
            return 4;

        default:
            return 0;
    }
}

inline static u64 GetBinaryOperatorPrecedence(const Token& token) {
    switch (token.Kind) {
        case TokenKind::Asterisk:
        case TokenKind::Slash:
        case TokenKind::Percent:
            return 3;

        case TokenKind::Plus:
        case TokenKind::Minus:
            return 2;

        case TokenKind::LessThan:
        case TokenKind::LessThanEquals:
        case TokenKind::GreaterThan:
        case TokenKind::GreaterThanEquals:
            return 1;

        default:
            return 0;
    }
}

AstExpression* Parser::ParseBinaryExpression(u64 parentPrecedence) {
    AstExpression* left;
    u64 unaryPrecedence = GetUnaryOperatorPrecedence(this->Current);
    if (unaryPrecedence > parentPrecedence) {
        Token operator_        = this->NextToken();
        AstExpression* operand = this->ParseBinaryExpression(unaryPrecedence);
        left                   = Ast_CreateUnary(this->ParentFile, this->ParentScope, { operator_, operand });
    } else {
        left = this->ParsePrimaryExpression();
    }

    while (true) {
        u64 precedence = GetBinaryOperatorPrecedence(this->Current);
        if (precedence == 0 || precedence <= parentPrecedence) {
            break;
        }

        Token operator_      = this->NextToken();
        AstExpression* right = this->ParseBinaryExpression(precedence);
        left                 = Ast_CreateBinary(this->ParentFile, this->ParentScope, { left, operator_, right });
    }

    return left;
}
