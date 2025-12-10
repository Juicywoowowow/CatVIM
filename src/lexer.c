#include "lexer.h"

void init_lexer(Lexer* lexer, const char* source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
}

static bool is_at_end(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->current[-1];
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (*lexer->current != expected) return false;
    lexer->current++;
    return true;
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    return token;
}

static Token error_token(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    return token;
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                advance(lexer);
                break;
            case '-':
                if (peek_next(lexer) == '-') {
                    // Comment until end of line
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) advance(lexer);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (is_at_end(lexer)) return error_token(lexer, "Unterminated string");

    advance(lexer); // Closing quote
    return make_token(lexer, TOKEN_STRING);
}

static Token number(Lexer* lexer) {
    while (is_digit(peek(lexer))) advance(lexer);

    // Look for decimal part
    if (peek(lexer) == '.' && is_digit(peek_next(lexer))) {
        advance(lexer); // Consume '.'
        while (is_digit(peek(lexer))) advance(lexer);
        return make_token(lexer, TOKEN_F64);
    }

    return make_token(lexer, TOKEN_I64);
}

static TokenType check_keyword(Lexer* lexer, int start, int length,
                               const char* rest, TokenType type) {
    if (lexer->current - lexer->start == start + length &&
        memcmp(lexer->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Lexer* lexer) {
    switch (lexer->start[0]) {
        case 'a': return check_keyword(lexer, 1, 2, "nd", TOKEN_AND);
        case 'b':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'o': return check_keyword(lexer, 2, 2, "ol", TOKEN_TYPE_BOOL);
                    case 'u': return check_keyword(lexer, 2, 5, "iltin", TOKEN_BUILTIN);
                }
            }
            break;
        case 'c': return check_keyword(lexer, 1, 3, "all", TOKEN_CALL);
        case 'd': return check_keyword(lexer, 1, 5, "efine", TOKEN_DEFINE);
        case 'e':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'l': return check_keyword(lexer, 2, 2, "se", TOKEN_ELSE);
                    case 'n': return check_keyword(lexer, 2, 1, "d", TOKEN_END);
                }
            }
            break;
        case 'f':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case '3': return check_keyword(lexer, 2, 1, "2", TOKEN_TYPE_F32);
                    case '6': return check_keyword(lexer, 2, 1, "4", TOKEN_TYPE_F64);
                    case 'a': return check_keyword(lexer, 2, 3, "lse", TOKEN_FALSE);
                    case 'o': return check_keyword(lexer, 2, 1, "r", TOKEN_FOR);
                    case 'r': return check_keyword(lexer, 2, 2, "om", TOKEN_FROM);
                    case 'u': return check_keyword(lexer, 2, 6, "nction", TOKEN_FUNCTION);
                }
            }
            break;
        case 'i':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case '8': return TOKEN_TYPE_I8;
                    case '1': return check_keyword(lexer, 2, 1, "6", TOKEN_TYPE_I16);
                    case '3': return check_keyword(lexer, 2, 1, "2", TOKEN_TYPE_I32);
                    case '6': return check_keyword(lexer, 2, 1, "4", TOKEN_TYPE_I64);
                    case 'f': return TOKEN_IF;
                }
            }
            break;
        case 'n':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a': return check_keyword(lexer, 2, 7, "mespace", TOKEN_NAMESPACE);
                    case 'i': return check_keyword(lexer, 2, 1, "l", TOKEN_NIL);
                    case 'o': return check_keyword(lexer, 2, 1, "t", TOKEN_NOT);
                }
            }
            break;
        case 'o': return check_keyword(lexer, 1, 1, "r", TOKEN_OR);
        case 'p':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a': return check_keyword(lexer, 2, 7, "rameter", TOKEN_PARAMETER);
                    case 'r': return check_keyword(lexer, 2, 5, "ivate", TOKEN_PRIVATE);
                    case 'u': return check_keyword(lexer, 2, 4, "blic", TOKEN_PUBLIC);
                }
            }
            break;
        case 'r':
            if (lexer->current - lexer->start > 2) {
                if (lexer->start[1] == 'e' && lexer->start[2] == 't') {
                    if (lexer->current - lexer->start > 6) {
                        return check_keyword(lexer, 3, 8, "urn_type", TOKEN_RETURN_TYPE);
                    }
                    return check_keyword(lexer, 3, 3, "urn", TOKEN_RETURN);
                }
            }
            break;
        case 's':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'e': return check_keyword(lexer, 2, 1, "t", TOKEN_SET);
                    case 't': return check_keyword(lexer, 2, 4, "ring", TOKEN_TYPE_STRING);
                }
            }
            break;
        case 't':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'o': return TOKEN_TO;
                    case 'r': return check_keyword(lexer, 2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'u':
            if (lexer->current - lexer->start > 1 && lexer->start[1] >= '1' && lexer->start[1] <= '8') {
                switch (lexer->start[1]) {
                    case '8': return TOKEN_TYPE_U8;
                    case '1': return check_keyword(lexer, 2, 1, "6", TOKEN_TYPE_U16);
                    case '3': return check_keyword(lexer, 2, 1, "2", TOKEN_TYPE_U32);
                    case '6': return check_keyword(lexer, 2, 1, "4", TOKEN_TYPE_U64);
                }
            }
            break;
        case 'v':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a': return check_keyword(lexer, 2, 6, "riable", TOKEN_VARIABLE);
                    case 'o': return check_keyword(lexer, 2, 2, "id", TOKEN_TYPE_VOID);
                }
            }
            break;
        case 'w': return check_keyword(lexer, 1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer* lexer) {
    while (is_alpha(peek(lexer)) || is_digit(peek(lexer))) advance(lexer);
    return make_token(lexer, identifier_type(lexer));
}

Token scan_token(Lexer* lexer) {
    skip_whitespace(lexer);
    lexer->start = lexer->current;

    if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);

    char c = advance(lexer);

    if (is_alpha(c)) return identifier(lexer);
    if (is_digit(c)) return number(lexer);

    switch (c) {
        case '{': return make_token(lexer, TOKEN_LEFT_BRACE);
        case '}': return make_token(lexer, TOKEN_RIGHT_BRACE);
        case ':': return make_token(lexer, TOKEN_COLON);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '-': return make_token(lexer, TOKEN_MINUS);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '/': return make_token(lexer, TOKEN_SLASH);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '!':
            return make_token(lexer, match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return make_token(lexer, match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return make_token(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return make_token(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string(lexer);
    }

    return error_token(lexer, "Unexpected character");
}
