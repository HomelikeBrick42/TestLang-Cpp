#include "Defines.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Parser.hpp"

int main(int argc, char** argv) {
    // Not sure if this is needed for no buffering of stderr and stdout
    std::setbuf(stderr, nullptr);
    std::setbuf(stdout, nullptr);

    if (argc != 2) {
        Error("Invalid arguments!\nUsage: %s file", argv[0]);
    }

    FILE* file = std::fopen(argv[1], "rb");
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

    Ast_Print(statement);

    delete[] data;
    return 0;
}
