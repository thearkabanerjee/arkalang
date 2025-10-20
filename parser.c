
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_VARIABLE,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGN,
    TOKEN_NUMBER,
    TOKEN_PRINT,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_EOF
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char text[32];
} Token;

// AST node types
typedef enum {
    NODE_VARIABLE_DECLARATION,
    NODE_PRINT_STATEMENT,
    NODE_BINARY_OPERATION,
    NODE_NUMBER,
    NODE_IDENTIFIER
} NodeType;

// AST node structure
typedef struct ASTNode {
    NodeType type;
    char value[32]; // For numbers and identifiers
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

// Global variables for parsing
char *source;
int pos = 0;
Token currentToken;

// Function to get the next token from source
Token getNextToken() {
    Token token;

    while (source[pos] != '\0') {
        char c = source[pos];

        if (isspace(c)) { pos++; continue; }
        if (c == '+') { token.type = TOKEN_PLUS; strcpy(token.text, "+"); pos++; return token; }
        if (c == '=') { token.type = TOKEN_ASSIGN; strcpy(token.text, "="); pos++; return token; }
        if (c == ';') { token.type = TOKEN_SEMICOLON; strcpy(token.text, ";"); pos++; return token; }
        if (c == '(') { token.type = TOKEN_LPAREN; strcpy(token.text, "("); pos++; return token; }
        if (c == ')') { token.type = TOKEN_RPAREN; strcpy(token.text, ")"); pos++; return token; }

        if (isdigit(c)) {
            int start = pos;
            while (isdigit(source[pos])) pos++;
            strncpy(token.text, source + start, pos - start);
            token.text[pos - start] = '\0';
            token.type = TOKEN_NUMBER;
            return token;
        }

        if (strncmp(source + pos, "variable", 8) == 0) {
            token.type = TOKEN_VARIABLE;
            strcpy(token.text, "variable");
            pos += 8;
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

// Function to advance to the next token
void advance() {
    currentToken = getNextToken();
}

// Create a new AST node
ASTNode *newASTNode(NodeType type, char *value, ASTNode *left, ASTNode *right) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = type;
    if (value) strcpy(node->value, value);
    node->left = left;
    node->right = right;
    return node;
}

// Parse numbers and identifiers
ASTNode *parsePrimary() {
    if (currentToken.type == TOKEN_NUMBER) {
        ASTNode *node = newASTNode(NODE_NUMBER, currentToken.text, NULL, NULL);
        advance();
        return node;
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        ASTNode *node = newASTNode(NODE_IDENTIFIER, currentToken.text, NULL, NULL);
        advance();
        return node;
    }
    printf("Syntax Error: Expected a number or identifier\n");
    exit(1);
}

// Parse binary operations (e.g., a + 5)
ASTNode *parseExpression() {
    ASTNode *left = parsePrimary();
    while (currentToken.type == TOKEN_PLUS) {
        advance();
        ASTNode *right = parsePrimary();
        left = newASTNode(NODE_BINARY_OPERATION, "+", left, right);
    }
    return left;
}

// Parse variable declaration (e.g., variable a = 10;)
ASTNode *parseVariableDeclaration() {
    advance(); // Skip 'variable'
    if (currentToken.type != TOKEN_IDENTIFIER) {
        printf("Syntax Error: Expected variable name\n");
        exit(1);
    }
    char varName[32];
    strcpy(varName, currentToken.text);
    advance();

    if (currentToken.type != TOKEN_ASSIGN) {
        printf("Syntax Error: Expected '=' after variable name\n");
        exit(1);
    }
    advance();

    ASTNode *expr = parseExpression();
    if (currentToken.type != TOKEN_SEMICOLON) {
        printf("Syntax Error: Expected ';' at the end of statement\n");
        exit(1);
    }
    advance();
    return newASTNode(NODE_VARIABLE_DECLARATION, varName, expr, NULL);
}

// Parse print statement (e.g., print(a + 5);)
ASTNode *parsePrintStatement() {
    advance(); // Skip 'print'
    if (currentToken.type != TOKEN_LPAREN) {
        printf("Syntax Error: Expected '(' after 'print'\n");
        exit(1);
    }
    advance();

    ASTNode *expr = parseExpression();

    if (currentToken.type != TOKEN_RPAREN) {
        printf("Syntax Error: Expected ')' in print statement\n");
        exit(1);
    }
    advance();

    if (currentToken.type != TOKEN_SEMICOLON) {
        printf("Syntax Error: Expected ';' at the end of print statement\n");
        exit(1);
    }
    advance();
    return newASTNode(NODE_PRINT_STATEMENT, NULL, expr, NULL);
}

// Parse the entire program
void parse() {
    advance(); // Get first token
    while (currentToken.type != TOKEN_EOF) {
        if (currentToken.type == TOKEN_VARIABLE) {
            ASTNode *varDecl = parseVariableDeclaration();
            printf("Parsed variable declaration: %s\n", varDecl->value);
        } else if (currentToken.type == TOKEN_PRINT) {
            ASTNode *printStmt = parsePrintStatement();
            printf("Parsed print statement\n");
        } else {
            printf("Syntax Error: Unexpected token '%s'\n", currentToken.text);
            exit(1);
        }
    }
}

// Main function
int main() {
    source = "variable a = 10; print(a + 5);";
    parse();
    return 0;
}
