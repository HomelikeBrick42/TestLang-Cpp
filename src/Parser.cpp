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

Token Parser::ExpectToken(TokenKind kind) {
    if (this->Current.Kind != kind) {
        const char* message = "Expected '%.*s' got '%.*s'";
        String name1        = GetTokenKindName(kind);
        String name2        = GetTokenKindName(this->Current.Kind);
        u64 size            = std::snprintf(nullptr, 0, message, (u32)name1.Length, name1.Data, (u32)name2.Length, name2.Data);
        char* buffer        = new char[size];
        std::sprintf(buffer, message, (u32)name1.Length, name1.Data, (u32)name2.Length, name2.Data);
        Array_Add(this->Errors, String(buffer));

        // TODO: Think about this
        return {};
    } else {
        return this->NextToken();
    }
}

AstStatement* Parser::ParseStatement() {
    AstExpression* expression = this->ParseExpression();

    switch (this->Current.Kind) {
        case TokenKind::Colon: {
            if (!Ast_IsName(expression)) {
                const char* message = "Expected 'Name' got '%.*s'";
                String name         = GetAstKindName(expression->Kind);
                u64 size            = std::snprintf(nullptr, 0, message, (u32)name.Length, name.Data);
                char* buffer        = new char[size];
                std::sprintf(buffer, message, (u32)name.Length, name.Data);
                Array_Add(this->Errors, String(message));
            }
            AstDeclaration* declaration = this->ParseDeclaration(expression);
            this->ExpectToken(TokenKind::Semicolon);
            return declaration;
        } break;

        default: {
            this->ExpectToken(TokenKind::Semicolon);
            return expression;
        } break;
    }
}

AstDeclaration* Parser::ParseDeclaration(AstName* name) {
    this->ExpectToken(TokenKind::Colon);

    AstType* type = nullptr;
    if (this->Current.Kind != TokenKind::Colon && this->Current.Kind != TokenKind::Equals) {
        type = nullptr; // TODO:
    }

    if (this->Current.Kind == TokenKind::Colon) {
        this->ExpectToken(TokenKind::Colon);
        AstExpression* value = this->ParseExpression();
        return Ast_CreateDeclaration(this->ParentFile, this->ParentScope, { true, name, type, value });
    } else if (this->Current.Kind == TokenKind::Equals) {
        this->ExpectToken(TokenKind::Equals);
        AstExpression* value = this->ParseExpression();
        return Ast_CreateDeclaration(this->ParentFile, this->ParentScope, { false, name, type, value });
    } else {
        if (type == nullptr) {
            Array_Add(this->Errors, String("Cannot declare variable with nether type nor value"));
        }
        return Ast_CreateDeclaration(this->ParentFile, this->ParentScope, { false, name, type, nullptr });
    }
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
                u64 size            = std::snprintf(nullptr, 0, message, (u32)name.Length, name.Data);
                char* buffer        = new char[size];
                std::sprintf(buffer, message, (u32)name.Length, name.Data);
                Array_Add(this->Errors, String(buffer));
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
