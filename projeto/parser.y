%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;
extern FILE *out;
extern int yylex();
void yyerror(const char *s);

typedef struct {
    char* nome;
    char* tipo;
    int linha;
    char* scope;
} Simbolo;

Simbolo tabela[100];
int n_simbolos = 0;
char* escopo_atual = "global";


int busca(char* nome, char* scope) {
    for (int i = 0; i < n_simbolos; i++) {
        if (
          strcmp(tabela[i].nome, nome) == 0 &&
          (
            strcmp(tabela[i].scope, scope) == 0 ||
            strcmp(tabela[i].scope, "global") == 0
          )
        )
            return i;
    }
    return -1;
}

void insere(char* nome, char* tipo, int linha, char* scope) {
    if (busca(nome, scope) != -1) {
        printf("Erro semantico: variavel '%s' ja declarada.\n", nome);
        return;
    }
    printf("Foi inserido o %s com tipo %s, linha: %d, escopo: %s\n", nome, tipo, linha, scope);

    tabela[n_simbolos].nome = strdup(nome);
    tabela[n_simbolos].tipo = strdup(tipo);
    tabela[n_simbolos].scope = strdup(scope);
    tabela[n_simbolos].linha = linha;

    n_simbolos++;
}
%}

%locations

%union {
    int ival;
    char *sval;
    char *tipo;
}

/* Tokens */
%token IF ELSE WHILE RETURN INT VOID
%token OUTPUT INPUT
%token EQ NEQ LEQ GEQ LT GT ASSIGN
%token PLUS MINUS TIMES DIVIDE
%token SEMI COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token <sval> ID
%token <ival> NUM

%type <tipo> type_specifier expression simple_expression additive_expression term factor

%%

program:
    declaration_list { printf("[ANALISE SINTATICA - OK]\n"); }
    ;

declaration_list:
      declaration_list declaration
    | declaration
    ;

declaration:
      var_declaration
    | fun_declaration
    ;

/* Suporte a arrays na declaração de variáveis */
var_declaration:
      type_specifier ID array_spec SEMI {
          insere($2, $1, @2.first_line, escopo_atual);
      }
    ;

array_spec:
      /* vazio */
    | LBRACKET NUM RBRACKET
    ;

fun_declaration:
      type_specifier ID LPAREN {
            insere($2, $1, @2.first_line, escopo_atual);

            escopo_atual = strdup($2);
      } 
      params RPAREN 
      compound_stmt {
            escopo_atual = "global";
      }
    ;


params:
      param_list
    | VOID
    ;

param_list:
      param_list COMMA param
    | param
    ;

param:
      type_specifier ID {
          insere($2, $1, @2.first_line, escopo_atual);
      }
    | type_specifier ID LBRACKET RBRACKET {
          insere($2, $1, @2.first_line, escopo_atual);
      }
    ;

type_specifier:
      INT  { $$ = "int"; }
    | VOID { $$ = "void"; }
    ;

compound_stmt:
      LBRACE local_declarations statement_list RBRACE
    ;

local_declarations:
      local_declarations var_declaration
    | /* vazio */
    ;

statement_list:
      statement_list statement
    | /* vazio */
    ;

statement:
      expression_stmt
    | compound_stmt
    | selection_stmt
    | iteration_stmt
    | return_stmt
    | io_stmt
    ;

/* Novos statements de I/O */
io_stmt:
      OUTPUT LPAREN expression RPAREN SEMI
    ;

expression_stmt:
      expression SEMI
    | SEMI
    ;

selection_stmt:
      IF LPAREN expression RPAREN statement
    | IF LPAREN expression RPAREN statement ELSE statement
    ;

iteration_stmt:
      WHILE LPAREN expression RPAREN statement
    ;

return_stmt:
      RETURN SEMI
    | RETURN expression SEMI
    ;

/* Suporte a elementos de array no lado esquerdo da atribuição */
expression:
      ID ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
      }
    | ID LBRACKET expression RBRACKET ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
    }
    | simple_expression
    ;

simple_expression:
      additive_expression relop additive_expression
    | additive_expression
    ;

relop:
      LEQ | LT | GT | GEQ | EQ | NEQ
    ;

additive_expression:
      additive_expression addop term
    | term
    ;

addop:
      PLUS | MINUS
    ;

term:
      term mulop factor
    | factor
    ;

mulop:
      TIMES | DIVIDE
    ;

factor:
      LPAREN expression RPAREN
    | ID {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
    }
    | ID LPAREN args RPAREN {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: funcao '%s' nao declarada.\n", $1);
          }
    }
    | ID LBRACKET expression RBRACKET {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
    }
    | NUM
    | INPUT LPAREN RPAREN
    ;

args:
      arg_list
    | /* vazio */
    ;

arg_list:
      arg_list COMMA expression
    | expression
    ;

%%

void yyerror(const char *s) {
    printf("\n[ERRO SINTATICO]: %s\n", s);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <entrada.c-> <saida.txt>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    out = fopen(argv[2], "w");
    if (!out) {
        perror("Erro ao abrir arquivo de saída");
        fclose(yyin);
        return 1;
    }

    yyparse();

    fclose(yyin);
    fclose(out);
    return 0;
}
