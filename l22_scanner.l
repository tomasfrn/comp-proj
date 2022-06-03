%option c++ prefix="l22_scanner_" outfile="l22_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "l22_parser.tab.h"

// don't change this
#define yyerror LexerError
%}
%x X_STRING X_COMMENT
%%
  yydebug=1; set_debug(1);


/* ====================================================================== */
/* ====[                      3.3 - COMMENTS                        ]==== */
/* ====================================================================== */

";".*                 ; /* ignore comments */

"(*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"(*"        yy_push_state(X_COMMENT);
<X_COMMENT>"*)"        yy_pop_state();
<X_COMMENT>.|\n        ; /* ignore content */

/* ====================================================================== */
/* ====[                                                            ]==== */
/* ====================================================================== */

"foreign"              return tFOREIGN;
"use"                  return tUSE;
"public"               return tPUBLIC;
"var"                  return tVAR;

"not"                  return tNOT; 
"and"                  return '&'; 
"or"                   return '|'; 

/* ====================================================================== */
/* ====[                                                            ]==== */
/* ====================================================================== */

">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE; 

"while"                return tWHILE; 
"if"                   return tIF; 
"then"                 return tTHEN; 
"elif"                 return tELIF; 
"else"                 return tELSE; 
"print"                return tPRINT; 
"read"                 return tREAD; 
"begin"                return tBEGIN; 
"end"                  return tEND; 

"do"                   return tDO;
"stop"                 return tSTOP;
"again"                return tAGAIN;
"return"               return tRETURN;

"write"                return tWRITE;
"writeln"              return tWRITELN;

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

\'                     yy_push_state(X_STRING); yylval.s = new std::string("");
<X_STRING>\'           yy_pop_state(); return tSTRING;
<X_STRING>\\\'         *yylval.s += yytext + 1;
<X_STRING>.            *yylval.s += yytext;
<X_STRING>\n           yyerror("newline in string");

[0-9]+                 yylval.i = strtol(yytext, nullptr, 10); return tINTEGER;

[-()<>=+*/%;{}.]       return *yytext;

[ \t\n]+               ; /* ignore whitespace */

.                      yyerror("Unknown character");

%%
