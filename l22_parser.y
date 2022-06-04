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
};

//qualificadores
%token tPUBLIC tFOREIGN tUSE tVAR
%token tTYPE_INT tTYPE_DOUBLE tTYPE_STRING tTYPE_VOID tTYPE_POINTER

%token <i> tINTEGER
%token <s> tIDENTIFIER tSTRING
%token tWHILE tIF tPRINT tREAD tBEGIN tEND

%nonassoc tIFX
%nonassoc tELSE

%right '='
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY

// FIXME por os nos nos sitios certos
%type <node> stmt program declaration
%type <sequence> list file expressions declarations opt_declaration opt_instructions
%type <expression> expr
%type <lvalue> lval
%type <block> blk

// TODO verificar se funct_type esta bem aqui ou nao, nao faco ideia
%type<type> data_type funct_type

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file : /* empty */ { compiler->ast ($$ = new cdk::sequence_node(LINE)); }
     | declarations { compiler->ast($$ = $1); }
     | declarations program { compiler-> ast( $$ = new cdk::sequence_node(LINE, NULL)); } //TODO está mal ver qual é o node
     ;

declarations :              declaration { $$ = new cdk::sequence_node(LINE, $1);     }
             | declarations declaration { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;
// TODO - pode levar varias expressoes como declaracao de variavel?
// TODO - no og havia o tuple declaration node para multiple variable declarations
// TODO - este lval sozinho é suposto poder existir? não esta no og
// TODO - fazer restantes casos
declaration : lval 				{ $$ = $1;}
	    | tPUBLIC data_type tIDENTIFIER '=' expr	{ $$ = new l22::variable_declaration_node(LINE, tPUBLIC, $2, *$3, $5);}
	    ;

data_type    : tTYPE_STRING                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING);  }
             | tTYPE_INT                        { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);     }
             | tTYPE_DOUBLE                     { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);  }
// TODO verificar se o type_void pode ser aqui... wtf é um reference_type
	     | tTYPE_VOID			{ $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID);    }
	     | tTYPE_POINTER '[' data_type ']'	{ $$ = cdk::reference_type::create(4, $3); 		  }

// FIXME nao sei o que estou a fazer
	     | funct_type			{ $$ = $1;}
             ;


// FIXME qual e o node que recebe um func type? nao pode ser o definition pq esse vai ser preciso qnd fizermos a func
funct_type   : data_type 			{ $$ = cdk::functional_type::create($1);}
//	     | data_type '<' data_type '>'	{ $$ = cdk::functional_type::create($3, $1); }
	     ;


expressions     : expr                     { $$ = new cdk::sequence_node(LINE, $1);     }
                | expr ',' expr     	   { $$ = new cdk::sequence_node(LINE, $3, $1); }
                ;

program	: tBEGIN blk tEND { compiler->ast(new l22::program_node(LINE, $2)); }
	      ;
// TOMAS instructions
list : stmt	     { $$ = new cdk::sequence_node(LINE, $1); }
	   | list stmt { $$ = new cdk::sequence_node(LINE, $2, $1); }
	   ;

// TOMAS instruction
stmt : expr ';'                         { $$ = new l22::evaluation_node(LINE, $1); }
 	   | tPRINT list ';'                  { $$ = new l22::write_node(LINE, $2); }
     | tREAD expr ';'                   { $$ = new l22::input_node(LINE, $2); }
     | tWHILE '(' expr ')' stmt         { $$ = new l22::while_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt %prec tIFX { $$ = new l22::if_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt tELSE stmt { $$ = new l22::if_else_node(LINE, $3, $5, $7); }
     | '{' list '}'                     { $$ = $2; }
     ;

expr : tINTEGER                { $$ = new cdk::integer_node(LINE, $1); }
	   | tSTRING                 { $$ = new cdk::string_node(LINE, $1); }
     | '-' expr %prec tUNARY   { $$ = new cdk::neg_node(LINE, $2); }
     | expr '+' expr	         { $$ = new cdk::add_node(LINE, $1, $3); }
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
     | '(' expr ')'            { $$ = $2; }
     | lval                    { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '=' expr           { $$ = new cdk::assignment_node(LINE, $1, $3); }
     ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, $1); }
     ;

// FIXME grammar conflict
opt_declaration : /* empty */ { $$ = NULL ;}
		| declarations { $$ = $1;   }
		;
opt_instructions: /* empty */  { $$ = new cdk::sequence_node(LINE); }
                | list { $$ = $1; }
                ;
blk : opt_declaration opt_instructions { $$ = new l22::block_node(LINE, $1, $2);}
    ;

%%
