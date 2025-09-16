#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN_LEN 40

// Enumeração dos tipos de tokens da linguagem TINY
typedef enum {
    TOKEN_IF, TOKEN_THEN, TOKEN_ELSE, TOKEN_END, TOKEN_REPEAT, TOKEN_UNTIL,
    TOKEN_READ, TOKEN_WRITE, TOKEN_ID, TOKEN_NUM, TOKEN_ASSIGN, TOKEN_EQ, 
    TOKEN_LT, TOKEN_PLUS, TOKEN_MINUS, TOKEN_TIMES, TOKEN_OVER, TOKEN_LPAREN, 
    TOKEN_RPAREN, TOKEN_SEMI, TOKEN_EOF, TOKEN_ERROR
} TokenType;

// Estrutura para armazenar um token
typedef struct {
    TokenType type;
    char lexeme[MAX_TOKEN_LEN];
} Token;

// Palavras-chave da linguagem TINY
const char* reservedWords[] = {
    "if", "then", "else", "end", "repeat", "until", "read", "write"
};

TokenType reservedWordTokens[] = {
    TOKEN_IF, TOKEN_THEN, TOKEN_ELSE, TOKEN_END, TOKEN_REPEAT, 
    TOKEN_UNTIL, TOKEN_READ, TOKEN_WRITE
};

// Função para verificar se um lexema é palavra-chave
TokenType checkReserved(const char* s) {
    for (int i = 0; i < 8; i++) {
        if (strcmp(s, reservedWords[i]) == 0)
            return reservedWordTokens[i];
    }
    return TOKEN_ID;
}

// Função principal do scanner dirigido por tabela
Token getToken(FILE* source) {
    enum State { START, INASSIGN, INNUM, INID, DONE } state = START;
    char c;
    int tokenIndex = 0;
    char tokenString[MAX_TOKEN_LEN] = "";
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme[0] = '\0';

    while (state != DONE) {
        c = fgetc(source);
        switch (state) {
            case START:
                if (isspace(c)) {
                    // Ignora espaços em branco
                } else if (isalpha(c)) {
                    state = INID;
                    tokenString[tokenIndex++] = c;
                } else if (isdigit(c)) {
                    state = INNUM;
                    tokenString[tokenIndex++] = c;
                } else if (c == ':') {
                    state = INASSIGN;
                } else {
                    state = DONE;
                    switch (c) {
                        case EOF:
                            token.type = TOKEN_EOF;
                            break;
                        case '+': token.type = TOKEN_PLUS; break;
                        case '-': token.type = TOKEN_MINUS; break;
                        case '*': token.type = TOKEN_TIMES; break;
                        case '/': token.type = TOKEN_OVER; break;
                        case '=': token.type = TOKEN_EQ; break;
                        case '<': token.type = TOKEN_LT; break;
                        case '(': token.type = TOKEN_LPAREN; break;
                        case ')': token.type = TOKEN_RPAREN; break;
                        case ';': token.type = TOKEN_SEMI; break;
                        default:
                            token.type = TOKEN_ERROR;
                            tokenString[tokenIndex++] = c;
                            break;
                    }
                }
                break;
            case INID:
                if (!isalpha(c)) {
                    ungetc(c, source);
                    tokenString[tokenIndex] = '\0';
                    token.type = checkReserved(tokenString);
                    strcpy(token.lexeme, tokenString);
                    state = DONE;
                } else {
                    tokenString[tokenIndex++] = c;
                }
                break;
            case INNUM:
                if (!isdigit(c)) {
                    ungetc(c, source);
                    tokenString[tokenIndex] = '\0';
                    token.type = TOKEN_NUM;
                    strcpy(token.lexeme, tokenString);
                    state = DONE;
                } else {
                    tokenString[tokenIndex++] = c;
                }
                break;
            case INASSIGN:
                if (c == '=') {
                    token.type = TOKEN_ASSIGN;
                    strcpy(token.lexeme, ":=");
                } else {
                    ungetc(c, source);
                    token.type = TOKEN_ERROR;
                    strcpy(token.lexeme, ":");
                }
                state = DONE;
                break;
            default:
                state = DONE;
                break;
        }
    }
    if (token.lexeme[0] == '\0' && token.type != TOKEN_EOF) {
        tokenString[tokenIndex] = '\0';
        strcpy(token.lexeme, tokenString);
    }
    return token;
}

// Função para imprimir o token reconhecido
void printToken(Token token) {
    const char* tokenNames[] = {
        "IF", "THEN", "ELSE", "END", "REPEAT", "UNTIL", "READ", "WRITE",
        "ID", "NUM", "ASSIGN", "EQ", "LT", "PLUS", "MINUS", "TIMES", "OVER",
        "LPAREN", "RPAREN", "SEMI", "EOF", "ERROR"
    };
    printf("<%s, '%s'>\n", tokenNames[token.type], token.lexeme);
}
