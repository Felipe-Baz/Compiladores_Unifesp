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
    int param_counter;
} Simbolo;

Simbolo tabela[100];
int n_simbolos = 0;
char* escopo_atual = "global";
int has_return = 0;

/* Geração de código intermediário */
FILE *codigo_intermediario;
int temp_count = 0;
int label_count = 0;

char* novo_temp() {
    char* t = (char*)malloc(20);
    sprintf(t, "t%d", temp_count++);
    return t;
}

char* novo_label() {
    char* l = (char*)malloc(20);
    sprintf(l, "L%d", label_count++);
    return l;
}

void emite(char* op, char* arg1, char* arg2, char* result) {
    if (codigo_intermediario) {
        if (arg2) {
            fprintf(codigo_intermediario, "%s = %s %s %s\n", result, arg1, op, arg2);
        } else if (arg1) {
            fprintf(codigo_intermediario, "%s %s %s %s\n", op, arg1, arg2 ? arg2 : "", result ? result : "");
        } else {
            fprintf(codigo_intermediario, "%s %s\n", op, result ? result : "");
        }
    }
}

void emite_label(char* label) {
    if (codigo_intermediario) {
        fprintf(codigo_intermediario, "%s:\n", label);
    }
}

void emite_goto(char* label) {
    if (codigo_intermediario) {
        fprintf(codigo_intermediario, "goto %s\n", label);
    }
}

void emite_if(char* cond, char* op, char* label) {
    if (codigo_intermediario) {
        fprintf(codigo_intermediario, "if %s %s goto %s\n", cond, op, label);
    }
}

void emite_param(char* param) {
    if (codigo_intermediario) {
        fprintf(codigo_intermediario, "param %s\n", param);
    }
}

void emite_call(char* func, int n_params, char* result) {
    if (codigo_intermediario) {
        if (result) {
            fprintf(codigo_intermediario, "%s = call %s, %d\n", result, func, n_params);
        } else {
            fprintf(codigo_intermediario, "call %s, %d\n", func, n_params);
        }
    }
}


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

void insere(char* nome, char* tipo, int linha, char* scope, int param_counter) {
    if (busca(nome, scope) != -1) {
        printf("Erro semantico: variavel '%s' ja declarada.\n", nome);
        return;
    }
    // printf("Foi inserido o %s com tipo %s, linha: %d, escopo: %s\n", nome, tipo, linha, scope);

    tabela[n_simbolos].nome = strdup(nome);
    tabela[n_simbolos].tipo = strdup(tipo);
    tabela[n_simbolos].scope = strdup(scope);
    tabela[n_simbolos].linha = linha;
    tabela[n_simbolos].param_counter = param_counter;

    n_simbolos++;
}
%}

%locations

%union {
    int ival;
    char *sval;
    char *tipo;
    int param_count;
    struct {
        char* addr;  /* endereço (temporário ou variável) */
        char* tipo;  /* tipo da expressão */
    } expr_attr;
}

/* Tokens */
%token IF ELSE WHILE RETURN INT VOID
%token OUTPUT INPUT
%token EQ NEQ LEQ GEQ LT GT ASSIGN
%token PLUS MINUS TIMES DIVIDE
%token SEMI COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token <sval> ID
%token <ival> NUM

%type <tipo> type_specifier
%type <expr_attr> expression simple_expression additive_expression term factor
%type <param_count> params param_list args arg_list

%%

program:
    declaration_list { 
        if(busca("main", "global") == -1) {
            printf("Erro semantico: funcao 'main' nao declarada.\n");
        } else {
            printf("[ANALISE SINTATICA - OK]\n"); 
        }
    }
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
        int id = busca($2, escopo_atual);
        if (id != -1) {
            printf("Erro semantico: variavel '%s' ja declarada.\n", $2);
        } else {
            if (strcmp($1, "void") == 0) {
                printf("Erro semantico: variavel '%s' nao pode ser do tipo void.\n", $2);
            } else {
                insere($2, $1, @2.first_line, escopo_atual, 0);
            }
        }
      }
    ;

array_spec:
      /* vazio */
    | LBRACKET NUM RBRACKET
    ;

fun_declaration:
      type_specifier ID LPAREN {
            int id = busca($2, escopo_atual);

            if (id != -1) {
                printf("Erro semantico: funcao '%s' ja declarada.\n", $2);
            } else {
                insere($2, $1, @2.first_line, escopo_atual, 0);
            }

            escopo_atual = strdup($2);
            
            /* Gera label de entrada da função */
            if (codigo_intermediario) {
                fprintf(codigo_intermediario, "\nfunc %s\n", $2);
            }
      } 
      params {
            int idx = busca($<sval>2, "global");
            if (idx != -1) {
                tabela[idx].param_counter = $5;
            }
      } 
      RPAREN 
      compound_stmt {
            int idx = busca(escopo_atual, "global");
            if (idx != -1) {
                if (strcmp(tabela[idx].tipo, "int") == 0 && has_return == 0) {
                    printf("Erro semantico: funcao '%s' do tipo int deve ter um return.\n", escopo_atual);
                }
            }

            /* Gera label de saída da função */
            if (codigo_intermediario) {
                fprintf(codigo_intermediario, "endfunc %s\n", escopo_atual);
            }

            escopo_atual = "global";
            has_return = 0;
      }
    ;


params:
      param_list { $$ = $1; }
    | VOID { $$ = 0; }
    ;

param_list:
      param_list COMMA param { $$ = $1 + 1; }
    | param { $$ = 1; }
    ;

param:
      type_specifier ID {
          int id = busca($2, escopo_atual);
          if (id != -1) {
              printf("Erro semantico: variavel '%s' ja declarada.\n", $2);
          } else {
              if (strcmp($1, "void") == 0) {
                  printf("Erro semantico: parametro '%s' nao pode ser do tipo void.\n", $2);
              } else {
                  insere($2, $1, @2.first_line, escopo_atual, 0);
              }
          }
      }
    | type_specifier ID LBRACKET RBRACKET {
          insere($2, $1, @2.first_line, escopo_atual, 0);
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
    | return_stmt {
        has_return = 1;
    }
    | io_stmt
    | call_stmt  /* NOVO: chamada de função como statement */
    ;

/* NOVO: Chamada de função como statement (sem uso do retorno) */
call_stmt:
      ID LPAREN args RPAREN SEMI {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: funcao '%s' nao declarada.\n", $1);
          } else {
            if (tabela[idx].param_counter != $3) {
                  printf("Erro semantico: funcao '%s' espera %d parametro(s), mas recebeu %d argumento(s).\n", 
                         $1, tabela[idx].param_counter, $3);
              } else {
                if(tabela[idx].tipo == "int") {
                    printf("Aviso semantico: retorno da funcao '%s' do tipo int nao utilizado.\n", $1);
                }
              }
          }
          
          /* Gera código para chamada de função como statement */
          emite_call($1, $3, NULL);
          /* NÃO verifica tipo void aqui - é permitido chamar função void como statement */
      }
    ;

/* Novos statements de I/O */
io_stmt:
      OUTPUT LPAREN expression RPAREN SEMI {
          /* Gera código para output */
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "output(%s)\n", $3.addr);
          }
      }
    ;

expression_stmt:
      expression SEMI
    | SEMI
    ;

selection_stmt:
      IF LPAREN expression RPAREN {
          /* Gera label e condicional */
          char* label_fim = novo_label();
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "ifFalse %s goto %s\n", $3.addr, label_fim);
          }
          $<sval>$ = label_fim; /* salva label para uso posterior */
      } statement {
          /* Emite label de fim do if */
          emite_label($<sval>5);
      }
    | IF LPAREN expression RPAREN {
          /* Gera labels para if-else */
          char* label_else = novo_label();
          char* label_fim = novo_label();
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "ifFalse %s goto %s\n", $3.addr, label_else);
          }
          $<sval>$ = label_else;
          $<sval>0 = label_fim; /* hack: armazena label_fim em posição anterior */
      } statement {
          char* label_fim = novo_label();
          emite_goto(label_fim);
          emite_label($<sval>5); /* label do else */
          $<sval>$ = label_fim;
      } ELSE statement {
          emite_label($<sval>7); /* label de fim */
      }
    ;

iteration_stmt:
      WHILE {
          /* Gera label de início do loop */
          char* label_inicio = novo_label();
          emite_label(label_inicio);
          $<sval>$ = label_inicio;
      } LPAREN expression RPAREN {
          /* Gera label de saída e condicional */
          char* label_fim = novo_label();
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "ifFalse %s goto %s\n", $4.addr, label_fim);
          }
          $<sval>$ = label_fim;
      } statement {
          /* Volta ao início e emite label de fim */
          emite_goto($<sval>2);
          emite_label($<sval>6);
      }
    ;

return_stmt:
      RETURN SEMI { 
        int id = busca(escopo_atual, "global");
        if (id != -1) {
            if (strcmp(tabela[id].tipo, "int") == 0) {
                printf("Erro semantico: funcao '%s' do tipo int tem que retornar valor.\n", escopo_atual);
            }
        }
        
        /* Gera código para return sem valor */
        if (codigo_intermediario) {
            fprintf(codigo_intermediario, "return\n");
        }
     }
    | RETURN expression SEMI {
        /* Gera código para return com valor */
        if (codigo_intermediario) {
            fprintf(codigo_intermediario, "return %s\n", $2.addr);
        }
    }
    ;

/* Suporte a elementos de array no lado esquerdo da atribuição */
expression:
      ID ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          } else {
              /* Gera código: ID = expressão */
              if (codigo_intermediario) {
                  fprintf(codigo_intermediario, "%s = %s\n", $1, $3.addr);
              }
              $$.addr = strdup($1);
              $$.tipo = tabela[idx].tipo;
          }
      }
    | ID LBRACKET expression RBRACKET ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          } else {
              /* Gera código: ID[index] = expressão */
              if (codigo_intermediario) {
                  fprintf(codigo_intermediario, "%s[%s] = %s\n", $1, $3.addr, $6.addr);
              }
              $$.addr = strdup($1);
              $$.tipo = tabela[idx].tipo;
          }
    }
    | simple_expression {
          $$.addr = $1.addr;
          $$.tipo = $1.tipo;
    }
    ;

simple_expression:
      additive_expression relop additive_expression {
          char* temp = novo_temp();
          char op[10];
          
          /* Determina o operador relacional */
          /* Nota: precisaríamos passar o operador via $2, mas por simplicidade 
             vamos usar um operador genérico aqui */
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "%s = %s relop %s\n", temp, $1.addr, $3.addr);
          }
          
          $$.addr = temp;
          $$.tipo = "int";
      }
    | additive_expression {
          $$.addr = $1.addr;
          $$.tipo = $1.tipo;
      }
    ;

relop:
      LEQ | LT | GT | GEQ | EQ | NEQ
    ;

additive_expression:
      additive_expression addop term {
          char* temp = novo_temp();
          
          /* Gera código de três endereços para adição/subtração */
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "%s = %s addop %s\n", temp, $1.addr, $3.addr);
          }
          
          $$.addr = temp;
          $$.tipo = "int";
      }
    | term {
          $$.addr = $1.addr;
          $$.tipo = $1.tipo;
      }
    ;

addop:
      PLUS | MINUS
    ;

term:
      term mulop factor {
          char* temp = novo_temp();
          
          /* Gera código de três endereços para multiplicação/divisão */
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "%s = %s mulop %s\n", temp, $1.addr, $3.addr);
          }
          
          $$.addr = temp;
          $$.tipo = "int";
      }
    | factor {
          $$.addr = $1.addr;
          $$.tipo = $1.tipo;
      }
    ;

mulop:
      TIMES | DIVIDE
    ;

factor:
      LPAREN expression RPAREN {
          $$.addr = $2.addr;
          $$.tipo = $2.tipo;
      }
    | ID {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
              $$.addr = strdup($1);
              $$.tipo = "int";
          } else if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", $1);
              $$.addr = strdup($1);
              $$.tipo = "void";
          } else {
              $$.addr = strdup($1);
              $$.tipo = tabela[idx].tipo;
          }
    }
    | ID LPAREN args RPAREN {
          /* CHAMADA DE FUNÇÃO DENTRO DE EXPRESSÃO - retorno é usado */
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: funcao '%s' nao declarada.\n", $1);
          } else if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", $1);
          } else {
              // Verifica número de argumentos
              if (tabela[idx].param_counter != $3) {
                  printf("Erro semantico: funcao '%s' espera %d parametro(s), mas recebeu %d argumento(s).\n", 
                         $1, tabela[idx].param_counter, $3);
              }
          }
          
          /* Gera código de chamada de função */
          char* temp = novo_temp();
          emite_call($1, $3, temp);
          
          $$.addr = temp;
          $$.tipo = (idx != -1) ? tabela[idx].tipo : "int";
    }
    | ID LBRACKET expression RBRACKET {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }

          if (idx != -1 && strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", $1);
          }
          
          /* Gera código para acesso a array */
          char* temp = novo_temp();
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "%s = %s[%s]\n", temp, $1, $3.addr);
          }
          
          $$.addr = temp;
          $$.tipo = (idx != -1) ? tabela[idx].tipo : "int";
    }
    | NUM {
          char* num_str = (char*)malloc(20);
          sprintf(num_str, "%d", $1);
          $$.addr = num_str;
          $$.tipo = "int";
      }
    | INPUT LPAREN RPAREN {
          char* temp = novo_temp();
          if (codigo_intermediario) {
              fprintf(codigo_intermediario, "%s = input()\n", temp);
          }
          $$.addr = temp;
          $$.tipo = "int";
      }
    ;

args:
      arg_list { $$ = $1; }
    | /* vazio */ { $$ = 0; }
    ;

arg_list:
      arg_list COMMA expression { $$ = $1 + 1; }
    | expression { $$ = 1; }
    ;

%%

void yyerror(const char *s) {
    printf("\n[ERRO SINTATICO na linha %d]: %s\n", yylloc.first_line, s);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <entrada.c-> <saida.txt> [codigo_intermediario.txt]\n", argv[0]);
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

    /* Abre arquivo de código intermediário se fornecido */
    if (argc >= 4) {
        codigo_intermediario = fopen(argv[3], "w");
        if (!codigo_intermediario) {
            perror("Aviso: não foi possível abrir arquivo de código intermediário");
        } else {
            fprintf(codigo_intermediario, "# Código Intermediário - Three Address Code\n\n");
        }
    }

    yyparse();

    fclose(yyin);
    fclose(out);
    
    if (codigo_intermediario) {
        fclose(codigo_intermediario);
        printf("\n[Codigo intermediario gerado com sucesso]\n");
    }
    
    return 0;
}