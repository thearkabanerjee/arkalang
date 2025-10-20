
#include <stdio.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    TOKEN_VARIABLE,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGN,
    TOKEN_NUMBER,
    TOKEN_PRINT,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[32];
} Token;

char *source;
int pos = 0;

Token getNextToken() {
    Token token;
    
    while (source[pos] != '\0') {
        char c = source[pos];

        if (isspace(c)) {
            pos++;  // Skip spaces
            continue;
        }

        if (c == '+') { token.type = TOKEN_PLUS; strcpy(token.text, "+"); pos++; return token; }
        if (c == '=') { token.type = TOKEN_ASSIGN; strcpy(token.text, "="); pos++; return token; }
        if (c == ';') { token.type = TOKEN_SEMICOLON; strcpy(token.text, ";"); pos++; return token; }

        if (isdigit(c)) {
            int start = pos;
            while (isdigit(source[pos])) pos++;
            strncpy(token.text, source + start, pos - start);
            token.text[pos - start] = '\0';
            token.type = TOKEN_NUMBER;
            return token;
        }

        if (strncmp(source + pos, "variable", 3) == 0) {
            token.type = TOKEN_VARIABLE;
            strcpy(token.text, "variable");
            pos += 3;
            return token;
        }

        if (strncmp(source + pos, "print", 5) == 0) {
            token.type = TOKEN_PRINT;
            strcpy(token.text, "print");
            pos += 5;
            return token;
        }

        if (isalpha(c)) {
            int start = pos;
            while (isalnum(source[pos])) pos++;
            strncpy(token.text, source + start, pos - start);
            token.text[pos - start] = '\0';
            token.type = TOKEN_IDENTIFIER;
            return token;
        }

        printf("Unknown token: %c\n", c);
        pos++;
    }

    token.type = TOKEN_EOF;
    return token;
}

int main() {
    source = "let x = 5; print(x + 3);";
    Token token;
    
    while ((token = getNextToken()).type != TOKEN_EOF) {
        printf("Token: %s\n", token.text);
    }

    return 0;
}
