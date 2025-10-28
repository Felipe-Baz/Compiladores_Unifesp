%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;
extern FILE *out;
extern int yylex();
void yyerror(const char *s);
%}

%union {
    int ival;
    char *sval;
}

/* Tokens */
%token IF ELSE WHILE RETURN INT VOID
%token EQ NEQ LEQ GEQ LT GT ASSIGN
%token PLUS MINUS TIMES DIVIDE
%token SEMI COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token <sval> ID
%token <ival> NUM

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
      type_specifier ID array_spec SEMI
    ;

array_spec:
      /* vazio */
    | LBRACKET NUM RBRACKET
    ;

fun_declaration:
      type_specifier ID LPAREN params RPAREN compound_stmt
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
      type_specifier ID
    | type_specifier ID LBRACKET RBRACKET
    ;

type_specifier:
      INT
    | VOID
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
      ID ASSIGN expression
    | ID LBRACKET expression RBRACKET ASSIGN expression
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
    | ID
    | ID LPAREN args RPAREN
    | ID LBRACKET expression RBRACKET  /* acessar elemento de array */
    | NUM
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
    fprintf(out, "\n[ERRO SINTÁTICO]: %s\n", s);
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
