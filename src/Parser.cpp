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

AstScope* Parser::ParseScope(Array<Ast*> extraVarsInScope) {
    this->ExpectToken(TokenKind::LBrace);
    AstScope* scope   = Ast_CreateScope(this->ParentFile, this->ParentScope, { Array_Create<AstStatement*>(), extraVarsInScope });
    this->ParentScope = scope;
    while (!Token_IsRBrace(this->Current) && !Token_IsEndOfFile(this->Current)) {
        Array_Add(scope->Scope.Statements, this->ParseStatement());
    }
    this->ExpectToken(TokenKind::RBrace);
    this->ParentScope = scope->ParentScope;
    return scope;
}

AstStatement* Parser::ParseStatement() {
    if (Token_IsSemicolon(this->Current)) {
        this->ExpectToken(TokenKind::Semicolon);
        return this->ParseStatement();
    }

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
            if (declaration != nullptr && !Ast_IsProcedure(declaration->Declaration.Value)) {
                this->ExpectToken(TokenKind::Semicolon);
            }
            return declaration;
        } break;

        default: {
            return expression;
        } break;
    }
}

AstDeclaration* Parser::ParseDeclaration(AstName* name) {
    this->ExpectToken(TokenKind::Colon);

    AstType* type = nullptr;
    if (this->Current.Kind != TokenKind::Colon && this->Current.Kind != TokenKind::Equals) {
        type = this->ParseType();
    }

    if (Token_IsColon(this->Current)) {
        this->ExpectToken(TokenKind::Colon);
        AstExpression* value = this->ParseExpression();
        return Ast_CreateDeclaration(this->ParentFile, this->ParentScope, { true, name, type, value });
    } else if (Token_IsEquals(this->Current)) {
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
            if (Token_IsRParen(this->Current)) {
                return this->ParseProcedure();
            }
            AstExpression* expression = this->ParseExpression();
            if (Token_IsColon(this->Current)) {
                if (!Ast_IsName(expression)) {
                    Array_Add(this->Errors, String("Expected name")); // TODO: Better error
                }
                return this->ParseProcedure(expression);
            }
            this->ExpectToken(TokenKind::RParen);
            return expression;
        } break;

        default: {
            const char* message = "Unexpected '%.*s'";
            String name         = GetTokenKindName(this->NextToken().Kind);
            u64 size            = std::snprintf(nullptr, 0, message, (u32)name.Length, name.Data);
            char* buffer        = new char[size];
            std::sprintf(buffer, message, (u32)name.Length, name.Data);
            Array_Add(this->Errors, String(buffer));
            return nullptr;
        } break;
    }
}

inline static u64 GetUnaryOperatorPrecedence(const Token& token) {
    switch (token.Kind) {
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::Asterisk:
        case TokenKind::Caret:
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
AstType* Parser::ParseType() {
    switch (this->Current.Kind) {
        case TokenKind::Caret: {
            this->ExpectToken(TokenKind::Caret);
            return Ast_CreateTypePointer(this->ParentFile, this->ParentScope, { this->ParseType() });
        } break;

        case TokenKind::Asterisk: {
            this->ExpectToken(TokenKind::Asterisk);
            return Ast_CreateTypeDeref(this->ParentFile, this->ParentScope, { this->ParseType() });
        } break;

        case TokenKind::Identifier: {
            Token token = this->ExpectToken(TokenKind::Identifier);
            return Ast_CreateTypeName(this->ParentFile, this->ParentScope, { token });
        } break;

        case TokenKind::LParen: {
            // TODO: Procedure types
            this->ExpectToken(TokenKind::LParen);
            AstType* type = this->ParseType();
            this->ExpectToken(TokenKind::RParen);
            return type;
        } break;

        default: {
            const char* message = "Unexpected '%.*s'";
            String name         = GetTokenKindName(this->NextToken().Kind);
            u64 size            = std::snprintf(nullptr, 0, message, (u32)name.Length, name.Data);
            char* buffer        = new char[size];
            std::sprintf(buffer, message, (u32)name.Length, name.Data);
            Array_Add(this->Errors, String(buffer));
            return nullptr;
        } break;
    }
}

AstProcedure* Parser::ParseProcedure(AstName* firstArgName) {
    Array<AstDeclaration*> arguments = Array_Create<AstDeclaration*>();

    if (firstArgName != nullptr) {
        this->ExpectToken(TokenKind::Colon);

        AstType* type = nullptr;
        if (!Token_IsEquals(this->Current)) {
            type = this->ParseType();
        }

        AstExpression* value = nullptr;
        if (Token_IsEquals(this->Current)) {
            value = this->ParseExpression();
        }

        if (type == nullptr && value == nullptr) {
            Array_Add(this->Errors, String("Cannot have a declaration with no type nor value"));
        }

        Array_Add(arguments, Ast_CreateDeclaration(this->ParentFile, this->ParentScope, { false, firstArgName, type, value }));

        if (!Token_IsRParen(this->Current)) {
            this->ExpectToken(TokenKind::Comma);
        }
    }

    while (!Token_IsRParen(this->Current) && !Token_IsEndOfFile(this->Current)) {
        AstName* name = Ast_CreateName(this->ParentFile, this->ParentScope, { this->ExpectToken(TokenKind::Identifier) });
        this->ExpectToken(TokenKind::Colon);

        AstType* type = nullptr;
        if (!Token_IsEquals(this->Current)) {
            type = this->ParseType();
        }

        AstExpression* value = nullptr;
        if (Token_IsEquals(this->Current)) {
            value = this->ParseExpression();
        }

        if (type == nullptr && value == nullptr) {
            Array_Add(this->Errors, String("Cannot have a declaration with no type nor value"));
        }

        Array_Add(arguments, Ast_CreateDeclaration(this->ParentFile, this->ParentScope, { false, name, type, value }));

        if (!Token_IsRParen(this->Current)) {
            this->ExpectToken(TokenKind::Comma);
        }
    }

    this->ExpectToken(TokenKind::RParen);

    AstType* returnType = nullptr;
    if (Token_IsMinus(this->Current)) {
        this->ExpectToken(TokenKind::Minus);
        this->ExpectToken(TokenKind::GreaterThan);
        returnType = this->ParseType();
    } else {
        returnType = Ast_CreateTypeVoid(this->ParentFile, this->ParentScope, {});
    }

    if (Token_IsLBrace(this->Current)) {
        Array<Ast*> scopeParams = Array_Create<Ast*>();
        AstProcedure* procedure =
            Array_Add(scopeParams, Ast_CreateProcedure(this->ParentFile, this->ParentScope, { arguments, returnType, nullptr }));
        AstScope* body            = this->ParseScope(scopeParams);
        procedure->Procedure.Body = body;
        return procedure;
    } else {
        Array<AstType*> argumentTypes = Array_Create<AstType*>();
        for (u64 i = 0; i < arguments.Length; i++) {
            AstType* type = arguments[i]->Declaration.Type;
            if (type == nullptr) {
                Array_Add(this->Errors, String("Must always have argument type for procedure type!"));
            }
            Array_Add(argumentTypes, type);
        }
        return Ast_CreateTypeProcedure(this->ParentFile, this->ParentScope, { argumentTypes, returnType });
    }
}
