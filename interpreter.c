#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_VARIABLE, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_NUMBER,
    TOKEN_PRINT, TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULTIPLY, TOKEN_DIVIDE,
    TOKEN_SEMICOLON, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_EOF
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char text[32];
} Token;

// AST node types
typedef enum {
    NODE_VARIABLE_DECLARATION, NODE_PRINT_STATEMENT, NODE_BINARY_OPERATION,
    NODE_NUMBER, NODE_IDENTIFIER
} NodeType;

// AST node structure
typedef struct ASTNode {
    NodeType type;
    char value[32];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

// Symbol table (stores variables)
typedef struct {
    char name[32];
    int value;
} Symbol;

Symbol symbolTable[100];
int symbolCount = 0;

char *source;
int pos = 0;
Token currentToken;

// Function to get the next token
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

// Parse expressions
ASTNode *parseExpression() {
    ASTNode *left = parsePrimary();
    while (currentToken.type == TOKEN_PLUS) {
        advance();
        ASTNode *right = parsePrimary();
        left = newASTNode(NODE_BINARY_OPERATION, "+", left, right);
    }
    return left;
}

// Add variable to symbol table
void addVariable(char *name, int value) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            symbolTable[i].value = value;
            return;
        }
    }
    strcpy(symbolTable[symbolCount].name, name);
    symbolTable[symbolCount].value = value;
    symbolCount++;
}

// Retrieve variable value
int getVariableValue(char *name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return symbolTable[i].value;
        }
    }
    printf("Runtime Error: Undefined variable '%s'\n", name);
    exit(1);
}

// Parse variable declaration
ASTNode *parseVariableDeclaration() {
    advance();
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

// Parse print statement
ASTNode *parsePrintStatement() {
    advance();
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

// Execute AST nodes
int execute(ASTNode *node) {
    if (node->type == NODE_NUMBER) return atoi(node->value);
    if (node->type == NODE_IDENTIFIER) return getVariableValue(node->value);
    if (node->type == NODE_BINARY_OPERATION) {
        int left = execute(node->left);
        int right = execute(node->right);
        return left + right;
    }
    if (node->type == NODE_VARIABLE_DECLARATION) {
        int value = execute(node->left);
        addVariable(node->value, value);
        return value;
    }
    if (node->type == NODE_PRINT_STATEMENT) {
        printf("%d\n", execute(node->left));
    }
    return 0;
}

// Parse and execute program
void parse() {
    advance();
    while (currentToken.type != TOKEN_EOF) {
        ASTNode *stmt = (currentToken.type == TOKEN_VARIABLE) ? parseVariableDeclaration() : parsePrintStatement();
        execute(stmt);
    }
}

// Main function
int main() {
    source = "variable a = 10; print(a + 5);";
    parse();
    return 0;
}
