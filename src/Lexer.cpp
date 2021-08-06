#include "Lexer.hpp"

Lexer::Lexer(const String& source) : Source(source), Position(0), Line(1), Column(1), Errors(Array_Create<String>()) {}

Lexer::~Lexer() = default;

Token Lexer::NextToken() {
#define Current ((this->Position >= this->Source.Length) ? '\0' : this->Source[this->Position])

    auto NextChar = [this]() -> u8 {
        u8 current = Current;
        this->Position++;
        this->Column++;
        if (current == '\n') {
            this->Line++;
            this->Column = 1;
        }
        return current;
    };

    while (true) {
        u64 startPosition = this->Position;
        u64 startLine     = this->Line;
        u64 startColumn   = this->Column;

#define MATCH(chr, kind)                                                     \
    case chr: {                                                              \
        NextChar();                                                          \
        return Token_Create##kind(startPosition, startLine, startColumn, 1); \
    } break

#define MATCH2(chr, kind, chr2, kind2)                                            \
    case chr: {                                                                   \
        NextChar();                                                               \
        if (Current == chr2) {                                                    \
            NextChar();                                                           \
            return Token_Create##kind2(startPosition, startLine, startColumn, 2); \
        }                                                                         \
        return Token_Create##kind(startPosition, startLine, startColumn, 1);      \
    } break

        switch (Current) {
            case '\0': {
                return Token_CreateEndOfFile(startPosition, startLine, startColumn, 0);
            } break;

                MATCH('(', LParen);
                MATCH(')', RParen);
                MATCH('{', LBrace);
                MATCH('}', RBrace);
                MATCH('[', LBracket);
                MATCH(']', RBracket);
                MATCH(':', Colon);
                MATCH(';', Semicolon);
                MATCH(',', Comma);

                MATCH2('+', Plus, '=', PlusEquals);
                MATCH2('-', Minus, '=', MinusEquals);
                MATCH2('*', Asterisk, '=', AsteriskEquals);
                MATCH2('/', Slash, '=', SlashEquals);
                MATCH2('%', Percent, '=', PercentEquals);
                MATCH2('=', Equals, '=', EqualsEquals);
                MATCH2('!', ExclamationMark, '=', ExclamationMarkEquals);
                MATCH2('<', LessThan, '=', LessThanEquals);
                MATCH2('>', GreaterThan, '=', GreaterThanEquals);

            case ' ':
            case '\t':
            case '\n':
            case '\r': {
                NextChar();
                continue;
            } break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                u64 base = 10;
                if (Current == '0') {
                    switch (NextChar()) {
                        case 'x':
                        case 'X': {
                            NextChar();
                            base = 16;
                        } break;

                        case 'b':
                        case 'B': {
                            NextChar();
                            base = 2;
                        } break;

                        default: {
                            base = 10;
                        } break;
                    }
                }

                u64 intValue = 0;

                while (true) {
                    switch (Current) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                        case 'A':
                        case 'B':
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f': {
                            u64 value = 0;
                            if (Current >= '0' && Current <= '9') {
                                value = Current - '0';
                            } else if (Current >= 'A' && Current <= 'F') {
                                value = Current - 'A';
                            } else if (Current >= 'a' && Current <= 'a') {
                                value = Current - 'a';
                            }

                            if ((value == 0 && Current != '0') || value >= base) {
                                const char* message = "Digit '%c' too big for base %llu";
                                u8 character        = Current;
                                u64 size            = std::snprintf(nullptr, 0, message, character, base);
                                char* buffer        = new char[size];
                                std::sprintf(buffer, message, character, base);
                                Array_Push(this->Errors, String(buffer));
                            }

                            intValue *= base;
                            intValue += value;

                            NextChar();
                        } break;

                        case '_': {
                            NextChar();
                        }
                            continue;

                        case '.': {
                            // TODO: Float
                            ASSERT(false);
                        } break;

                        default: {
                        } break;
                    }
                    break;
                }

                return Token_CreateInteger(startPosition, startLine, startColumn, this->Position - startPosition, intValue);
            } break;

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w':
            case 'x':
            case 'y':
            case 'z': {
                Array<u8> buffer = Array_Create<u8>();

                while (true) {
                    switch (Current) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                        case 'A':
                        case 'B':
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                        case 'G':
                        case 'H':
                        case 'I':
                        case 'J':
                        case 'K':
                        case 'L':
                        case 'M':
                        case 'N':
                        case 'O':
                        case 'P':
                        case 'Q':
                        case 'R':
                        case 'S':
                        case 'T':
                        case 'U':
                        case 'V':
                        case 'W':
                        case 'X':
                        case 'Y':
                        case 'Z':
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f':
                        case 'g':
                        case 'h':
                        case 'i':
                        case 'j':
                        case 'k':
                        case 'l':
                        case 'm':
                        case 'n':
                        case 'o':
                        case 'p':
                        case 'q':
                        case 'r':
                        case 's':
                        case 't':
                        case 'u':
                        case 'v':
                        case 'w':
                        case 'x':
                        case 'y':
                        case 'z': {
                            Array_Push(buffer, NextChar());
                        }
                            continue;

                        default: {
                        } break;
                    }
                    break;
                }

                String identifier = { new u8[buffer.Length + 1], buffer.Length };
                std::memcpy(identifier.Data, buffer.Data, buffer.Length * sizeof(u8));
                identifier[buffer.Length] = '\0';
                return Token_CreateIdentifier(startPosition, startLine, startColumn, this->Position - startPosition, identifier);
            } break;

            default: {
                const char* message = "Unknown character '%c'";
                u8 character        = NextChar();
                u64 size            = std::snprintf(nullptr, 0, message, character);
                char* buffer        = new char[size];
                std::sprintf(buffer, message, character);
                Array_Push(this->Errors, String(buffer));
            } break;
        }

#undef MATCH
#undef MATCH2
    }

#undef Current
#undef NextChar
}
