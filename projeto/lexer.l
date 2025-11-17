%{
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h" 

FILE *out;

#define YY_USER_ACTION yylloc.first_line = yylineno; \
                       
%}

%option yylineno
%option noyywrap

%x COMMENT

%%

"//".*                    { /* ignora comentario de linha */ }
"/*"                      { BEGIN(COMMENT); }

<COMMENT>{
    "*/"                  { BEGIN(INITIAL); }
    \n                    { }   /* reseta coluna em nova linha */
    .                     { /* ignora */ }
}

"if"                      { fprintf(out, "[IF] "); return IF; }
"else"                    { fprintf(out, "[ELSE] "); return ELSE; }
"while"                   { fprintf(out, "[WHILE] "); return WHILE; }
"return"                  { fprintf(out, "[RETURN] "); return RETURN; }
"int"                     { fprintf(out, "[INT] "); return INT; }
"void"                    { fprintf(out, "[VOID] "); return VOID; }
"output"                  { fprintf(out, "[OUTPUT] "); return OUTPUT; }
"input"                   { fprintf(out, "[INPUT] "); return INPUT; }

[0-9]+                    { fprintf(out, "[NUM:%s] ", yytext); yylval.ival = atoi(yytext); return NUM; }
[a-zA-Z_][a-zA-Z0-9_]*    { fprintf(out, "[ID:%s] ", yytext); yylval.sval = strdup(yytext); return ID; }

"!="                      { fprintf(out, "[NEQ] "); return NEQ; }
"=="                      { fprintf(out, "[EQ] "); return EQ; }
">="                      { fprintf(out, "[GEQ] "); return GEQ; }
"<="                      { fprintf(out, "[LEQ] "); return LEQ; }
"<"                       { fprintf(out, "[LT] "); return LT; }
">"                       { fprintf(out, "[GT] "); return GT; }

"+"                       { fprintf(out, "[PLUS] "); return PLUS; }
"*"                       { fprintf(out, "[TIMES] "); return TIMES; }
"-"                       { fprintf(out, "[MINUS] "); return MINUS; }
"/"                       { fprintf(out, "[DIVIDE] "); return DIVIDE; }
"="                       { fprintf(out, "[ASSIGN] "); return ASSIGN; }

";"                       { fprintf(out, "[SEMI] "); return SEMI; }
"{"                       { fprintf(out, "[LBRACE] "); return LBRACE; }
"}"                       { fprintf(out, "[RBRACE] "); return RBRACE; }
"("                       { fprintf(out, "[LPAREN] "); return LPAREN; }
")"                       { fprintf(out, "[RPAREN] "); return RPAREN; }
"["                       { fprintf(out, "[LBRACKET] "); return LBRACKET; }
"]"                       { fprintf(out, "[RBRACKET] "); return RBRACKET; }
","                       { fprintf(out, "[COMMA] "); return COMMA; }

\n                        { fprintf(out, "\n"); }
[ \t\r]                   { /* ignora espaços e tabs */ }

.                         { fprintf(out, "[UNK:%s] "); return yytext[0]; }

%%