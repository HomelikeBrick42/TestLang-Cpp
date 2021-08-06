#include "Ast.hpp"

void Ast_Print(Ast* ast, u64 indent) {
    auto PrintIndent = [&](u64 extraIndent = 0) -> void {
        for (u64 i = 0; i < (indent + extraIndent); i++) {
            Print("\t");
        }
    };

    auto PrintCategory = [&](const char* message) -> void {
        Print("\n");
        PrintIndent(1);
        Print(message);
    };

    if (ast == nullptr) {
        Print("nullptr");
        return;
    }

    switch (ast->Kind) {
        case AstKind::File: {
            Print("(<File> Scope: ");
            Ast_Print(ast->File.Scope, indent + 1);
            Print(")");
        } break;

        case AstKind::Scope: {
            Print("(<Scope> (\n");
            for (u64 i = 0; i < ast->Scope.Statements.Length; i++) {
                if (i != 0) {
                    Print(",\n");
                }

                PrintIndent(1);
                Ast_Print(ast->Scope.Statements[i], indent + 1);
            }
            Print("\n");
            PrintIndent();
            Print(")");
        } break;

        case AstKind::Declaration: {
            Print("(<Declaration>");
            PrintCategory("Constant: ");
            Print("%s", ast->Declaration.Constant ? "true" : "false");
            PrintCategory("Name: ");
            Ast_Print(ast->Declaration.Name, indent + 1);
            PrintCategory("Type: ");
            Ast_Print(ast->Declaration.Type, indent + 1);
            PrintCategory("Value: ");
            Ast_Print(ast->Declaration.Value, indent + 1);
            Print(")");
        } break;

        case AstKind::IntegerLiteral: {
            Print("(<Integer>");
            PrintCategory("Value: ");
            Print("%llu)", ast->IntegerLiteral.IntToken.Data.IntValue);
        } break;

        case AstKind::FloatLiteral: {
            Print("(<Float>");
            PrintCategory("Value: ");
            Print("%f)", ast->FloatLiteral.FloatToken.Data.FloatValue);
        } break;

        case AstKind::Name: {
            Print("(<Name>");
            PrintCategory("Value: ");
            Print("'%.*s')", (u32)ast->Name.Identifier.Data.Name.Length, ast->Name.Identifier.Data.Name.Data);
        } break;

        case AstKind::Unary: {
            Print("(<Unary>");
            PrintCategory("Operator: ");
            Print("'%s'", GetTokenKindName(ast->Unary.Operator.Kind).Data);
            PrintCategory("Operand: ");
            Ast_Print(ast->Unary.Operand, indent + 1);
            Print(")");
        } break;

        case AstKind::Binary: {
            Print("(<Binary>");
            PrintCategory("Operator: ");
            Print("'%s'", GetTokenKindName(ast->Binary.Operator.Kind).Data);
            PrintCategory("Left: ");
            Ast_Print(ast->Binary.Left, indent + 1);
            PrintCategory("Right: ");
            Ast_Print(ast->Binary.Right, indent + 1);
            Print(")");
        } break;

        case AstKind::TypeName: {
            Print("(<Type Name>");
            PrintCategory("Value: ");
            Print("'%.*s')", (u32)ast->TypeName.Name.Data.Name.Length, ast->TypeName.Name.Data.Name.Data);
        } break;

        case AstKind::TypePointer: {
            Print("(<Type Pointer>");
            PrintCategory("Pointer To: ");
            Ast_Print(ast->TypePointer.PointerTo, indent + 1);
            Print(")");
        } break;

        case AstKind::TypeDeref: {
            Print("(<Type Deref>");
            PrintCategory("Derefed Type: ");
            Ast_Print(ast->TypeDeref.DerefedType, indent + 1);
            Print(")");
        } break;

        case AstKind::_Statement_Begin:
        case AstKind::_Statement_End:
        case AstKind::_Expression_Begin:
        case AstKind::_Expression_End:
        case AstKind::_Type_Begin:
        case AstKind::_Type_End:
            ASSERT(false);
    }
}
