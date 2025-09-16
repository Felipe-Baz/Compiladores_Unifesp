#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum { INICIO = 0, EH_ID = 1 };
enum { LETRA = 0, DIGITO = 1, OUTRO = 2 };

// cria a tabela de transição
int trans[2][3] = {
    //             LETRA   DIGITO   OUTRO
    /* START(0) */ {   1 ,     0 ,     0 },  // se estiver em "X" estado, e receber "L" letra, vai pra que estado?
    /* IN_ID(1) */ {   1 ,     1 ,     0 }
};

// Qual a classe da palavra
int char_class(int c) {
    if (isalpha((unsigned char)c)) return LETRA;
    if (isdigit((unsigned char)c)) return DIGITO;
    return OUTRO;
}

int main(int argc, char *argv[]) {
    const char *infile = "sort.txt";
    const char *outfile = "out.txt";

    if (argc >= 2) infile = argv[1];
    if (argc >= 3) outfile = argv[2];

    FILE *fin = fopen(infile, "r");
    if (!fin) {
        perror("Erro abrindo arquivo de entrada");
        return 1;
    }
    FILE *fout = fopen(outfile, "w");
    if (!fout) {
        perror("Erro abrindo arquivo de saída");
        fclose(fin);
        return 1;
    }


    int state = INICIO;
    int c;
    char id_buffer[256];
    int id_len = 0;

    // Lista de palavras reservadas
    const char* reserved[] = {"if", "while", "int", "else", "for", "return", "void", "read", "write"};
    int reserved_count = sizeof(reserved)/sizeof(reserved[0]);

    while ((c = fgetc(fin)) != EOF) {
        int cls = char_class(c);
        int next = trans[state][cls];

        if (state == INICIO) {
            if (cls == LETRA) {
                id_buffer[0] = c;
                id_len = 1;
                state = next;
            } else {
                fputc(c, fout);
                state = next;
            }
        } else if (state == EH_ID) {
            if (cls == LETRA || cls == DIGITO) {
                if (id_len < 255) {
                    id_buffer[id_len++] = c;
                }
                state = next;
            } else {
                id_buffer[id_len] = '\0';
                int is_reserved = 0;
                for (int i = 0; i < reserved_count; i++) {
                    if (strcmp(id_buffer, reserved[i]) == 0) {
                        fputs(id_buffer, fout);
                        is_reserved = 1;
                        break;
                    }
                }
                if (!is_reserved) {
                    fputs("ID", fout);
                }
                fputc(c, fout);
                id_len = 0;
                state = next;
            }
        }
    }

    if (state == EH_ID) {
        id_buffer[id_len] = '\0';
        int is_reserved = 0;
        for (int i = 0; i < reserved_count; i++) {
            if (strcmp(id_buffer, reserved[i]) == 0) {
                fputs(id_buffer, fout);
                is_reserved = 1;
                break;
            }
        }
        if (!is_reserved) {
            fputs("ID", fout);
        }
    }

    fclose(fin);
    fclose(fout);
    printf("Processamento concluído. Saída em: %s\n", outfile);
    return 0;
}
