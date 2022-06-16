%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;	/* integer value */
  double                d;    /*double value*/
  std::string          *s;	/* symbol name or string literal */
  cdk::basic_node      *node;	/* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  l22::block_node      *block;
  cdk::typed_node      *typenode;
  std::vector<std::shared_ptr<cdk::basic_type>>  *vector;

};

//qualificadores
%token tPUBLIC tFOREIGN tUSE tVAR tPRIVATE
%token tTYPE_STRING tTYPE_VOID /*tTYPE_POINTER*/
%token <i> tINTEGER
%token <s> tIDENTIFIER tTEXT
%token <d> tDOUBLE
%token tWHILE tIF tWRITE tWRITELN tINPUT tBEGIN tEND tAGAIN tSTOP tRETURN tTHEN tDO tELIF tSIZEOF tRETURNS tNULL tTYPE_INT tTYPE_DOUBLE

%nonassoc tIFX
%nonassoc tELSE

%right '=' tNOT
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%' tAND tOR
%nonassoc tUNARY

%type <node> instruction program vardeclaration bigif
%type <sequence> instructions file expressions declarations variables
%type <expression> expr opt_initializer funcdef
%type <lvalue> lval
%type <block> blk
%type <i> var
%type <s> text
%type <vector> data_types

%type<type> data_type funct_type

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%
file : /* empty */ { compiler->ast ($$ = new cdk::sequence_node(LINE)); }
     | declarations { compiler->ast($$ = new cdk::sequence_node(LINE, $1)); }
     | declarations program { compiler-> ast( $$ = new cdk::sequence_node(LINE, $2, $1)); }
     | program { compiler->ast($$ = new cdk::sequence_node(LINE,$1)); }
     ;

program	: tBEGIN blk tEND { $$ = (new l22::program_node(LINE, $2)); }
	;

declarations : vardeclaration { $$ = new cdk::sequence_node(LINE, $1);     }
 		| vardeclaration ';'{ $$ = new cdk::sequence_node(LINE, $1);     }
             | declarations vardeclaration { $$ = new cdk::sequence_node(LINE, $2, $1); }
             | declarations vardeclaration ';' { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;

opt_initializer  : /* empty */         { $$ = nullptr; /* must be nullptr, not NIL */ }
                 | '=' expr      { $$ = $2; }
                 ;

vardeclaration :	 data_type tIDENTIFIER opt_initializer	{ $$ = new l22::variable_declaration_node(LINE, tPRIVATE, $1, *$2, $3); delete $2;}
	       | tPUBLIC data_type tIDENTIFIER opt_initializer	{ $$ = new l22::variable_declaration_node(LINE, tPUBLIC, $2, *$3, $4); delete $3; }
	       | tUSE    data_type tIDENTIFIER 	{ $$ = new l22::variable_declaration_node(LINE, tUSE, $2, *$3, nullptr); delete $3;}
	       | tFOREIGN data_type tIDENTIFIER 	{ $$ = new l22::variable_declaration_node(LINE, tFOREIGN, $2, *$3, nullptr); delete $3;}
	       | 	  var 	tIDENTIFIER '=' expr 		{ $$ = new l22::variable_declaration_node(LINE, tPRIVATE, nullptr, *$2, $4); delete $2;}
	       | tPUBLIC  var 	tIDENTIFIER '=' expr 		{ $$ = new l22::variable_declaration_node(LINE, tPUBLIC, nullptr, *$3, $5); delete $3;}
	       ;

var            : tVAR         { $$ = tVAR; }
               |              { $$ = '\0'; }
               ;

variables  : /* empty */         		{ $$ = nullptr;  }
	   | vardeclaration			{ $$ = new cdk::sequence_node(LINE, $1);}
	   | variables ',' vardeclaration	{ $$ = new cdk::sequence_node(LINE, $3, $1);}
	   ;


data_type    : tTYPE_STRING                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING);  }
             | tTYPE_INT                        { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);     }
             | tTYPE_DOUBLE                     { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);  }
	     | tTYPE_VOID			{ $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID);    }
	     | '[' data_type ']'		{ $$ = cdk::reference_type::create(4, $2); 		  }
	     | funct_type			{ $$ = $1;}
             ;

data_types : /* vazio */                     { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); }
           | data_type                       { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); $$->push_back($1); }
           | data_types ',' data_type        { $$ = $1; $$->push_back($3); }


funct_type    : data_type '<' data_types '>'	{ $$ = cdk::functional_type::create(*$3, $1); }
	     ;

funcdef      : '(' variables ')' tRETURNS data_type ':' blk { $$ = new l22::function_definition_node(LINE, $2, $5, $7);}
	     ;

expressions     : expr                     { $$ = new cdk::sequence_node(LINE, $1);     }
                | expressions ',' expr     	   { $$ = new cdk::sequence_node(LINE, $3, $1); }
     		| 				{ $$ = new cdk::sequence_node(LINE);}
                ;

instructions : instruction ';'	     { $$ = new cdk::sequence_node(LINE, $1); }
 		| instruction 	     { $$ = new cdk::sequence_node(LINE, $1); }
	   | instructions instruction ';' { $$ = new cdk::sequence_node(LINE, $2, $1); }
	   | instructions instruction  { $$ = new cdk::sequence_node(LINE, $2, $1); }
	   ;

instruction : expr                      { $$ = new l22::evaluation_node(LINE, $1); }
 	    | tWRITE expressions       	{ $$ = new l22::write_node(LINE, $2, false); }
 	    | tWRITELN expressions      { $$ = new l22::write_node(LINE, $2, true); }
 	    | tAGAIN			{ $$ = new l22::again_node(LINE);}
 	    | tSTOP			{ $$ = new l22::stop_node(LINE);}
 	    | tRETURN expr		{ $$ = new l22::return_node(LINE, $2);}

	    | tIF '(' expr ')' tTHEN blk	{ $$ = new l22::if_node(LINE, $3, $6);}
	    | tIF '(' expr ')' tTHEN blk bigif	{ $$ = new l22::if_else_node(LINE, $3, $6, $7);}

	    | tWHILE '(' expr ')' tDO blk	{ $$ = new l22::while_node(LINE, $3, $6);}
	    | blk                     { $$ = $1; }
            ;

bigif      : tELSE blk                   { $$ = $2; }
            | tELIF '(' expr ')' tTHEN blk        { $$ = new l22::if_node(LINE, $3, $6); }
            | tELIF '(' expr ')' tTHEN blk bigif   { $$ = new l22::if_else_node(LINE, $3, $6, $7); }
            ;

text          : tTEXT                       { $$ = $1; }
                | text tTEXT                { $$ = new std::string(*$1 + *$2); delete $1; delete $2; }
                ;

expr : tINTEGER                { $$ = new cdk::integer_node(LINE, $1); }
     | tDOUBLE		{ $$ = new cdk::double_node(LINE, $1);  }
     | text                 { $$ = new cdk::string_node(LINE, $1); }
     | '-' expr %prec tUNARY   { $$ = new cdk::neg_node(LINE, $2); }
     | '+' expr %prec tUNARY   { $$ = new l22::identity_node(LINE, $2); }
     | tNOT expr		{ $$ = new cdk::not_node(LINE, $2);}
     | expr '+' expr	        { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr	         { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr	         { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr	         { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr	         { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr	         { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr	         { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr	         { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr           { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr	         { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr	         { $$ = new cdk::eq_node(LINE, $1, $3); }
     | expr tOR expr		{ $$ = new cdk::or_node(LINE, $1, $3);}
     | expr tAND expr		{ $$ = new cdk::and_node(LINE, $1, $3);}
     | tNULL			{ $$ = new l22::null_ptr_node(LINE);}
     | tINPUT 			{ $$ = new l22::input_node(LINE);}
     | tSIZEOF '(' expr ')'	{ $$ = new l22::sizeof_node(LINE, $3);}
     | funcdef			{ $$ = $1;}
     | '(' expr ')'            { $$ = $2; }
     | lval '?'		       { $$ = new l22::address_of_node(LINE, $1);	}
     | lval                    { $$ = new cdk::rvalue_node(LINE, $1); }
     | lval '=' expr           { $$ = new cdk::assignment_node(LINE, $1, $3); }
     | '[' expr ']' 	{ $$ = new l22::stack_alloc_node(LINE, $2);}
     | tIDENTIFIER '(' expressions ')' { $$ = new l22::function_call_node(LINE, *$1, $3);}
     | '@' '(' expressions ')'	   { $$ = new l22::function_call_node(LINE, nullptr, $3);}
     ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, $1); }
     | lval '[' expr ']'      { $$ = new l22::index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
     ;

blk :	'{' instructions '}' { $$ = new l22::block_node(LINE, nullptr, $2);}
    |	 '{' declarations instructions '}' { $$ = new l22::block_node(LINE, $2, $3);}
    | '{' declarations '}' 	{ $$ = new l22::block_node(LINE, $2, nullptr);}
    | '{' '}' 			{ $$ = new l22::block_node(LINE, nullptr, nullptr);}
    ;

%%