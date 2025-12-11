
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "parser.y"

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
            if (node->name != NULL) {
                fprintf(out, "Stmt: Var: Id: %s\n", node->name);
            } else {
                fprintf(out, "Stmt: Var\n");
            }
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


/* Line 189 of yacc.c  */
#line 285 "parser.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IF = 258,
     ELSE = 259,
     WHILE = 260,
     RETURN = 261,
     INT = 262,
     VOID = 263,
     OUTPUT = 264,
     INPUT = 265,
     EQ = 266,
     NEQ = 267,
     LEQ = 268,
     GEQ = 269,
     LT = 270,
     GT = 271,
     ASSIGN = 272,
     PLUS = 273,
     MINUS = 274,
     TIMES = 275,
     DIVIDE = 276,
     SEMI = 277,
     COMMA = 278,
     LPAREN = 279,
     RPAREN = 280,
     LBRACE = 281,
     RBRACE = 282,
     LBRACKET = 283,
     RBRACKET = 284,
     ID = 285,
     NUM = 286
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 214 "parser.y"

    int ival;
    char *sval;
    char *tipo;
    int param_count;
    struct TreeNode* node;



/* Line 214 of yacc.c  */
#line 362 "parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 387 "parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   122

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNRULES -- Number of states.  */
#define YYNSTATES  124

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    14,    19,    20,
      24,    25,    26,    35,    37,    39,    43,    45,    48,    53,
      55,    57,    62,    65,    66,    69,    70,    72,    74,    76,
      78,    80,    82,    84,    85,    92,    98,   101,   103,   109,
     117,   123,   126,   130,   134,   141,   143,   147,   149,   151,
     153,   155,   157,   159,   161,   165,   167,   169,   171,   175,
     177,   179,   181,   185,   187,   188,   194,   199,   201,   205,
     207,   208,   212
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      33,     0,    -1,    34,    -1,    34,    35,    -1,    35,    -1,
      36,    -1,    38,    -1,    44,    30,    37,    22,    -1,    -1,
      28,    31,    29,    -1,    -1,    -1,    44,    30,    24,    39,
      41,    40,    25,    45,    -1,    42,    -1,     8,    -1,    42,
      23,    43,    -1,    43,    -1,    44,    30,    -1,    44,    30,
      28,    29,    -1,     7,    -1,     8,    -1,    26,    46,    47,
      27,    -1,    46,    36,    -1,    -1,    47,    48,    -1,    -1,
      52,    -1,    45,    -1,    53,    -1,    54,    -1,    55,    -1,
      51,    -1,    49,    -1,    -1,    30,    24,    50,    65,    25,
      22,    -1,     9,    24,    56,    25,    22,    -1,    56,    22,
      -1,    22,    -1,     3,    24,    56,    25,    48,    -1,     3,
      24,    56,    25,    48,     4,    48,    -1,     5,    24,    56,
      25,    48,    -1,     6,    22,    -1,     6,    56,    22,    -1,
      30,    17,    56,    -1,    30,    28,    56,    29,    17,    56,
      -1,    57,    -1,    59,    58,    59,    -1,    59,    -1,    13,
      -1,    15,    -1,    16,    -1,    14,    -1,    11,    -1,    12,
      -1,    59,    60,    61,    -1,    61,    -1,    18,    -1,    19,
      -1,    61,    62,    63,    -1,    63,    -1,    20,    -1,    21,
      -1,    24,    56,    25,    -1,    30,    -1,    -1,    30,    24,
      64,    65,    25,    -1,    30,    28,    56,    29,    -1,    31,
      -1,    10,    24,    25,    -1,    66,    -1,    -1,    66,    23,
      56,    -1,    56,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   242,   242,   254,   262,   270,   271,   276,   306,   307,
     315,   330,   315,   370,   371,   375,   376,   380,   404,   425,
     426,   430,   453,   462,   466,   476,   480,   481,   482,   483,
     484,   488,   489,   494,   494,   528,   536,   537,   541,   547,
     557,   566,   578,   593,   608,   627,   631,   636,   640,   645,
     650,   655,   660,   665,   673,   678,   682,   687,   695,   700,
     704,   709,   717,   718,   732,   732,   762,   782,   787,   794,
     795,   799,   803
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IF", "ELSE", "WHILE", "RETURN", "INT",
  "VOID", "OUTPUT", "INPUT", "EQ", "NEQ", "LEQ", "GEQ", "LT", "GT",
  "ASSIGN", "PLUS", "MINUS", "TIMES", "DIVIDE", "SEMI", "COMMA", "LPAREN",
  "RPAREN", "LBRACE", "RBRACE", "LBRACKET", "RBRACKET", "ID", "NUM",
  "$accept", "program", "declaration_list", "declaration",
  "var_declaration", "array_spec", "fun_declaration", "$@1", "$@2",
  "params", "param_list", "param", "type_specifier", "compound_stmt",
  "local_declarations", "statement_list", "statement", "call_stmt", "$@3",
  "io_stmt", "expression_stmt", "selection_stmt", "iteration_stmt",
  "return_stmt", "expression", "simple_expression", "relop",
  "additive_expression", "addop", "term", "mulop", "factor", "$@4", "args",
  "arg_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    32,    33,    34,    34,    35,    35,    36,    37,    37,
      39,    40,    38,    41,    41,    42,    42,    43,    43,    44,
      44,    45,    46,    46,    47,    47,    48,    48,    48,    48,
      48,    48,    48,    50,    49,    51,    52,    52,    53,    53,
      54,    55,    55,    56,    56,    56,    57,    57,    58,    58,
      58,    58,    58,    58,    59,    59,    60,    60,    61,    61,
      62,    62,    63,    63,    64,    63,    63,    63,    63,    65,
      65,    66,    66
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     4,     0,     3,
       0,     0,     8,     1,     1,     3,     1,     2,     4,     1,
       1,     4,     2,     0,     2,     0,     1,     1,     1,     1,
       1,     1,     1,     0,     6,     5,     2,     1,     5,     7,
       5,     2,     3,     3,     6,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     0,     5,     4,     1,     3,     1,
       0,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    19,    20,     0,     2,     4,     5,     6,     0,     1,
       3,     8,    10,     0,     0,     0,     0,     7,    14,    11,
      13,    16,     0,     9,     0,     0,    17,     0,    15,     0,
      23,    12,    18,    25,    22,     0,     0,     8,     0,     0,
       0,     0,     0,    37,     0,    21,    63,    67,    27,    24,
      32,    31,    26,    28,    29,    30,     0,    45,    47,    55,
      59,     0,     0,    41,    63,     0,     0,     0,     0,     0,
      33,     0,    36,    52,    53,    48,    51,    49,    50,    56,
      57,     0,     0,    60,    61,     0,     0,     0,    64,    42,
       0,    68,    62,    43,    70,    70,     0,    63,    46,    54,
      58,     0,     0,     0,    72,     0,    69,     0,    66,     0,
      38,    40,    35,     0,     0,    65,     0,     0,     0,    34,
      71,    44,    66,    39
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    14,     7,    15,    24,    19,
      20,    21,     8,    48,    33,    36,    49,    50,    94,    51,
      52,    53,    54,    55,    56,    57,    81,    58,    82,    59,
      85,    60,    95,   105,   106
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -101
static const yytype_int8 yypact[] =
{
       0,  -101,  -101,     5,     0,  -101,  -101,  -101,    -3,  -101,
    -101,   -18,  -101,     3,    17,    12,    19,  -101,    16,  -101,
      29,  -101,    30,  -101,    43,     0,    45,    49,  -101,    57,
    -101,  -101,  -101,     0,  -101,    47,     6,    61,    68,    71,
      40,    72,    73,  -101,    -7,  -101,    25,  -101,  -101,  -101,
    -101,  -101,  -101,  -101,  -101,  -101,    76,  -101,    69,    70,
    -101,    -7,    -7,  -101,    39,    77,    -7,    75,    78,    -7,
    -101,    -7,  -101,  -101,  -101,  -101,  -101,  -101,  -101,  -101,
    -101,    48,    48,  -101,  -101,    48,    79,    80,  -101,  -101,
      81,  -101,  -101,  -101,    -7,    -7,    64,    23,    -5,    70,
    -101,    35,    35,    85,  -101,    83,    86,    87,    84,    -7,
      90,  -101,  -101,    88,    -7,  -101,    -7,    82,    35,  -101,
    -101,  -101,  -101,  -101
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -101,  -101,  -101,    98,    89,  -101,  -101,  -101,  -101,  -101,
    -101,    91,    10,    92,  -101,  -101,  -100,  -101,  -101,  -101,
    -101,  -101,  -101,  -101,   -40,  -101,  -101,    32,  -101,    33,
    -101,    36,  -101,    22,  -101
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -21
static const yytype_int8 yytable[] =
{
      65,   110,   111,    42,    68,     9,    12,     1,     2,    38,
      13,    39,    40,    79,    80,    41,    42,    44,   123,     1,
      18,    86,    87,    64,    47,    22,    90,    11,    43,    93,
      44,    96,    30,    45,    16,    22,    46,    47,    38,    17,
      39,    40,    69,    35,    41,    42,   -20,    88,    23,    70,
      42,   109,    25,    71,   104,   104,    69,    43,    42,    44,
      26,    30,    63,    88,    44,    46,    47,    71,    27,   117,
      64,    47,    44,    29,   120,    30,   121,    37,    97,    47,
      73,    74,    75,    76,    77,    78,    32,    79,    80,    13,
      83,    84,    61,   108,   118,    62,    66,    67,    72,    89,
      91,   116,    10,    92,   101,   102,   103,   112,   113,   114,
     119,   122,   115,    98,     0,    99,    28,   107,     0,    31,
       0,   100,    34
};

static const yytype_int8 yycheck[] =
{
      40,   101,   102,    10,    44,     0,    24,     7,     8,     3,
      28,     5,     6,    18,    19,     9,    10,    24,   118,     7,
       8,    61,    62,    30,    31,    15,    66,    30,    22,    69,
      24,    71,    26,    27,    31,    25,    30,    31,     3,    22,
       5,     6,    17,    33,     9,    10,    30,    24,    29,    24,
      10,    28,    23,    28,    94,    95,    17,    22,    10,    24,
      30,    26,    22,    24,    24,    30,    31,    28,    25,   109,
      30,    31,    24,    28,   114,    26,   116,    30,    30,    31,
      11,    12,    13,    14,    15,    16,    29,    18,    19,    28,
      20,    21,    24,    29,     4,    24,    24,    24,    22,    22,
      25,    17,     4,    25,    25,    25,    25,    22,    25,    23,
      22,    29,    25,    81,    -1,    82,    25,    95,    -1,    27,
      -1,    85,    33
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,     8,    33,    34,    35,    36,    38,    44,     0,
      35,    30,    24,    28,    37,    39,    31,    22,     8,    41,
      42,    43,    44,    29,    40,    23,    30,    25,    43,    28,
      26,    45,    29,    46,    36,    44,    47,    30,     3,     5,
       6,     9,    10,    22,    24,    27,    30,    31,    45,    48,
      49,    51,    52,    53,    54,    55,    56,    57,    59,    61,
      63,    24,    24,    22,    30,    56,    24,    24,    56,    17,
      24,    28,    22,    11,    12,    13,    14,    15,    16,    18,
      19,    58,    60,    20,    21,    62,    56,    56,    24,    22,
      56,    25,    25,    56,    50,    64,    56,    30,    59,    61,
      63,    25,    25,    25,    56,    65,    66,    65,    29,    28,
      48,    48,    22,    25,    23,    25,    17,    56,     4,    22,
      56,    56,    29,    48
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 242 "parser.y"
    { 
        if(busca("main", "global") == -1) {
            printf("Erro semantico: funcao 'main' nao declarada.\n");
        } else {
            printf("[ANALISE SINTATICA - OK]\n"); 
        }
        root = (yyvsp[(1) - (1)].node);
        (yyval.node) = root;
    ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 254 "parser.y"
    {
        if ((yyvsp[(1) - (2)].node) == NULL) {
            (yyval.node) = createNode(NODE_PROG);
        } else {
            (yyval.node) = (yyvsp[(1) - (2)].node);
        }
        addChild((yyval.node), (yyvsp[(2) - (2)].node));
      ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 262 "parser.y"
    {
        TreeNode* listNode = createNode(NODE_PROG);
        addChild(listNode, (yyvsp[(1) - (1)].node));
        (yyval.node) = listNode;
      ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 270 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 271 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 276 "parser.y"
    {
        int id = busca((yyvsp[(2) - (4)].sval), escopo_atual);
        if (id != -1) {
            printf("Erro semantico: variavel '%s' ja declarada.\n", (yyvsp[(2) - (4)].sval));
        } else {
            if (strcmp((yyvsp[(1) - (4)].tipo), "void") == 0) {
                printf("Erro semantico: variavel '%s' nao pode ser do tipo void.\n", (yyvsp[(2) - (4)].sval));
            } else {
                insere((yyvsp[(2) - (4)].sval), (yyvsp[(1) - (4)].tipo), (yylsp[(2) - (4)]).first_line, escopo_atual, 0);
            }
        }
        
        TreeNode* typeNode = createNode(NODE_TYPE);
        typeNode->name = strdup((yyvsp[(1) - (4)].tipo));
        TreeNode* varNode = createNode(NODE_VAR);
        varNode->name = strdup((yyvsp[(2) - (4)].sval));
        
        // Se for array, adiciona o tamanho como filho do nó VAR
        if ((yyvsp[(3) - (4)].node) != NULL) {
            addChild(varNode, (yyvsp[(3) - (4)].node));
        }
        
        addChild(typeNode, varNode);
        (yyval.node) = typeNode;
        
        free((yyvsp[(2) - (4)].sval));
      ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 306 "parser.y"
    { (yyval.node) = NULL; ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 307 "parser.y"
    {
        TreeNode* sizeNode = createNode(NODE_CONST);
        sizeNode->value = (yyvsp[(2) - (3)].ival);
        (yyval.node) = sizeNode;
      ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 315 "parser.y"
    {
            int id = busca((yyvsp[(2) - (3)].sval), escopo_atual);

            if (id != -1) {
                printf("Erro semantico: funcao '%s' ja declarada.\n", (yyvsp[(2) - (3)].sval));
            } else {
                insere((yyvsp[(2) - (3)].sval), (yyvsp[(1) - (3)].tipo), (yylsp[(2) - (3)]).first_line, escopo_atual, 0);
            }

            if (strcmp(escopo_atual, "global") != 0) {
                free(escopo_atual);
            }
            escopo_atual = strdup((yyvsp[(2) - (3)].sval));
            temp_params_count = 0;  // Reset da lista de parâmetros
      ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 330 "parser.y"
    {
            int idx = busca((yyvsp[(2) - (5)].sval), "global");
            if (idx != -1) {
                tabela[idx].param_counter = (yyvsp[(5) - (5)].param_count);
            }
      ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 337 "parser.y"
    {
            int idx = busca(escopo_atual, "global");
            if (idx != -1) {
                if (strcmp(tabela[idx].tipo, "int") == 0 && has_return == 0) {
                    printf("Erro semantico: funcao '%s' do tipo int deve ter um return.\n", escopo_atual);
                }
            }

            TreeNode* typeNode = createNode(NODE_TYPE);
            typeNode->name = strdup((yyvsp[(1) - (8)].tipo));
            TreeNode* funcNode = createNode(NODE_FUNC);
            funcNode->name = strdup((yyvsp[(2) - (8)].sval));
            addChild(typeNode, funcNode);
            
            // Adiciona os parâmetros ao nó da função
            for (int i = 0; i < temp_params_count; i++) {
                addChild(funcNode, temp_params[i]);
            }
            
            addChild(funcNode, (yyvsp[(8) - (8)].node));
            (yyval.node) = typeNode;

            if (strcmp(escopo_atual, "global") != 0) {
                free(escopo_atual);
            }
            escopo_atual = "global";
            has_return = 0;
            temp_params_count = 0;  // Limpa a lista de parâmetros
      ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 370 "parser.y"
    { (yyval.param_count) = (yyvsp[(1) - (1)].param_count); ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 371 "parser.y"
    { (yyval.param_count) = 0; ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 375 "parser.y"
    { (yyval.param_count) = (yyvsp[(1) - (3)].param_count) + 1; ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 376 "parser.y"
    { (yyval.param_count) = 1; ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 380 "parser.y"
    {
          int id = busca((yyvsp[(2) - (2)].sval), escopo_atual);
          if (id != -1) {
              printf("Erro semantico: variavel '%s' ja declarada.\n", (yyvsp[(2) - (2)].sval));
          } else {
              if (strcmp((yyvsp[(1) - (2)].tipo), "void") == 0) {
                  printf("Erro semantico: parametro '%s' nao pode ser do tipo void.\n", (yyvsp[(2) - (2)].sval));
              } else {
                  insere((yyvsp[(2) - (2)].sval), (yyvsp[(1) - (2)].tipo), (yylsp[(2) - (2)]).first_line, escopo_atual, 0);
              }
          }
          
          TreeNode* typeNode = createNode(NODE_TYPE);
          typeNode->name = strdup((yyvsp[(1) - (2)].tipo));
          TreeNode* varNode = createNode(NODE_VAR);
          varNode->name = strdup((yyvsp[(2) - (2)].sval));
          addChild(typeNode, varNode);
          (yyval.node) = typeNode;
          
          // Adiciona à lista temporária de parâmetros
          temp_params[temp_params_count++] = typeNode;
          
          free((yyvsp[(2) - (2)].sval));
      ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 404 "parser.y"
    {
          insere((yyvsp[(2) - (4)].sval), (yyvsp[(1) - (4)].tipo), (yylsp[(2) - (4)]).first_line, escopo_atual, 0);
          
          TreeNode* typeNode = createNode(NODE_TYPE);
          typeNode->name = strdup((yyvsp[(1) - (4)].tipo));
          TreeNode* varNode = createNode(NODE_VAR);
          // Adiciona [] ao nome para indicar que é um parâmetro array
          char* arrayName = (char*)malloc(strlen((yyvsp[(2) - (4)].sval)) + 3);
          sprintf(arrayName, "%s[]", (yyvsp[(2) - (4)].sval));
          varNode->name = arrayName;
          addChild(typeNode, varNode);
          (yyval.node) = typeNode;
          
          // Adiciona à lista temporária de parâmetros
          temp_params[temp_params_count++] = typeNode;
          
          free((yyvsp[(2) - (4)].sval));
      ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 425 "parser.y"
    { (yyval.tipo) = "int"; ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 426 "parser.y"
    { (yyval.tipo) = "void"; ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 430 "parser.y"
    {
        // Cria um nó container para o compound statement
        TreeNode* compoundNode = createNode(NODE_PROG);
        
        // Adiciona todas as declarações locais primeiro
        if ((yyvsp[(2) - (4)].node) != NULL) {
            for (int i = 0; i < (yyvsp[(2) - (4)].node)->child_count; i++) {
                addChild(compoundNode, (yyvsp[(2) - (4)].node)->children[i]);
            }
        }
        
        // Adiciona todos os statements
        if ((yyvsp[(3) - (4)].node) != NULL) {
            for (int i = 0; i < (yyvsp[(3) - (4)].node)->child_count; i++) {
                addChild(compoundNode, (yyvsp[(3) - (4)].node)->children[i]);
            }
        }
        
        (yyval.node) = compoundNode;
      ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 453 "parser.y"
    {
        if ((yyvsp[(1) - (2)].node) == NULL) {
            (yyval.node) = createNode(NODE_PROG);
            addChild((yyval.node), (yyvsp[(2) - (2)].node));
        } else {
            (yyval.node) = (yyvsp[(1) - (2)].node);
            addChild((yyval.node), (yyvsp[(2) - (2)].node));
        }
      ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 462 "parser.y"
    { (yyval.node) = NULL; ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 466 "parser.y"
    {
        if ((yyvsp[(1) - (2)].node) == NULL) {
            (yyval.node) = createNode(NODE_PROG);
        } else {
            (yyval.node) = (yyvsp[(1) - (2)].node);
        }
        if ((yyvsp[(2) - (2)].node) != NULL) {
            addChild((yyval.node), (yyvsp[(2) - (2)].node));
        }
      ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 476 "parser.y"
    { (yyval.node) = createNode(NODE_PROG); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 480 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 481 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 482 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 483 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 484 "parser.y"
    {
        has_return = 1;
        (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 488 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 489 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 494 "parser.y"
    { temp_args_count = 0; ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 494 "parser.y"
    {
          int idx = busca((yyvsp[(1) - (6)].sval), escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: funcao '%s' nao declarada.\n", (yyvsp[(1) - (6)].sval));
          } else {
            if (tabela[idx].param_counter != (yyvsp[(4) - (6)].param_count)) {
                  printf("Erro semantico: funcao '%s' espera %d parametro(s), mas recebeu %d argumento(s).\n", 
                         (yyvsp[(1) - (6)].sval), tabela[idx].param_counter, (yyvsp[(4) - (6)].param_count));
              } else {
                if(tabela[idx].tipo == "int") {
                    printf("Aviso semantico: retorno da funcao '%s' do tipo int nao utilizado.\n", (yyvsp[(1) - (6)].sval));
                }
              }
          }
          
          TreeNode* callNode = createNode(NODE_CALL);
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup((yyvsp[(1) - (6)].sval));
          
          // Adiciona os argumentos como filhos do ID da função
          for (int i = 0; i < temp_args_count; i++) {
              addChild(idNode, temp_args[i]);
          }
          
          addChild(callNode, idNode);
          (yyval.node) = callNode;
          temp_args_count = 0;
          
          free((yyvsp[(1) - (6)].sval));
      ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 528 "parser.y"
    {
        TreeNode* outputNode = createNode(NODE_OUTPUT);
        addChild(outputNode, (yyvsp[(3) - (5)].node));
        (yyval.node) = outputNode;
      ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 536 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 537 "parser.y"
    { (yyval.node) = NULL; ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 541 "parser.y"
    {
        TreeNode* ifNode = createNode(NODE_IF);
        addChild(ifNode, (yyvsp[(3) - (5)].node));
        addChild(ifNode, (yyvsp[(5) - (5)].node));
        (yyval.node) = ifNode;
      ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 547 "parser.y"
    {
        TreeNode* ifNode = createNode(NODE_IF);
        addChild(ifNode, (yyvsp[(3) - (7)].node));
        addChild(ifNode, (yyvsp[(5) - (7)].node));
        addChild(ifNode, (yyvsp[(7) - (7)].node));
        (yyval.node) = ifNode;
      ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 557 "parser.y"
    {
        TreeNode* whileNode = createNode(NODE_WHILE);
        addChild(whileNode, (yyvsp[(3) - (5)].node));
        addChild(whileNode, (yyvsp[(5) - (5)].node));
        (yyval.node) = whileNode;
      ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 566 "parser.y"
    { 
        int id = busca(escopo_atual, "global");
        if (id != -1) {
            if (strcmp(tabela[id].tipo, "int") == 0) {
                printf("Erro semantico: funcao '%s' do tipo int tem que retornar valor.\n", escopo_atual);
            }
        }
        
        TreeNode* returnNode = createNode(NODE_RETURN);
        returnNode->name = strdup("void");
        (yyval.node) = returnNode;
     ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 578 "parser.y"
    {
        int id = busca(escopo_atual, "global");
        TreeNode* returnNode = createNode(NODE_RETURN);
        if (id != -1) {
            returnNode->name = strdup(tabela[id].tipo);
        } else {
            returnNode->name = strdup("unknown");
        }
        addChild(returnNode, (yyvsp[(2) - (3)].node));
        (yyval.node) = returnNode;
    ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 593 "parser.y"
    {
          int idx = busca((yyvsp[(1) - (3)].sval), escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", (yyvsp[(1) - (3)].sval));
          }
          
          TreeNode* assignNode = createNode(NODE_ASSIGN);
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup((yyvsp[(1) - (3)].sval));
          addChild(assignNode, idNode);
          addChild(assignNode, (yyvsp[(3) - (3)].node));
          (yyval.node) = assignNode;
          
          free((yyvsp[(1) - (3)].sval));
      ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 608 "parser.y"
    {
          int idx = busca((yyvsp[(1) - (6)].sval), escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", (yyvsp[(1) - (6)].sval));
          }
          
          TreeNode* assignNode = createNode(NODE_ASSIGN);
          TreeNode* idNode = createNode(NODE_ID);
          char* arrayName = (char*)malloc(strlen((yyvsp[(1) - (6)].sval)) + 3);
          sprintf(arrayName, "%s[]", (yyvsp[(1) - (6)].sval));
          idNode->name = arrayName;
          // Adiciona o índice como filho do ID
          addChild(idNode, (yyvsp[(3) - (6)].node));
          addChild(assignNode, idNode);
          addChild(assignNode, (yyvsp[(6) - (6)].node));
          (yyval.node) = assignNode;
          
          free((yyvsp[(1) - (6)].sval));
    ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 627 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 631 "parser.y"
    {
        (yyval.node) = (yyvsp[(2) - (3)].node);
        addChild((yyval.node), (yyvsp[(1) - (3)].node));
        addChild((yyval.node), (yyvsp[(3) - (3)].node));
      ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 636 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 640 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("<=");
        (yyval.node) = opNode;
      ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 645 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("<");
        (yyval.node) = opNode;
      ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 650 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup(">");
        (yyval.node) = opNode;
      ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 655 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup(">=");
        (yyval.node) = opNode;
      ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 660 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("==");
        (yyval.node) = opNode;
      ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 665 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("!=");
        (yyval.node) = opNode;
      ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 673 "parser.y"
    {
        (yyval.node) = (yyvsp[(2) - (3)].node);
        addChild((yyval.node), (yyvsp[(1) - (3)].node));
        addChild((yyval.node), (yyvsp[(3) - (3)].node));
      ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 678 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 682 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("ADD");
        (yyval.node) = opNode;
      ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 687 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("SUB");
        (yyval.node) = opNode;
      ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 695 "parser.y"
    {
        (yyval.node) = (yyvsp[(2) - (3)].node);
        addChild((yyval.node), (yyvsp[(1) - (3)].node));
        addChild((yyval.node), (yyvsp[(3) - (3)].node));
      ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 700 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 704 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("MULT");
        (yyval.node) = opNode;
      ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 709 "parser.y"
    {
        TreeNode* opNode = createNode(NODE_OP);
        opNode->op = strdup("DIV");
        (yyval.node) = opNode;
      ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 717 "parser.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 718 "parser.y"
    {
          int idx = busca((yyvsp[(1) - (1)].sval), escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", (yyvsp[(1) - (1)].sval));
          } else if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", (yyvsp[(1) - (1)].sval));
          }
          
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup((yyvsp[(1) - (1)].sval));
          (yyval.node) = idNode;
          
          free((yyvsp[(1) - (1)].sval));
    ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 732 "parser.y"
    { temp_args_count = 0; ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 732 "parser.y"
    {
          /* CHAMADA DE FUNÇÃO DENTRO DE EXPRESSÃO - retorno é usado */
          int idx = busca((yyvsp[(1) - (5)].sval), escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: funcao '%s' nao declarada.\n", (yyvsp[(1) - (5)].sval));
          } else if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", (yyvsp[(1) - (5)].sval));
          } else {
              // Verifica número de argumentos
              if (tabela[idx].param_counter != (yyvsp[(4) - (5)].param_count)) {
                  printf("Erro semantico: funcao '%s' espera %d parametro(s), mas recebeu %d argumento(s).\n", 
                         (yyvsp[(1) - (5)].sval), tabela[idx].param_counter, (yyvsp[(4) - (5)].param_count));
              }
          }
          
          TreeNode* callNode = createNode(NODE_CALL);
          TreeNode* idNode = createNode(NODE_ID);
          idNode->name = strdup((yyvsp[(1) - (5)].sval));
          
          // Adiciona os argumentos como filhos do ID da função
          for (int i = 0; i < temp_args_count; i++) {
              addChild(idNode, temp_args[i]);
          }
          
          addChild(callNode, idNode);
          (yyval.node) = callNode;
          temp_args_count = 0;
          
          free((yyvsp[(1) - (5)].sval));
    ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 762 "parser.y"
    {
          int idx = busca((yyvsp[(1) - (4)].sval), escopo_atual);
          if (idx == -1) {
              printf("Erro semantico: variavel '%s' nao declarada.\n", (yyvsp[(1) - (4)].sval));
          }

          if (strcmp(tabela[idx].tipo, "void") == 0) {
              printf("Erro semantico: variavel '%s' do tipo void nao pode ser usada em expressoes.\n", (yyvsp[(1) - (4)].sval));
          }
          
          TreeNode* idNode = createNode(NODE_ID);
          char* arrayName = (char*)malloc(strlen((yyvsp[(1) - (4)].sval)) + 3);
          sprintf(arrayName, "%s[]", (yyvsp[(1) - (4)].sval));
          idNode->name = arrayName;
          // Adiciona o índice como filho do ID
          addChild(idNode, (yyvsp[(3) - (4)].node));
          (yyval.node) = idNode;
          
          free((yyvsp[(1) - (4)].sval));
    ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 782 "parser.y"
    {
        TreeNode* constNode = createNode(NODE_CONST);
        constNode->value = (yyvsp[(1) - (1)].ival);
        (yyval.node) = constNode;
    ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 787 "parser.y"
    {
        TreeNode* inputNode = createNode(NODE_INPUT);
        (yyval.node) = inputNode;
    ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 794 "parser.y"
    { (yyval.param_count) = (yyvsp[(1) - (1)].param_count); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 795 "parser.y"
    { (yyval.param_count) = 0; ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 799 "parser.y"
    { 
        (yyval.param_count) = (yyvsp[(1) - (3)].param_count) + 1; 
        temp_args[temp_args_count++] = (yyvsp[(3) - (3)].node);
      ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 803 "parser.y"
    { 
        (yyval.param_count) = 1; 
        temp_args[temp_args_count++] = (yyvsp[(1) - (1)].node);
      ;}
    break;



/* Line 1455 of yacc.c  */
#line 2632 "parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 809 "parser.y"


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
