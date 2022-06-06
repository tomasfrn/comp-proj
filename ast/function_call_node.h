#ifndef __L22_AST_FUNCTION_CALL_H__
#define __L22_AST_FUNCTION_CALL_H__

#include <string>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>

namespace l22
{

    class function_call_node : public cdk::expression_node
    {
        std::string _identifier;
        cdk::expression_node *_expr;
        cdk::sequence_node *_arguments;


    public:
        function_call_node(int lineno, const std::string &identifier) : cdk::expression_node(lineno), _identifier(identifier), _expr(nullptr), _arguments(new cdk::sequence_node(lineno)){
        }

        function_call_node(int lineno, const std::string &identifier, cdk::sequence_node *arguments) : cdk::expression_node(lineno), _identifier(identifier), _expr(nullptr), _arguments(arguments){
        }

        function_call_node(int lineno, cdk::expression_node *expr) : cdk::expression_node(lineno), _identifier(""), _expr(expr), _arguments(new cdk::sequence_node(lineno)){
        }

        function_call_node(int lineno, cdk::expression_node *expr, cdk::sequence_node *arguments) : cdk::expression_node(lineno), _identifier(""), _expr(expr), _arguments(arguments){
        }

    public:
        const std::string &identifier()
        {
            return _identifier;
        }
        cdk::sequence_node *arguments()
        {
            return _arguments;
        }
        cdk::expression_node *expr()
        {
            return _expr;
        }

        void accept(basic_ast_visitor *sp, int level)
        {
            sp->do_function_call_node(this, level);
        }
    };

} // l22

#endif