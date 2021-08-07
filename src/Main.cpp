#include "Defines.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Parser.hpp"

void ResolveAst(Ast* ast);

int main(int argc, char** argv) {
    // Not sure if this is needed for no buffering of stderr and stdout
    std::setbuf(stderr, nullptr);
    std::setbuf(stdout, nullptr);

    if (argc != 2) {
        Error("Invalid arguments!\nUsage: %s file", argv[0]);
    }

    std::FILE* file = std::fopen(argv[1], "rb");
    if (file == nullptr) {
        Error("Unable to open file: '%s'", argv[1]);
    }

    std::fseek(file, 0, SEEK_END);
    u64 fileSize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    u8* data = new u8[fileSize];
    if (fread(data, sizeof(u8), fileSize, file) != fileSize) {
        Error("Unable to read file: '%s'", argv[1]);
    }
    std::fclose(file);

    String fileSource(data, fileSize);

    Parser parser(fileSource);
    AstStatement* statement = parser.ParseStatement();

    if (parser.Lexer.Errors.Length != 0) {
        PrintError("\nLexer Errors:\n");

        for (u64 i = 0; i < parser.Lexer.Errors.Length; i++) {
            PrintError("%.*s\n", (u32)parser.Lexer.Errors[i].Length, parser.Lexer.Errors[i].Data);
        }
    }

    if (parser.Errors.Length != 0) {
        PrintError("\nParser Errors:\n");

        for (u64 i = 0; i < parser.Errors.Length; i++) {
            PrintError("%.*s\n", (u32)parser.Errors[i].Length, parser.Errors[i].Data);
        }
    }

    if (parser.Lexer.Errors.Length != 0 || parser.Errors.Length != 0) {
        Error("\nThere were errors. We cannot continue.");
    }

    ResolveAst(statement);
    Ast_Print(statement);

    delete[] data;
    return 0;
}

bool TypesEqual(AstType* a, AstType* b) {
    ASSERT(Ast_IsType(a) && Ast_IsType(b));

    if (a->Kind != b->Kind) {
        if (Ast_IsTypeName(a) && Ast_IsTypeName(b)) {
            return TypesEqual(a->Type, b->Type);
        } else if (Ast_IsTypeName(a)) {
            return TypesEqual(a->Type, b);
        } else if (Ast_IsTypeName(b)) {
            return TypesEqual(a, b->Type);
        } else {
            return false;
        }
    }

    switch (a->Kind) {
        case AstKind::TypeName: {
            return a->TypeName.Name.Data.Name == b->TypeName.Name.Data.Name;
        } break;

        case AstKind::TypeProcedure: {
            if (a->TypeProcedure.Arguments.Length != b->TypeProcedure.Arguments.Length) {
                return false;
            }

            if (!TypesEqual(a->TypeProcedure.ReturnType, b->TypeProcedure.ReturnType)) {
                return false;
            }

            for (u64 i = 0; i < a->TypeProcedure.Arguments.Length; i++) {
                if (!TypesEqual(a->TypeProcedure.Arguments[i], b->TypeProcedure.Arguments[i])) {
                    return false;
                }
            }

            return true;
        } break;

        case AstKind::TypeVoid: {
            return true;
        } break;

        case AstKind::TypeFloat: {
            return a->TypeFloat.Size == b->TypeFloat.Size;
        } break;

        case AstKind::TypeInteger: {
            return a->TypeInteger.Signed == b->TypeInteger.Signed && a->TypeInteger.Size == b->TypeInteger.Size;
        } break;

        case AstKind::TypeDeref: {
            return TypesEqual(a->TypeDeref.DerefedType, b->TypeDeref.DerefedType);
        } break;

        case AstKind::TypePointer: {
            return TypesEqual(a->TypePointer.PointerTo, b->TypePointer.PointerTo);
        } break;

        default: {
            ASSERT(false);
            return false;
        } break;
    }
}

#include <functional>

static AstTypeType* TypeType   = Ast_CreateTypeType(nullptr, nullptr, nullptr, {});
static AstTypeVoid* TypeVoid   = Ast_CreateTypeVoid(nullptr, nullptr, nullptr, {});
static AstTypeInteger* TypeInt = Ast_CreateTypeInteger(nullptr, nullptr, nullptr, { 0, true });

void ResolveAst(Ast* ast) {
    if (ast == nullptr) {
        return;
    }

    if (ast->Completion == AstCompletion::Complete) {
        return;
    } else if (ast->Completion == AstCompletion::Completing) {
        Error("Cyclic dependency found!");
    } else { // AstCompletion::Incomplete
        ast->Completion = AstCompletion::Completing;
    }

    switch (ast->Kind) {
        case AstKind::Declaration: {
            ResolveAst(ast->Declaration.Type);
            ResolveAst(ast->Declaration.Value);

            if (ast->Declaration.Type == nullptr) {
                ast->Declaration.Type = ast->Declaration.Value->Type;
            } else {
                if (!TypesEqual(ast->Declaration.Type, ast->Declaration.Value->Type)) {
                    Error("Types not compatible!");
                }
            }

            ast->Type = TypeVoid;
        } break;

        case AstKind::File: {
            ResolveAst(ast->File.Scope);
            ast->Type = TypeVoid;
        } break;

        case AstKind::Scope: {
            for (u64 i = 0; i < ast->Scope.ExtraVariablesInScope.Length; i++) {
                ResolveAst(ast->Scope.ExtraVariablesInScope[i]);
            }
            for (u64 i = 0; i < ast->Scope.Statements.Length; i++) {
                ResolveAst(ast->Scope.Statements[i]);
            }
            ast->Type = TypeVoid;
        } break;

        case AstKind::IntegerLiteral: {
            ast->Type = TypeInt;
        } break;

        case AstKind::FloatLiteral: {
            ASSERT(false);
        } break;

        case AstKind::Name: {
            std::function<void(AstScope*)> findFunc = [&](AstScope* scope) -> void {
                if (scope == nullptr) {
                    return;
                }

                for (u64 i = 0; i < scope->Scope.Statements.Length; i++) {
                    if (scope->Scope.Statements[i] == ast || scope->Scope.Statements[i] == ast->ParentStatement) {
                        break;
                    }

                    if (Ast_IsDeclaration(scope->Scope.Statements[i])) {
                        if (scope->Scope.Statements[i]->Declaration.Name->Name.Identifier.Data.Name ==
                            ast->Name.Identifier.Data.Name) {
                            ResolveAst(scope->Scope.Statements[i]);
                            ast->Type = scope->Scope.Statements[i]->Declaration.Type;
                            return;
                        }
                    }
                }

                findFunc(scope->ParentScope);
            };

            String name = ast->TypeName.Name.Data.Name;
            if (name == "type") {
                ast->Type = TypeType;
            } else if (name == "void") {
                ast->Type = TypeVoid;
            } else if (name == "int") {
                ast->Type = TypeInt;
            } else {
                findFunc(ast->ParentScope);
                if (ast->Type == nullptr) {
                    Error("Could not find name!");
                }
            }
        } break;

        case AstKind::Unary:
            break;

        case AstKind::Binary:
            break;

        case AstKind::Procedure: {
            Array<AstType*> argumentTypes = Array_Create<AstType*>();
            for (u64 i = 0; i < ast->Procedure.Arguments.Length; i++) {
                ResolveAst(ast->Procedure.Arguments[i]);
                Array_Add(argumentTypes, ast->Procedure.Arguments[i]->Declaration.Type);
            }
            ResolveAst(ast->Procedure.ReturnType);
            ResolveAst(ast->Procedure.Body);
            ast->Type = Ast_CreateTypeProcedure(
                ast->ParentFile, ast->ParentScope, ast->ParentStatement, { argumentTypes, ast->Procedure.ReturnType });
        } break;

        case AstKind::TypeName: {
            std::function<void(AstScope*)> findFunc = [&](AstScope* scope) -> void {
                if (scope == nullptr) {
                    return;
                }

                for (u64 i = 0; i < scope->Scope.Statements.Length; i++) {
                    if (scope->Scope.Statements[i] == ast || scope->Scope.Statements[i] == ast->ParentStatement) {
                        break;
                    }

                    if (Ast_IsDeclaration(scope->Scope.Statements[i])) {
                        if (scope->Scope.Statements[i]->Declaration.Name->Name.Identifier.Data.Name ==
                            ast->TypeName.Name.Data.Name) {
                            ResolveAst(scope->Scope.Statements[i]);
                            ast->Type = scope->Scope.Statements[i]->Declaration.Type;
                            std::memcpy(ast->Type, scope->Scope.Statements[i]->Declaration.Type, sizeof(Ast));
                            return;
                        }
                    }
                }

                findFunc(scope->ParentScope);
            };

            String name = ast->TypeName.Name.Data.Name;
            if (name == "type") {
                std::memcpy(ast, TypeType, sizeof(Ast));
            } else if (name == "void") {
                std::memcpy(ast, TypeVoid, sizeof(Ast));
            } else if (name == "int") {
                std::memcpy(ast, TypeInt, sizeof(Ast));
            } else {
                findFunc(ast->ParentScope);
                if (ast->Type == nullptr) {
                    Error("Could not find name!");
                }
            }
        } break;

        case AstKind::TypePointer: {
            ResolveAst(ast->TypePointer.PointerTo);
            ast->Type = TypeType;
        } break;

        case AstKind::TypeDeref: {
            ResolveAst(ast->TypeDeref.DerefedType);
            if (!Ast_IsTypePointer(ast->TypeDeref.DerefedType)) {
                Error("Unable to deref type that is not pointer!");
            }
            std::memcpy(ast, ast->TypeDeref.DerefedType, sizeof(Ast));
            ast->Type = TypeType;
        } break;

        case AstKind::TypeProcedure: {
            for (u64 i = 0; i < ast->TypeProcedure.Arguments.Length; i++) {
                ResolveAst(ast->TypeProcedure.Arguments[i]);
            }
            ResolveAst(ast->TypeProcedure.ReturnType);
            ast->Type = TypeType;
        } break;

        case AstKind::TypeInteger:
        case AstKind::TypeFloat:
        case AstKind::TypeVoid:
        case AstKind::TypeType: {
            ast->Type = TypeType;
        } break;

        case AstKind::_Statement_Begin:
        case AstKind::_Statement_End:
        case AstKind::_Expression_Begin:
        case AstKind::_Expression_End:
        case AstKind::_Type_Begin:
        case AstKind::_Type_End:
            ASSERT(false);
    }

    ast->Completion = AstCompletion::Complete;
}
