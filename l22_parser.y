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
  std::string          *s;	/* symbol name or string literal */
  cdk::basic_node      *node;	/* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  l22::block_node      *block;
  cdk::typed_node      *typenode;

};

//qualificadores
%token tPUBLIC tFOREIGN tUSE tVAR tPRIVATE
%token tTYPE_STRING tTYPE_VOID /*tTYPE_POINTER*/

%token <i> tTYPE_INT
%token <s> tIDENTIFIER tSTRING
%token <d> tTYPE_DOUBLE
%token tWHILE tIF tWRITE tWRITELN tINPUT tBEGIN tEND tAGAIN tSTOP tRETURN tTHEN tDO tELIF tNOT tSIZEOF

%nonassoc tIFX
%nonassoc tELSE

%right '='
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%' '|' '&'
%nonassoc tUNARY

// FIXME por os nos nos sitios certos
%type <node> instruction program vardeclaration bigif
%type <sequence> instructions file expressions declarations opt_declaration opt_instructions variables
%type <expression> expr opt_initializer
%type <lvalue> lval
%type <block> blk
%type <typenode> funcdef

// TODO verificar se funct_type esta bem aqui ou nao, nao faco ideia
%type<type> data_type funct_type

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file : /* empty */ { compiler->ast ($$ = new cdk::sequence_node(LINE)); }
     | declarations { compiler->ast($$ = $1); }
     | declarations program { compiler-> ast( $$ = new cdk::sequence_node(LINE, NULL)); } //TODO está mal ver qual é o node
            | program 		{ compiler->ast($$ = $1); }
     ;

declarations :              vardeclaration { $$ = new cdk::sequence_node(LINE, $1);     }
             | declarations vardeclaration { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;
// TODO - pode levar varias expressoes como declaracao de variavel?
// TODO - no og havia o tuple declaration node para multiple variable declarations

opt_initializer  : /* empty */         { $$ = nullptr; /* must be nullptr, not NIL */ }
                 | '=' expr      { $$ = $2; }
                 ;

vardeclaration :	      data_type tIDENTIFIER opt_initializer	{ $$ = new l22::variable_declaration_node(LINE, tPRIVATE, $1, *$2, $3);}
	       | tPUBLIC data_type tIDENTIFIER opt_initializer	{ $$ = new l22::variable_declaration_node(LINE, tPUBLIC, $2, *$3, $4);}
	       | tPUBLIC tVAR 	tIDENTIFIER '=' expr 		{ $$ = new l22::variable_declaration_node(LINE, tPUBLIC, tVAR, *$3, $5);}
	       | tUSE    data_type tIDENTIFIER opt_initializer	{ $$ = new l22::variable_declaration_node(LINE, tUSE, $2, *$3, $4);}
	       // Ponteiro para funcao definida externamente
	       | tFOREIGN data_type tIDENTIFIER opt_initializer	{ $$ = new l22::variable_declaration_node(LINE, tFOREIGN, $2, *$3, $4);}
	       ;

variables  : /* empty */         		{ $$ = new cdk::sequence_node(LINE);  }
	   | vardeclaration			{ $$ = new cdk::sequence_node(LINE, $1);}
	   | variables ',' vardeclaration	{ $$ new cdk::sequence_node(LINE, $3, $1);}
	   ;


data_type    : tTYPE_STRING                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING);  }
             | tTYPE_INT                        { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);     }
             | tTYPE_DOUBLE                     { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);  }
	     | tTYPE_VOID			{ $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID);    }
//	     | tTYPE_POINTER 			{ $$ = cdk::primitive_type::create(4, cdk::TYPE_POINTER); }
	     | '[' data_type ']'		{ $$ = cdk::reference_type::create(4, $2); 		  }
	     | funct_type			{ $$ = $1;}
             ;


funct_type   : data_type 			{ $$ = cdk::functional_type::create($1);}
	     | data_type '<' data_type '>'	{ $$ = cdk::functional_type::create($3, $1); }
	     ;
// FIXME mudar o func definition node
funcdef      : '(' variables ')' '-' '>' data_type ':' blk { $$ = new l22::function_definition_node(LINE, NULL, NULL, $2, $8);}
	     ;

expressions     : expr                     { $$ = new cdk::sequence_node(LINE, $1);     }
                | expr ',' expr     	   { $$ = new cdk::sequence_node(LINE, $3, $1); }
                ;

program	: tBEGIN blk tEND { compiler->ast(new l22::program_node(LINE, $2)); }
	      ;
// TOMAS instructions
instructions : instruction	     { $$ = new cdk::sequence_node(LINE, $1); }
	   | instructions instruction { $$ = new cdk::sequence_node(LINE, $2, $1); }
	   ;

// TOMAS instruction
instruction : expr                      { $$ = new l22::evaluation_node(LINE, $1); }
 	    | tWRITE expressions       	{ $$ = new l22::write_node(LINE, $2, false); }
 	    | tWRITELN expressions      { $$ = new l22::write_node(LINE, $2, true); }
 	    | tAGAIN			{ $$ = new l22::again_node(LINE);}
 	    | tSTOP			{ $$ = new l22::stop_node(LINE);}
 	    | tRETURN expr		{ $$ = new l22::return_node(LINE, $2);}
     	    | tINPUT expr ';'                   { $$ = new l22::input_node(LINE, $2); }

	    | tIF '(' expr ')' tTHEN blk	{ $$ = new l22::if_node(LINE, $3, $6);}
	    | tIF '(' expr ')' tTHEN blk bigif	{ $$ = new l22::if_else_node(LINE, $3, $6, $7);}

	    | tWHILE '(' expr ')' tDO blk	{ $$ = new l22::while_node(LINE, $3, $6);}
            | blk                     { $$ = $1; }
            ;

bigif      : tELSE blk                   { $$ = $2; }
            | tELIF '(' expr ')' tTHEN blk        { $$ = new l22::if_node(LINE, $3, $6); }
            | tELIF '(' expr ')' tTHEN blk bigif   { $$ = new l22::if_else_node(LINE, $3, $6, $7); }
            ;

expr : tTYPE_INT                { $$ = new cdk::integer_node(LINE, $1); }
     | tTYPE_DOUBLE		{ $$ = new cdk::double_node(LINE, $1);  }
     | tSTRING                 { $$ = new cdk::string_node(LINE, $1); }
     | '-' expr %prec tUNARY   { $$ = new cdk::neg_node(LINE, $2); }
     | '-' expr %prec tUNARY   { $$ = new l22::identity_node(LINE, $2); }
     | tNOT expr		{ $$ = new cdk:not_node(LINE, $2);}
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
     | expr '|' expr		{ $$ = new cdk::or_node(LINE, $1, $3);}
     | expr '&' expr		{ $$ = new cdk::and_node(LINE, $1, $3);}
     | tINPUT 			{ $$ = new l22::input_node(LINE);}
     | tSIZEOF '(' expressions ')'	{ $$ = new l22::size_of(LINE, new cdk::sequence_node(LINE, $3);}
     | '(' expr ')'            { $$ = $2; }
     | lval '?'		       { $$ = new l22::address_of_node(LINE, $1);	}
     | lval                    { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '=' expr           { $$ = new cdk::assignment_node(LINE, $1, $3); }
// TODO stack allocation node    |
     ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, $1); }
     ;

// FIXME grammar conflict
opt_declaration : /* empty */ { $$ = NULL ;}
		| declarations { $$ = $1;   }
		;
opt_instructions: /* empty */  { $$ = new cdk::sequence_node(LINE); }
                | instructions { $$ = $1; }
                ;
blk : opt_declaration opt_instructions { $$ = new l22::block_node(LINE, $1, $2);}
    ;

%%
