
#include <stdio.h>
#include <stdlib.h>
#include "scanner.c"


int main() {
    FILE *source = fopen("sample.tny", "r");
    if (!source) {
        printf("Erro ao abrir o arquivo sample.tny\n");
        return 1;
    }
    Token token;
    do {
        token = getToken(source);
        printToken(token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
    fclose(source);
    return 0;
}