#ifndef __L22_AST_NULLPTR_H__
#define __L22_AST_NULLPTR_H__

#include <cdk/ast/expression_node.h>

namespace l22 {

    class null_ptr_node: public cdk::expression_node {
    public:
        null_ptr_node(int lineno) :
                cdk::expression_node(lineno) {
        }

    public:
        void accept(basic_ast_visitor *sp, int level) {
            sp->do_null_ptr_node(this, level);
        }

    };

} // l22

#endif
