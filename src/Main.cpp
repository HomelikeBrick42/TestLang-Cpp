#include "Defines.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Parser.hpp"

// void ResolveAst(Ast* ast);

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

    // ResolveAst(statement);
    Ast_Print(statement);

    delete[] data;
    return 0;
}

bool TypesEqual(AstType* a, AstType* b) {
    ASSERT(Ast_IsType(a) && Ast_IsType(b));

    if (a->Kind != b->Kind) {
        return false;
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

/*
void ResolveAst(Ast* ast) {
    switch (ast->Kind) {
        case AstKind::Declaration: {
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
*/
