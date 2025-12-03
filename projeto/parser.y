%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;
extern FILE *out;
extern int yylex();
void yyerror(const char *s);

// Estrutura para a Árvore Sintática Abstrata (AST)
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_FUN_DECL,
    NODE_PARAM,
    NODE_COMPOUND_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_RETURN_STMT,
    NODE_ASSIGN,
    NODE_EXPRESSION,
    NODE_VAR,
    NODE_OP,
    NODE_NUM,
    NODE_CALL,
    NODE_OUTPUT,
    NODE_INPUT,
    NODE_ARRAY_ACCESS
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* name;        // Para identificadores, operadores, tipos
    int value;         // Para números
    struct ASTNode** children;
    int num_children;
    int line;
} ASTNode;

ASTNode* root = NULL;

ASTNode* create_node(NodeType type, char* name, int value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->name = name ? strdup(name) : NULL;
    node->value = value;
    node->children = NULL;
    node->num_children = 0;
    node->line = line;
    return node;
}

void add_child(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;
    parent->num_children++;
    parent->children = (ASTNode**)realloc(parent->children, sizeof(ASTNode*) * parent->num_children);
    parent->children[parent->num_children - 1] = child;
}

void print_ast(ASTNode* node, int depth, FILE* ast_file) {
    if (!node) return;
    
    for (int i = 0; i < depth; i++) fprintf(ast_file, "  ");
    
    switch(node->type) {
        case NODE_PROGRAM:
            fprintf(ast_file, "PROGRAM\n");
            break;
        case NODE_VAR_DECL:
            fprintf(ast_file, "VAR_DECL: %s\n", node->name ? node->name : "");
            break;
        case NODE_FUN_DECL:
            fprintf(ast_file, "FUN_DECL: %s\n", node->name ? node->name : "");
            break;
        case NODE_PARAM:
            fprintf(ast_file, "PARAM: %s\n", node->name ? node->name : "");
            break;
        case NODE_COMPOUND_STMT:
            fprintf(ast_file, "COMPOUND_STMT\n");
            break;
        case NODE_IF_STMT:
            fprintf(ast_file, "IF_STMT\n");
            break;
        case NODE_WHILE_STMT:
            fprintf(ast_file, "WHILE_STMT\n");
            break;
        case NODE_RETURN_STMT:
            fprintf(ast_file, "RETURN_STMT\n");
            break;
        case NODE_ASSIGN:
            fprintf(ast_file, "ASSIGN\n");
            break;
        case NODE_EXPRESSION:
            fprintf(ast_file, "EXPRESSION\n");
            break;
        case NODE_VAR:
            fprintf(ast_file, "VAR: %s\n", node->name ? node->name : "");
            break;
        case NODE_OP:
            fprintf(ast_file, "OP: %s\n", node->name ? node->name : "");
            break;
        case NODE_NUM:
            fprintf(ast_file, "NUM: %d\n", node->value);
            break;
        case NODE_CALL:
            fprintf(ast_file, "CALL: %s\n", node->name ? node->name : "");
            break;
        case NODE_OUTPUT:
            fprintf(ast_file, "OUTPUT\n");
            break;
        case NODE_INPUT:
            fprintf(ast_file, "INPUT\n");
            break;
        case NODE_ARRAY_ACCESS:
            fprintf(ast_file, "ARRAY_ACCESS: %s\n", node->name ? node->name : "");
            break;
    }
    
    for (int i = 0; i < node->num_children; i++) {
        print_ast(node->children[i], depth + 1, ast_file);
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;
    for (int i = 0; i < node->num_children; i++) {
        free_ast(node->children[i]);
    }
    if (node->name) free(node->name);
    if (node->children) free(node->children);
    free(node);
}

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

void liberar_tabela() {
    for (int i = 0; i < n_simbolos; i++) {
        free(tabela[i].nome);
        free(tabela[i].tipo);
        free(tabela[i].scope);
    }
}
%}

%locations

%union {
    int ival;
    char *sval;
    char *tipo;
    int param_count;
    struct ASTNode* node;
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
%type <param_count> params param_list args arg_list
%type <node> program declaration_list declaration var_declaration fun_declaration
%type <node> compound_stmt local_declarations statement_list statement
%type <node> expression_stmt selection_stmt iteration_stmt return_stmt io_stmt call_stmt

%%

program:
    declaration_list { 
        if(busca("main", "global") == -1) {
            printf("Erro semantico: funcao 'main' nao declarada.\n");
        } else {
            printf("[ANALISE SINTATICA - OK]\n");
        }
        root = create_node(NODE_PROGRAM, NULL, 0, @1.first_line);
        add_child(root, $1);
        $$ = root;
    }
    ;

declaration_list:
      declaration_list declaration {
        $$ = $1;
        if ($2) add_child($$, $2);
      }
    | declaration {
        $$ = create_node(NODE_PROGRAM, "declarations", 0, @1.first_line);
        if ($1) add_child($$, $1);
      }
    ;

declaration:
      var_declaration { $$ = $1; }
    | fun_declaration { $$ = $1; }
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
        char decl_name[256];
        snprintf(decl_name, sizeof(decl_name), "%s: %s", $2, $1);
        $$ = create_node(NODE_VAR_DECL, decl_name, 0, @2.first_line);
        free($2);
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

            if (strcmp(escopo_atual, "global") != 0) {
                free(escopo_atual);
            }
            escopo_atual = strdup($2);
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

            char func_name[256];
            snprintf(func_name, sizeof(func_name), "%s: %s", $<sval>2, $1);
            $$ = create_node(NODE_FUN_DECL, func_name, 0, @2.first_line);
            if ($8) add_child($$, $8);

            if (strcmp(escopo_atual, "global") != 0) {
                free(escopo_atual);
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
          free($2);
      }
    | type_specifier ID LBRACKET RBRACKET {
          insere($2, $1, @2.first_line, escopo_atual, 0);
          free($2);
      }
    ;

type_specifier:
      INT  { $$ = "int"; }
    | VOID { $$ = "void"; }
    ;

compound_stmt:
      LBRACE local_declarations statement_list RBRACE {
        $$ = create_node(NODE_COMPOUND_STMT, NULL, 0, @1.first_line);
        if ($2) add_child($$, $2);
        if ($3) add_child($$, $3);
      }
    ;

local_declarations:
      local_declarations var_declaration {
        if (!$1) {
            $$ = create_node(NODE_COMPOUND_STMT, "local_vars", 0, @1.first_line);
        } else {
            $$ = $1;
        }
        if ($2) add_child($$, $2);
      }
    | /* vazio */ { $$ = NULL; }
    ;

statement_list:
      statement_list statement {
        if (!$1) {
            $$ = create_node(NODE_COMPOUND_STMT, "statements", 0, @1.first_line);
        } else {
            $$ = $1;
        }
        if ($2) add_child($$, $2);
      }
    | /* vazio */ { $$ = NULL; }
    ;

statement:
      expression_stmt { $$ = $1; }
    | compound_stmt { $$ = $1; }
    | selection_stmt { $$ = $1; }
    | iteration_stmt { $$ = $1; }
    | return_stmt {
        has_return = 1;
        $$ = $1;
    }
    | io_stmt { $$ = $1; }
    | call_stmt { $$ = $1; }  /* NOVO: chamada de função como statement */
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
          $$ = create_node(NODE_CALL, $1, 0, @1.first_line);
          free($1);
          /* NÃO verifica tipo void aqui - é permitido chamar função void como statement */
      }
    ;

/* Novos statements de I/O */
io_stmt:
      OUTPUT LPAREN expression RPAREN SEMI {
        $$ = create_node(NODE_OUTPUT, NULL, 0, @1.first_line);
      }
    ;

expression_stmt:
      expression SEMI {
        $$ = create_node(NODE_EXPRESSION, NULL, 0, @1.first_line);
      }
    | SEMI { $$ = NULL; }
    ;

selection_stmt:
      IF LPAREN expression RPAREN statement {
        $$ = create_node(NODE_IF_STMT, NULL, 0, @1.first_line);
        if ($5) add_child($$, $5);
      }
    | IF LPAREN expression RPAREN statement ELSE statement {
        $$ = create_node(NODE_IF_STMT, "if-else", 0, @1.first_line);
        if ($5) add_child($$, $5);
        if ($7) add_child($$, $7);
      }
    ;

iteration_stmt:
      WHILE LPAREN expression RPAREN statement {
        $$ = create_node(NODE_WHILE_STMT, NULL, 0, @1.first_line);
        if ($5) add_child($$, $5);
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
        $$ = create_node(NODE_RETURN_STMT, NULL, 0, @1.first_line);
     }
    | RETURN expression SEMI {
        $$ = create_node(NODE_RETURN_STMT, "with_value", 0, @1.first_line);
    }
    ;

/* Suporte a elementos de array no lado esquerdo da atribuição */
expression:
      ID ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
          free($1);
      }
    | ID LBRACKET expression RBRACKET ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
          free($1);
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
          } else if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", $1);
          }
          free($1);
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
          free($1);
    }
    | ID LBRACKET expression RBRACKET {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }

          if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", $1);
          }
          free($1);
    }
    | NUM
    | INPUT LPAREN RPAREN
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
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <entrada.c-> <saida.txt> <ast.txt>\n", argv[0]);
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

    // Imprime a Árvore Sintática Abstrata
    if (root) {
        FILE* ast_file = fopen(argv[3], "w");
        if (!ast_file) {
            perror("Erro ao abrir arquivo de AST");
        } else {
            fprintf(ast_file, "\n===== ARVORE SINTATICA ABSTRATA =====\n");
            print_ast(root, 0, ast_file);
            fprintf(ast_file, "======================================\n");
            fclose(ast_file);
            printf("Arvore sintatica abstrata salva em: %s\n", argv[3]);
        }
        free_ast(root);
    }

    liberar_tabela();
    if (strcmp(escopo_atual, "global") != 0) {
        free(escopo_atual);
    }

    fclose(yyin);
    fclose(out);
    return 0;
}