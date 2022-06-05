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

SPACE     [ ]
%x X_STRING X_SPECIALSTRING X_COMMENT X_NEWLINE
%%
  yydebug=1; set_debug(1);

  /*------TIPOS DE DADOS------*/

"int"                    return tTYPE_INT;
"double"                 return tTYPE_DOUBLE;
"text"                   return tTYPE_STRING;
"void"                   return tTYPE_VOID;
  /*"["[0-9]"]"            return tTYPE_POINTER;*/

  /*------INDENTACAO------*/

\n                   yy_push_state(X_NEWLINE); /* delay decision */

<X_NEWLINE>^{SPACE}+  {
    yy_pop_state();

    if (valid(yyleng)) {
          std::cerr << "CASE 1" << std::endl;
          if (lLevelsToPop == 0) {
            // at the same level: ignore index
            // do not return tINDENT;
            std::cerr << "CASE 1a (levels to pop = 0)" << std::endl;
            return ';';
          }
          else {
            // nothing is done here: the scanner will ignore the input while levels > 0
            // and send block-end tokens to the parser
            std::cerr << "CASE 1b (levels to pop = " << lLevelsToPop << ")" << std::endl;
            // at least one has to be returned here...
            if (lLevelsToPop > 0) {
              std::cerr << "POPPING LEVELS (" << lLevelsToPop << ")" << std::endl;
              lLevelsToPop--;
              indents.pop_back();
              return '}';
            }
          }
    }
    else if (indents.size() == 0) {
          // first block to open
          std::cerr << "CASE 2" << std::endl;
          indents.push_back(yyleng);
          return '{';
    }
    else if (yyleng > indents.back()) {
          // this is like the one before, but not the first
          std::cerr << "CASE 3" << std::endl;
          indents.push_back(yyleng);
          return '{';
    }
    else {
          // something wrong: bad indent: communicate to user
          std::cerr << "CASE 4" << std::endl;
          std::cerr << "bad indent at line " << yylineno;
    }

}
<X_NEWLINE>\n      ; // ignore successive newline chars
<X_NEWLINE>.       {
  // in this case, we have a new function starting
  // first, return whatever we are seeing to the input
  yyless(0);
  yy_pop_state();
  if (indents.size() > 0) {
    std::cerr << "should restart " << indents.size() << " functions at " << yylineno << std::endl;
    indents.pop_back();
    if (indents.size() > 0) { restart_functions = true; }
    return '}';
  }
  else return ';'; // top-level declarations
}

  /*------COMMENTS------*/

";".*                 ; /* ignore comments */

"(*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"(*"        yy_push_state(X_COMMENT);
<X_COMMENT>"*)"        yy_pop_state();
<X_COMMENT>.|\n        ; /* ignore content */

  /*------DECLARACAO------*/

"public"               return tPUBLIC;
"use"                  return tUSE;
"foreign"              return tFOREIGN;
"var"                  return tVAR;

  /*------INDICACAO DE POSICAO------*/

"?"                    return tPOS;

  /*------OPERADORES------*/ 

">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE; 
"not"                  return tNOT; 
"and"                  return '&'; 
"or"                   return '|'; 
"="                    return tATR;
"->"                   return tRETURNS;

  /*------ITERACAO TERMINACAO CONTINUACAO------*/

"while"                return tWHILE; 
"do:"                  return tDO;
"stop"                 return tSTOP;
"again"                return tAGAIN;

  /*------CONDICIONAL------*/

"if"                   return tIF; 
"then:"                 return tTHEN; 
"elif"                 return tELIF; 
"else:"                 return tELSE; 

  /*------RETORNO------*/

"return"               return tRETURN;

  /*------FUNCAO PRINCIPAL------*/

"begin"                return tBEGIN; 
"end"                  return tEND; 

  /*------SIZEOF------*/

"sizeof"               return tSIZEOF;

  /*------ESCRITA------*/

"write"                return tWRITE;
"writeln"              return tWRITELN;

  /*------EXPRESSOES------*/

"input"                return tINPUT;
"null"                 return tNULL;

  /*------IDENTIFIERS------*/

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

  /*------STRINGZORDS------*/

"\""                   yy_push_state(X_STRING);
<X_STRING>"\\"          yy_push_state(X_SPECIALSTRING);
<X_STRING>"\""         {
  yylval.s = new std::string(strlit.str());
  strlit.str("");
  yy_pop_state();
  return tSTRING;
}
<X_STRING>.            strlit << *yytext;

<X_SPECIALSTRING>n         strlit << '\n'; yy_pop_state();
<X_SPECIALSTRING>r         strlit << '\r'; yy_pop_state();
<X_SPECIALSTRING>t         strlit << '\t'; yy_pop_state();
<X_SPECIALSTRING>"\""      strlit << '"'; yy_pop_state();
<X_SPECIALSTRING>"\\"       strlit << '\\'; yy_pop_state();
<X_SPECIALSTRING>[0-6]{1,3} {
  strlit << (char)(unsigned char)strtoul(yytext, NULL, 7);
  yy_pop_state();
}
<X_SPECIALSTRING>.                 strlit << *yytext; yy_pop_state();

  /*------INTEIROS------*/

0[0-6]+              yylval.i = strtoul(yytext, NULL, 7); return tINTEGER;
0[0-9]+              yyerror("Invalid base 7 number.");

0|[1-9][0-9]*        yylval.i = strtoul(yytext, NULL, 10); return tINTEGER;

  /*------REAIS------*/

([0-9]*\.[0-9]+|[0-9]+\.[0-9])([Ee]([-+])?[0-9]+)? { yylval.d = strtod(yytext, NULL); return tREAL; }


  /*------OUTROS------*/

[-()<>=+*/%;{}.]       return *yytext;

[ \t\n]+               ; /* ignore whitespace */

.                      yyerror("Unknown character");

%%
