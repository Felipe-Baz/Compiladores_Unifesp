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

// Estrutura para a AST
typedef enum {
    NODE_PROG,
    NODE_FUNC,
    NODE_VAR,
    NODE_TYPE,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_ASSIGN,
    NODE_CALL,
    NODE_OUTPUT,
    NODE_INPUT,
    NODE_OP,
    NODE_ID,
    NODE_CONST
} NodeKind;

typedef struct TreeNode {
    NodeKind kind;
    char* name;
    char* op;
    int value;
    struct TreeNode** children;
    int child_count;
    int child_capacity;
} TreeNode;

TreeNode* root = NULL;

// Lista temporária para armazenar os nós dos parâmetros
TreeNode* temp_params[20];
int temp_params_count = 0;

// Lista temporária para armazenar os nós dos argumentos
TreeNode* temp_args[20];
int temp_args_count = 0;

TreeNode* createNode(NodeKind kind) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->kind = kind;
    node->name = NULL;
    node->op = NULL;
    node->value = 0;
    node->child_count = 0;
    node->child_capacity = 4;
    node->children = (TreeNode**)malloc(sizeof(TreeNode*) * node->child_capacity);
    return node;
}

void addChild(TreeNode* parent, TreeNode* child) {
    if (parent == NULL || child == NULL) return;
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (TreeNode**)realloc(parent->children, sizeof(TreeNode*) * parent->child_capacity);
    }
    parent->children[parent->child_count++] = child;
}

void printAST(TreeNode* node, int depth) {
    if (node == NULL) return;
    
    for (int i = 0; i < depth; i++) {
        fprintf(out, "----");
    }
    
    switch (node->kind) {
        case NODE_PROG:
            fprintf(out, "Stmt: Prog\n");
            break;
        case NODE_FUNC:
            fprintf(out, "Stmt: Func: %s\n", node->name);
            break;
        case NODE_VAR:
            fprintf(out, "Stmt: Var\n");
            break;
        case NODE_TYPE:
            fprintf(out, "Exp: Type: %s\n", node->name);
            break;
        case NODE_IF:
            fprintf(out, "Stmt: If\n");
            break;
        case NODE_WHILE:
            fprintf(out, "Stmt: While\n");
            break;
        case NODE_RETURN:
            fprintf(out, "Stmt: Return: %s\n", node->name);
            break;
        case NODE_ASSIGN:
            fprintf(out, "Stmt: Assign\n");
            break;
        case NODE_CALL:
            fprintf(out, "Stmt: Call\n");
            break;
        case NODE_OUTPUT:
            fprintf(out, "Stmt: Output\n");
            break;
        case NODE_INPUT:
            fprintf(out, "Stmt: Input\n");
            break;
        case NODE_OP:
            fprintf(out, "Exp: Op: %s\n", node->op);
            break;
        case NODE_ID:
            fprintf(out, "Exp: Id: %s\n", node->name);
            break;
        case NODE_CONST:
            fprintf(out, "Exp: Const: %d\n", node->value);
            break;
    }
    
    for (int i = 0; i < node->child_count; i++) {
        printAST(node->children[i], depth + 1);
    }
}

void freeAST(TreeNode* node) {
    if (node == NULL) return;
    for (int i = 0; i < node->child_count; i++) {
        freeAST(node->children[i]);
    }
    if (node->name) free(node->name);
    if (node->op) free(node->op);
    free(node->children);
    free(node);
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

void liberar_tabela() {
    for (int i = 0; i < n_simbolos; i++) {
        free(tabela[i].nome);
        free(tabela[i].tipo);
        free(tabela[i].scope);
    }
}

void imprimir_tabela_simbolos(FILE* arquivo) {
    fprintf(arquivo, "=== TABELA DE SIMBOLOS ===\n\n");
    fprintf(arquivo, "%-20s %-10s %-10s %-15s %-10s\n", "Nome", "Tipo", "Linha", "Escopo", "Parametros");
    fprintf(arquivo, "--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < n_simbolos; i++) {
        fprintf(arquivo, "%-20s %-10s %-10d %-15s %-10d\n", 
                tabela[i].nome, 
                tabela[i].tipo, 
                tabela[i].linha, 
                tabela[i].scope,
                tabela[i].param_counter);
    }
    
    fprintf(arquivo, "\nTotal de simbolos: %d\n", n_simbolos);
}
%}

%locations

%union {
    int ival;
    char *sval;
    char *tipo;
    int param_count;
    struct TreeNode* node;
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
%type <node> program declaration_list declaration var_declaration fun_declaration
%type <node> compound_stmt statement_list statement expression_stmt selection_stmt
%type <node> iteration_stmt return_stmt io_stmt call_stmt
%type <node> expression simple_expression additive_expression term factor
%type <node> local_declarations param addop mulop
%type <param_count> params param_list args arg_list

%%

program:
    declaration_list { 
        if(busca("main", "global") == -1) {
            printf("Erro semantico: funcao 'main' nao declarada.\n");
        } else {
            printf("[ANALISE SINTATICA - OK]\n"); 
        }
        root = createNode(NODE_PROG);
        addChild(root, $1);
        $$ = root;
    }
    ;

declaration_list:
      declaration_list declaration {
        $$ = $1;
        addChild($$, $2);
      }
    | declaration {
        $$ = $1;
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
        
        TreeNode* typeNode = createNode(NODE_TYPE);
        typeNode->name = strdup($1);
        TreeNode* varNode = createNode(NODE_VAR);
        addChild(typeNode, varNode);
        $$ = typeNode;
        
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
            temp_params_count = 0;  // Reset da lista de parâmetros
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

            TreeNode* typeNode = createNode(NODE_TYPE);
            typeNode->name = strdup($1);
            TreeNode* funcNode = createNode(NODE_FUNC);
            funcNode->name = strdup($<sval>2);
            addChild(typeNode, funcNode);
            
            // Adiciona os parâmetros ao nó da função
            for (int i = 0; i < temp_params_count; i++) {
                addChild(funcNode, temp_params[i]);
            }
            
            addChild(funcNode, $8);
            $$ = typeNode;

            if (strcmp(escopo_atual, "global") != 0) {
                free(escopo_atual);
            }
            escopo_atual = "global";
            has_return = 0;
            temp_params_count = 0;  // Limpa a lista de parâmetros
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
          
          TreeNode* typeNode = createNode(NODE_TYPE);
          typeNode->name = strdup($1);
          TreeNode* varNode = createNode(NODE_VAR);
          addChild(typeNode, varNode);
          $$ = typeNode;
          
          // Adiciona à lista temporária de parâmetros
          temp_params[temp_params_count++] = typeNode;
          
          free($2);
      }
    | type_specifier ID LBRACKET RBRACKET {
          insere($2, $1, @2.first_line, escopo_atual, 0);
          
          TreeNode* typeNode = createNode(NODE_TYPE);
          typeNode->name = strdup($1);
          TreeNode* varNode = createNode(NODE_VAR);
          addChild(typeNode, varNode);
          $$ = typeNode;
          
          // Adiciona à lista temporária de parâmetros
          temp_params[temp_params_count++] = typeNode;
          
          free($2);
      }
    ;

type_specifier:
      INT  { $$ = "int"; }
    | VOID { $$ = "void"; }
    ;

compound_stmt:
      LBRACE local_declarations statement_list RBRACE {
        // Cria um nó container para o compound statement
        TreeNode* compoundNode = createNode(NODE_PROG);
        
        // Adiciona todas as declarações locais primeiro
        if ($2 != NULL) {
            for (int i = 0; i < $2->child_count; i++) {
                addChild(compoundNode, $2->children[i]);
            }
        }
        
        // Adiciona todos os statements
        if ($3 != NULL) {
            for (int i = 0; i < $3->child_count; i++) {
                addChild(compoundNode, $3->children[i]);
            }
        }
        
        $$ = compoundNode;
      }
    ;

local_declarations:
      local_declarations var_declaration {
        if ($1 == NULL) {
            $$ = createNode(NODE_PROG);
            addChild($$, $2);
        } else {
            $$ = $1;
            addChild($$, $2);
        }
      }
    | /* vazio */ { $$ = NULL; }
    ;

statement_list:
      statement_list statement {
        if ($1 == NULL) {
            $$ = createNode(NODE_PROG);
        } else {
            $$ = $1;
        }
        if ($2 != NULL) {
            addChild($$, $2);
        }
      }
    | /* vazio */ { $$ = createNode(NODE_PROG); }
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
    | call_stmt { $$ = $1; }
    ;

/* NOVO: Chamada de função como statement (sem uso do retorno) */
call_stmt:
      ID LPAREN { temp_args_count = 0; } args RPAREN SEMI {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: funcao '%s' nao declarada.\n", $1);
          } else {
            if (tabela[idx].param_counter != $4) {
                  printf("Erro semantico: funcao '%s' espera %d parametro(s), mas recebeu %d argumento(s).\n", 
                         $1, tabela[idx].param_counter, $4);
              } else {
                if(tabela[idx].tipo == "int") {
                    printf("Aviso semantico: retorno da funcao '%s' do tipo int nao utilizado.\n", $1);
                }
              }
          }
          
          TreeNode* callNode = createNode(NODE_CALL);
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup($1);
          addChild(callNode, idNode);
          
          // Adiciona os argumentos ao nó da chamada
          for (int i = 0; i < temp_args_count; i++) {
              addChild(callNode, temp_args[i]);
          }
          
          $$ = callNode;
          temp_args_count = 0;
          
          free($1);
      }
    ;

/* Novos statements de I/O */
io_stmt:
      OUTPUT LPAREN expression RPAREN SEMI {
        TreeNode* outputNode = createNode(NODE_OUTPUT);
        addChild(outputNode, $3);
        $$ = outputNode;
      }
    ;

expression_stmt:
      expression SEMI { $$ = $1; }
    | SEMI { $$ = NULL; }
    ;

selection_stmt:
      IF LPAREN expression RPAREN statement {
        TreeNode* ifNode = createNode(NODE_IF);
        addChild(ifNode, $3);
        addChild(ifNode, $5);
        $$ = ifNode;
      }
    | IF LPAREN expression RPAREN statement ELSE statement {
        TreeNode* ifNode = createNode(NODE_IF);
        addChild(ifNode, $3);
        addChild(ifNode, $5);
        addChild(ifNode, $7);
        $$ = ifNode;
      }
    ;

iteration_stmt:
      WHILE LPAREN expression RPAREN statement {
        TreeNode* whileNode = createNode(NODE_WHILE);
        addChild(whileNode, $3);
        addChild(whileNode, $5);
        $$ = whileNode;
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
        
        TreeNode* returnNode = createNode(NODE_RETURN);
        returnNode->name = strdup("void");
        $$ = returnNode;
     }
    | RETURN expression SEMI {
        int id = busca(escopo_atual, "global");
        TreeNode* returnNode = createNode(NODE_RETURN);
        if (id != -1) {
            returnNode->name = strdup(tabela[id].tipo);
        } else {
            returnNode->name = strdup("unknown");
        }
        addChild(returnNode, $2);
        $$ = returnNode;
    }
    ;

/* Suporte a elementos de array no lado esquerdo da atribuição */
expression:
      ID ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
          
          TreeNode* assignNode = createNode(NODE_ASSIGN);
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup($1);
          addChild(assignNode, idNode);
          addChild(assignNode, $3);
          $$ = assignNode;
          
          free($1);
      }
    | ID LBRACKET expression RBRACKET ASSIGN expression {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          }
          
          TreeNode* assignNode = createNode(NODE_ASSIGN);
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup($1);
          addChild(assignNode, idNode);
          addChild(assignNode, $3);
          addChild(assignNode, $6);
          $$ = assignNode;
          
          free($1);
    }
    | simple_expression { $$ = $1; }
    ;

simple_expression:
      additive_expression relop additive_expression {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("COMP");
        addChild(opNode, $1);
        addChild(opNode, $3);
        $$ = opNode;
      }
    | additive_expression { $$ = $1; }
    ;

relop:
      LEQ | LT | GT | GEQ | EQ | NEQ
    ;

additive_expression:
      additive_expression addop term {
        $$ = $2;
        addChild($$, $1);
        addChild($$, $3);
      }
    | term { $$ = $1; }
    ;

addop:
      PLUS {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("ADD");
        $$ = opNode;
      }
    | MINUS {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("SUB");
        $$ = opNode;
      }
    ;

term:
      term mulop factor {
        $$ = $2;
        addChild($$, $1);
        addChild($$, $3);
      }
    | factor { $$ = $1; }
    ;

mulop:
      TIMES {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("MULT");
        $$ = opNode;
      }
    | DIVIDE {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("DIV");
        $$ = opNode;
      }
    ;

factor:
      LPAREN expression RPAREN { $$ = $2; }
    | ID {
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", $1);
          } else if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", $1);
          }
          
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup($1);
          $$ = idNode;
          
          free($1);
    }
    | ID LPAREN { temp_args_count = 0; } args RPAREN {
          /* CHAMADA DE FUNÇÃO DENTRO DE EXPRESSÃO - retorno é usado */
          int idx = busca($1, escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: funcao '%s' nao declarada.\n", $1);
          } else if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", $1);
          } else {
              // Verifica número de argumentos
              if (tabela[idx].param_counter != $4) {
                  printf("Erro semantico: funcao '%s' espera %d parametro(s), mas recebeu %d argumento(s).\n", 
                         $1, tabela[idx].param_counter, $4);
              }
          }
          
          TreeNode* callNode = createNode(NODE_CALL);
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup($1);
          addChild(callNode, idNode);
          
          // Adiciona os argumentos ao nó da chamada
          for (int i = 0; i < temp_args_count; i++) {
              addChild(callNode, temp_args[i]);
          }
          
          $$ = callNode;
          temp_args_count = 0;
          
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
          
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup($1);
          $$ = idNode;
          
          free($1);
    }
    | NUM {
        TreeNode* constNode = createNode(NODE_CONST);
        constNode->value = $1;
        $$ = constNode;
    }
    | INPUT LPAREN RPAREN {
        TreeNode* inputNode = createNode(NODE_INPUT);
        $$ = inputNode;
    }
    ;

args:
      arg_list { $$ = $1; }
    | /* vazio */ { $$ = 0; }
    ;

arg_list:
      arg_list COMMA expression { 
        $$ = $1 + 1; 
        temp_args[temp_args_count++] = $3;
      }
    | expression { 
        $$ = 1; 
        temp_args[temp_args_count++] = $1;
      }
    ;

%%

void yyerror(const char *s) {
    printf("\n[ERRO SINTATICO na linha %d]: %s\n", yylloc.first_line, s);
}

int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <entrada.c-> <saida.txt> <arvore.txt> <tabela.txt>\n", argv[0]);
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

    fclose(out);

    // Imprimir a AST no arquivo separado
    FILE* astFile = fopen(argv[3], "w");
    if (!astFile) {
        perror("Erro ao abrir arquivo de AST");
        fclose(yyin);
        return 1;
    }

    if (root != NULL) {
        // Temporariamente redireciona out para o arquivo da AST
        out = astFile;
        printAST(root, 0);
        freeAST(root);
    }

    fclose(astFile);

    // Imprimir a tabela de símbolos no arquivo separado
    FILE* tabelaFile = fopen(argv[4], "w");
    if (!tabelaFile) {
        perror("Erro ao abrir arquivo de tabela de simbolos");
        fclose(yyin);
        return 1;
    }

    imprimir_tabela_simbolos(tabelaFile);
    fclose(tabelaFile);

    liberar_tabela();
    if (strcmp(escopo_atual, "global") != 0) {
        free(escopo_atual);
    }

    fclose(yyin);
    return 0;
}